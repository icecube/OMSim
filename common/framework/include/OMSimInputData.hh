/** @file 
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
 * Its main use is as base class of InputDataManager, but you may use it as needed (see @ref ExampleUsage).
 * 
 * @ingroup common
 * 
 * @section ExampleUsage Example usage
 * @code
 * // Create an instance of the table
 * ParameterTable lTable;
 * 
 * // Load a JSON file into the table
 * lTable.appendAndReturnTree("path/to/file.json");
 * 
 * // Fetch a specific value using its key and parameter name
 * G4double lValue = lTable.getValue<G4double>("SomeKey", "SomeParameter");
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
    {
        const T lValue = mTable.at(pKey).get<T>(pParameter);
        return lValue;
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
    {
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
