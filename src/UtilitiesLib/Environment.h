#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_

#include <boost/filesystem.hpp>

namespace GeneAssembler {
namespace Environment {

boost::filesystem::path getDatabaseFile();
boost::filesystem::path getSharedDirectory();
boost::filesystem::path getLocalDirectory();
boost::filesystem::path getGeneAssemblerRootDirectory();
size_t getNbNodes();
size_t getNbCoresPerNode();
size_t getNodeMemoryGB();
std::string getMysqlHost();
std::string getMysqlUser();
std::string getMysqlPassword();
std::string getMysqlTaxDatabase();
std::string getMysqlMyDatabase();

} // namespace Environment
} // namespace GeneAssembler

#endif /* ENVIRONMENT_H_ */