include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/BoostLib
    ${MYSQL_INCLUDE_DIR}
)

add_library(
    GenomeLib
    CDSDatabase.cpp
    CDSIterator.cpp
    CharTypes.cpp
    CompressedCharTwoInOne.cpp
    CompressedCharFourInOne.cpp
    FullMatchManager.cpp
    LocalAlignment.cpp
    Location.cpp
    NucleotideDatabaseInformation.cpp
    Organism.cpp
    OrganismsDataset.cpp
    OrganismsReport.cpp
    Settings.cpp
    Taxonomy.cpp
    Quality.cpp
)
    
target_link_libraries(
    GenomeLib
    UtilitiesLib
    ${MYSQL_LIBRARIES}
)
