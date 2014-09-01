#ifndef FIXEDSIZEHASHMAPSEARCH_H_
#define FIXEDSIZEHASHMAPSEARCH_H_

#include "ArgumentInterpreter.h"
#include "FullMatchManager.h"
#include "Genome.h"
#include "Match.h"
#include "NucleotideDatabaseIterator.h"
#include "StringUtilities.h"
#include <boost/filesystem.hpp>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mutex;

namespace GeneAssembler {

template < class T >
void threadFunction( FullMatchManager& fullMatchManager, FASTQIterator& iterEntryCur, FASTQIterator const& iterEntryEnd,
	size_t& countRead, size_t maxReads, size_t step, NucleotideDatabaseInformation& readInfo, NucleotideDatabase<T> const& db )
{
	mutex.lock();
	for ( ; iterEntryCur != iterEntryEnd and countRead != maxReads; ++iterEntryCur, ++countRead )
	{
		FASTQ read(**iterEntryCur);
		if (step == 1) readInfo.add(read.getSequence().getNbBases());

		Match match = fullMatchManager.getMatch(read.getName());

		mutex.unlock();
		fullMatchManager.findBestMatch(db,read,match);
		mutex.lock();

		fullMatchManager.addMatch(read.getName(),match);
	}
	mutex.unlock();
}

template < FixedTokenSizeType Size >
void fixedSizeHashMapSearchImpl( FullMatchManager& fullMatchManager, NucleotideDatabaseSettings const& nucleotideDatabaseSettings, ArgumentInterpreter const& arg )
{
	using namespace std;
	using boost::filesystem::path;

	path readFile = arg.getNonOptionalArgument("readFile");
	if (!exists(readFile)) throw GeneAssemblerException("File not found: " + readFile.string());

	path ntFile;
	if ( arg.isOptionalFlagSet("NTDatabase") ) ntFile = arg.getOptionalArgument("NTDatabase");
	else ntFile = Environment::getDatabaseFile();

	size_t maxEntries = arg.isOptionalFlagSet("maxEntries") ? boost::lexical_cast<size_t>(arg.getOptionalArgument("maxEntries")) : numeric_limits<size_t>::max();
	size_t maxBases = arg.isOptionalFlagSet("maxBases") ? boost::lexical_cast<size_t>(arg.getOptionalArgument("maxBases")) : numeric_limits<size_t>::max();
	size_t maxBasesPerStep = arg.isOptionalFlagSet("maxBasesPerStep") ? boost::lexical_cast<size_t>(arg.getOptionalArgument("maxBasesPerStep")) : numeric_limits<size_t>::max();
	size_t maxReads = arg.isOptionalFlagSet("maxReads") ? boost::lexical_cast<size_t>(arg.getOptionalArgument("maxReads")) : numeric_limits<size_t>::max();
	size_t startEntry = arg.isOptionalFlagSet("startEntry") ? boost::lexical_cast<size_t>(arg.getOptionalArgument("startEntry")) : numeric_limits<size_t>::min();

	NucleotideDatabaseInformation totalInfo;
	NucleotideDatabaseInformation readInfo;

	ofstream hashMapInfoFile;
	if (arg.isBooleanFlagSet("printHashMapInfo")) hashMapInfoFile.open("NucleotideDatabaseHashMapInformation.txt");

	size_t step(1);
	size_t nbCoresPerNode = Environment::getNbCoresPerNode();
	for ( NucleotideDatabaseIterator<Traits<Size> > iterNTDBCur(ntFile,maxEntries,maxBases,maxBasesPerStep,startEntry,nucleotideDatabaseSettings),
		iterNTDBEnd; iterNTDBCur != iterNTDBEnd; ++iterNTDBCur, ++step )
	{
		totalInfo.merge((*iterNTDBCur)->getInformation());

		if (arg.isBooleanFlagSet("printSteps")) {
			string header = " Intermediate report import informations for step " + boost::lexical_cast<string>(step);
			cout << endl << header << endl << string(header.size()+1,'-') << endl << (*iterNTDBCur)->getInformation() << endl;
		}

		if (arg.isBooleanFlagSet("printHashMapInfo")) {
			hashMapInfoFile << underline("NucleotideDatabaseHashMapInformation for step " + boost::lexical_cast<string>(step) + " " ) << endl;
			(*iterNTDBCur)->printHashMapReferenceSequenceState(hashMapInfoFile);
			hashMapInfoFile << endl;

			hashMapInfoFile << underline("Top five tokens: ") << endl;
			(*iterNTDBCur)->printMostFrequentOccurrences(hashMapInfoFile,5);
			hashMapInfoFile << endl;
		}

		size_t countRead(0);
		if ( nbCoresPerNode == 0 ) {
			for ( FASTQIterator iterEntryCur(readFile), iterEntryEnd;
				iterEntryCur != iterEntryEnd and countRead != maxReads; ++iterEntryCur, ++countRead )
			{
				if (step == 1) readInfo.add((*iterEntryCur)->getSequence().getNbBases());
				fullMatchManager.includeBestMatch(**iterNTDBCur,**iterEntryCur);
			}
		} else {
			vector< shared_ptr<thread> > threads(nbCoresPerNode);

			FASTQIterator iterEntryCur(readFile), iterEntryEnd;

			for ( auto & singleThread : threads ) {
				singleThread.reset(new thread(threadFunction< Traits<Size> >,ref(fullMatchManager),ref(iterEntryCur),ref(iterEntryEnd),
					ref(countRead),maxReads,step,ref(readInfo),ref(**iterNTDBCur)));
			}

			for ( auto & thread : threads ) thread->join();
		}

		if (step == 1) {
			string header = " Summary read report import informations";
			cout << endl << header << endl << string(header.size()+1,'-') << endl << readInfo << endl;
			fullMatchManager.setTotalNbImportedReadSeq(readInfo.getNbImportedGeneSeq());
		}

		cout << "Write binary FullMatchManager to file ... " << flush;
		writeBinary(fullMatchManager,"FullMatchManager","FullMatchManager.bin");
		cout << "done." << endl;

		cout << "Write xml FullMatchManager to file ... " << flush;
		writeXML(fullMatchManager,"FullMatchManager","FullMatchManager.xml");
		cout << "done." << endl;
	}

	string header = " Summary nucleotide report import informations";
	cout << endl << header << endl << string(header.size()+1,'-') << endl << totalInfo << endl;
}

void fixedSizeHashMapSearch( FixedTokenSizeType fixedTokenSize, FullMatchManager& fullMatchManager,
	NucleotideDatabaseSettings const& nucleotideDatabaseSettings, ArgumentInterpreter const& arg )
{
	if (fixedTokenSize == 12 ) {
		fixedSizeHashMapSearchImpl<12>(fullMatchManager,nucleotideDatabaseSettings,arg);
	} else if (fixedTokenSize == 16 ) {
		fixedSizeHashMapSearchImpl<16>(fullMatchManager,nucleotideDatabaseSettings,arg);
	} else if (fixedTokenSize == 20 ) {
		fixedSizeHashMapSearchImpl<20>(fullMatchManager,nucleotideDatabaseSettings,arg);
	} else if (fixedTokenSize == 24 ) {
		fixedSizeHashMapSearchImpl<24>(fullMatchManager,nucleotideDatabaseSettings,arg);
	} else if (fixedTokenSize == 36 ) {
		fixedSizeHashMapSearchImpl<36>(fullMatchManager,nucleotideDatabaseSettings,arg);
	} else if (fixedTokenSize == 48 ) {
		fixedSizeHashMapSearchImpl<48>(fullMatchManager,nucleotideDatabaseSettings,arg);
	} else
		throw GeneAssemblerException("fixedTokenSize not supported.");
}

} // namespace GeneAssembler

#endif /* FIXEDSIZEHASHMAPSEARCH_H_ */