/** @file
 *  @brief Definition of ParameterTable and OMSimInputData.
 * @ingroup common
 */

#pragma once

#include "OMSimLogger.hh"
#include "OMSimOpBoundaryProcess.hh"
#include <G4SystemOfUnits.hh>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

/**
 * @class ParameterTable
 * @brief A utility class for managing JSON-based data tables.
 *
 * Interface for handling and querying data in the form of a property tree of the boost library. It facilitates the extraction of specific
 * parameters from loaded JSON files while also supporting units and scales.
 * 
 * Its main use is as base class of OMSimInputData, but you may use it as needed (see @ref ExampleUsageParameterTable).
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
        log_trace("Fetching parameter {} in key {}", pParameter, pKey);
        try
        {
            const T lValue = m_table.at(pKey).get<T>(pParameter);
            return lValue;
        }

        catch (const std::exception &e)
        {
            log_error("Fetching parameter {} in key {} from table failed!", pParameter, pKey);
            throw std::runtime_error("Fetching parameter failed");
        }
    }

    G4bool checkIfTreeNameInTable(G4String pKey);
    G4bool checkIfKeyInTree(G4String p_treeName, G4String p_key);
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
        log_trace("Parsing content in key {} to a vector", pKey);
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
     * @param p_MapKey Key of the JSON map to retrieve the desired subtree.
     * @param pKey JSON key within the subtree whose associated array values are to be extracted.
     * @param pScaling Scaling factor applied to each value.
     * @param pInverse If true, the value is divided by the scaling factor, if false it's multiplied.
     *
     * @tparam T Type of the values to be extracted from the JSON tree.
     */
    template <typename T>
    void parseKeyContentToVector(std::vector<T> &pVector,
                                 std::basic_string<char> p_MapKey,
                                 std::basic_string<char> pKey, G4double pScaling,
                                 bool pInverse)
    {
        log_trace("Parsing content in key {} to a vector", pKey);

        // Get the JSON tree
        boost::property_tree::ptree lTree = getJSONTree(p_MapKey);

        // Check if the tree is empty
        if (lTree.empty())
        {
            log_warning("JSON tree is empty for key {}", p_MapKey);
            return;
        }

        // Access the child node with the key "pKey"
        auto lChildNode = lTree.get_child_optional(pKey);

        // Check if the child node exists
        if (!lChildNode)
        {
            log_warning("Child node with key {} not found in JSON tree", pKey);
            return;
        }

        // Iterate over the child nodes and parse the content
        for (const auto &ridx : *lChildNode)
        {
            if (pInverse)
            {
                // if we need 1/x
                pVector.push_back(pScaling / ridx.second.get_value<T>());
            }
            else
            {
                // otherwise we only multiply by scaling factor
                pVector.push_back(ridx.second.get_value<T>() * pScaling);
            }
        }
    };

private:
    std::map<G4String, boost::property_tree::ptree> m_table; ///< A table mapping keys to property trees.
    std::map<G4String, G4String> m_keyToFileName;            ///< A table mapping keys to original file name.
};

/**
 * @class OMSimInputData
 * @brief Manages the input data, including parsing and storing material properties.
 *
 * @details
 * Extends the functionalities provided by the `ParameterTable` class.
 * It's dedicated to the specific needs of managing input data related to materials and optical properties.
 *
 * This class follows a global instance pattern and its lifecycle is managed by OMSimDetectorConstruction. It can be accessed by other classes using OMSimInputData::getInstance().
 * For example:
 *
 * @code
 * G4Material* lWater = OMSimInputData::getInstance().getMaterial("CustomWater"); // Retrieve a Geant4 material by name
 * G4OpticalSurface* lSurface = OMSimInputData::getInstance().getOpticalSurface("SomeSurfaceName"); // Retrieve an optical surface by name
 * @endcode
 *
 * (see also @ref ExampleUsageParameterTable)
 * This class assumes certain conventions in naming and structuring the input files, which aids in automatically identifying and processing them.
 *
 * @ingroup common
 */
class OMSimInputData : public ParameterTable
{
public:
    static void init();
    static void shutdown();
    static OMSimInputData& getInstance();
    G4Material *getMaterial(G4String pName);
    G4OpticalSurface *getOpticalSurface(G4String pName);
    void searchFolders();
    std::map<G4String, G4OpticalSurface *> m_opticalSurfaceMap; ///< Map that links names with optical surfaces.

private:
    enum class FileType {
        IceCubeICE,
        Scintillator,
        Extra,
        Table,
        Surface,
        Material
    };

    static const std::unordered_map<std::string, FileType> fileTypePrefixes;
    FileType getFileType(const std::string& fileName) const;

    OMSimInputData() = default;
    ~OMSimInputData() = default;
    OMSimInputData(const OMSimInputData&) = delete;
    OMSimInputData& operator=(const OMSimInputData&) = delete;
    void scannDataDirectory();
    void processFile(const std::string &filePath, const std::string &fileName);
    G4String m_dataDirectory; ///< The current directory being scanned for data.
};

inline OMSimInputData* g_OMSimInputData = nullptr;

