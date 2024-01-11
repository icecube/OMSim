/** @file
 *  @brief Definition of ParameterTable and InputDataManager.
 * @ingroup common
 */

#ifndef OMSimInputData_h
#define OMSimInputData_h 1

#include "OMSimLogger.hh"
#include <G4OpBoundaryProcess.hh>
#include <G4SystemOfUnits.hh>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

/**
 * @class ParameterTable
 * @brief A utility class for managing JSON-based data tables.
 *
 * The ParameterTable class provides an interface for handling and querying JSON data
 * represented in the form of a property tree of the boost library. It facilitates the extraction of specific
 * parameters from loaded JSON files while also supporting units and scales. The class is
 * capable of loading multiple JSON datasets, each uniquely identified by a key,
 * and can perform actions such as fetching a value, checking the presence of a key,
 * and more.
 *
 * Internally, this class leverages the `boost::property_tree::ptree` to represent and
 * manage the JSON data structure, which makes it easy to traverse and fetch desired
 * parameters.
 *
 * Its main use is as base class of InputDataManager, but you may use it as needed (see @ref ExampleUsageParameterTable).
 *
 * @ingroup common
 *
 * @subsection ExampleUsageParameterTable Minimal example
 * @code
 * ParameterTable lTable; // Create an instance of the table
 * lTable.appendAndReturnTree("path/to/file.json"); // Load a JSON file into the table. The key of this file (in the following "SomeKey") is contained within the file under the variable "jName"
 * G4double lValue = lTable.getValue<G4double>("SomeKey", "SomeParameter"); // Fetch a specific value using its key and parameter name
 *
 * // Parsing array content from a JSON subtree into a vector using parseKeyContentToVector
 * std::vector<G4double> lVector;
 * pt::ptree lSubtree = lTable.getJSONTree("SomeKey"); // Retrieving the subtree of the file
 * lTable.parseKeyContentToVector(lVector, lSubtree, "keyOfArray", 1.0, false); // parsing array into vector
 * @endcode
 *
 * @warning It's essential to ensure that the provided JSON files are formatted correctly
 * and that the keys and parameters being queried exist within the loaded datasets
 * to avoid runtime errors or unexpected behavior.
 *
 * @note While the class provides type templating for fetching values, care must be taken
 * to ensure the correctness of the types.
 */
class ParameterTable
{
public:
    ParameterTable(){};

    /**
     * @brief Fetches a value from the table based on a key and parameter.
     *
     * @tparam T The type of the value to fetch.
     * @param pKey The main key.
     * @param pParameter The parameter within the main key's tree.
     * @return The value associated with the key and parameter.
     */
    template <typename T>
    T getValue(G4String pKey, G4String pParameter)
    {   log_trace("Fetching parameter {} in key {}", pParameter, pKey);
        try
        {
            const T lValue = mTable.at(pKey).get<T>(pParameter);
            return lValue;
        }

        catch (const std::exception &e)
        {
            log_error("Fetching parameter {} in key {} from table failed!", pParameter, pKey);
        }
    }

    G4bool checkIfKeyInTable(G4String pKey);
    G4double getValueWithUnit(G4String pKey, G4String pParameter);
    pt::ptree appendAndReturnTree(G4String pFileName);
    pt::ptree getJSONTree(G4String pKey);

    /**
     * @brief Parses the content of a JSON subtree into a vector, scaling values if necessary.
     *
     * @param pVector Vector where the values will be stored.
     * @param pTree JSON subtree to extract values from.
     * @param pKey JSON key whose associated array values are to be extracted.
     * @param pScaling Scaling factor applied to each value.
     * @param pInverse If true, the value is divided by the scaling factor, if false it's multiplied.
     *
     * @tparam T Type of the values to be extracted from the JSON tree.
     */
    template <typename T>
    void parseKeyContentToVector(std::vector<T> &pVector, pt::ptree pTree,
                                 std::basic_string<char> pKey, G4double pScaling,
                                 bool pInverse)
    {   log_trace("Parsing content in key {} to a vector", pKey);
        for (pt::ptree::value_type &ridx : pTree.get_child(
                 pKey))
        { // get array from element with key "pKey" of the json
            if (pInverse)
            { // if we need 1/x
                pVector.push_back(pScaling / ridx.second.get_value<T>());
            }
            else
            { // otherwise we only by scaling factor
                pVector.push_back(ridx.second.get_value<T>() * pScaling);
            }
        }
    };

    /**
     * @brief Parses the content of a JSON subtree into a vector, scaling values if necessary.
     *
     * This overloaded method additionally requires a map key to first retrieve the JSON subtree.
     *
     * @param pVector Vector where the values will be stored.
     * @param pMapKey Key of the JSON map to retrieve the desired subtree.
     * @param pKey JSON key within the subtree whose associated array values are to be extracted.
     * @param pScaling Scaling factor applied to each value.
     * @param pInverse If true, the value is divided by the scaling factor, if false it's multiplied.
     *
     * @tparam T Type of the values to be extracted from the JSON tree.
     */
    template <typename T>
    void parseKeyContentToVector(std::vector<T> &pVector,
                                 std::basic_string<char> pMapKey,
                                 std::basic_string<char> pKey, G4double pScaling,
                                 bool pInverse)
    {
        log_trace("Parsing content in key {} to a vector", pKey);
        for (pt::ptree::value_type &ridx : getJSONTree(pMapKey).get_child(
                 pKey))
        { // get array from element with key "pKey" of the json
            if (pInverse)
            { // if we need 1/x
                pVector.push_back(pScaling / ridx.second.get_value<T>());
            }
            else
            { // otherwise we only by scaling factor
                pVector.push_back(ridx.second.get_value<T>() * pScaling);
            }
        }
    };

private:
    std::map<G4String, boost::property_tree::ptree> mTable; ///< A table mapping keys to property trees.
    std::map<G4String, G4String> mKeyFileOrigin; ///< A table mapping keys to original file name.
};

/**
 * @class InputDataManager
 * @brief Manages the input data, including parsing and storing material properties.
 *
 * @details
 * The `InputDataManager` class extends the functionalities provided by the `ParameterTable` class.
 * It's dedicated to the specific needs of managing input data related to materials and optical properties
 * for the Geant4-based simulation.
 *
 * You probably should have a single instance of this class (no need of loading everything twice...probably it also would break things...).
 * Normally it is loaded in the main DetectorConstruction and passed to the other construction classes.
 *
 *
 * Example usage (see also @ref ExampleUsageParameterTable):
 *
 * @code
 * InputDataManager lManager;
 * lManager.searchFolders(); // Search for all recognized data files in the predefined directories
 * G4Material* lWater = manager.getMaterial("CustomWater"); // Retrieve a Geant4 material by name
 * G4OpticalSurface* lSurface = manager.getOpticalSurface("SomeSurfaceName"); // Retrieve an optical surface by name
 *
 * auto lData = manager.loadtxt("path/to/data.txt"); // Load data from a text file into a 2D vector
 * @endcode
 *
 * This class assumes certain conventions in naming and structuring the input files, which aids in automatically identifying and processing them. For example, files with names starting with "RiAbs" are treated as describing refractive and absorption properties.
 *
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
