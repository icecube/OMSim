/** @file OMSimInputData.hh
 *  @brief Input data from external files are read and saved to memory.
 *
 *  All materials for the modules, pmts and environment, as well as the shape
 * parameters of the PMTs and Modules are in the ../common/data folder. This
 * class read all files and parse them to tables.
 * @ingroup common
 */

#ifndef OMSimInputData_h
#define OMSimInputData_h 1

#include <G4OpBoundaryProcess.hh>
#include <G4SystemOfUnits.hh>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

/**
 * @class ParameterTable
 * @brief A helper class that provides functionalities for handling JSON-based data tables.
 * @ingroup common
 */
class ParameterTable
{
public:
    ParameterTable(){}; 

    template <typename T>
    T getValue(G4String pKey, G4String pParameter);
    G4bool checkIfKeyInTable(G4String pKey);
    G4double getValueWithUnit(G4String pKey, G4String pParameter);
    pt::ptree appendAndReturnTree(G4String pFileName);
    pt::ptree getJSONTree(G4String pKey);

    template <typename T>
    void parseKeyContentToVector(std::vector<T> &pVector, pt::ptree pTree,
                                 std::basic_string<char> pKey, G4double pScaling,
                                 bool pInverse);

    template <typename T>
    void parseKeyContentToVector(std::vector<T> &pVector,
                                 std::basic_string<char> pMapKey,
                                 std::basic_string<char> pKey, G4double pScaling,
                                 bool pInverse);
private:
    std::map<G4String, boost::property_tree::ptree> mTable; ///< A table mapping keys to property trees.
};



/**
 * @class InputDataManager
 * @brief Manages the input data, including parsing and storing material properties.
 * @ingroup common
 */
class InputDataManager : public ParameterTable
{
public:
    InputDataManager();
    G4Material *getMaterial(G4String name);
    G4OpticalSurface *getOpticalSurface(G4String pName);
    static std::vector<std::vector<double>> loadtxt(const std::string &pFilePath,
                                                    bool pUnpack = true,
                                                    size_t pSkipRows = 0,
                                                    char pDelimiter = ' ');
    void searchFolders();
    std::map<G4String, G4OpticalSurface *> mOpticalSurfaceMap; ///< Map that links names with optical surfaces.

private:

    void scannDataDirectory();
    void processFile(const std::string &filePath, const std::string &fileName);
    G4String mDataDirectory; ///< The current directory being scanned for data.
};

#endif
//
