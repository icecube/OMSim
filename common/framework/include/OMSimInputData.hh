/** @file OMSimInputData.hh
 *  @brief Input data from external files are read and saved to memory.
 *
 *  All materials for the modules, pmts and environment, as well as the shape
 * parameters of the PMTs and Modules are in the ../common/data folder. This
 * class read all files and parse them to tables.
 */

#ifndef OMSimInputData_h
#define OMSimInputData_h 1

#include <G4OpBoundaryProcess.hh>
#include <G4SystemOfUnits.hh>
#include <boost/property_tree/json_parser.hpp>

namespace pt = boost::property_tree;

// Helper class
class ParameterTable
{
public:
    ParameterTable(){}; // This class provides handling of table with json Trees
    G4bool checkIfKeyInTable(G4String pKey);

    /**
     * Get values from a pTree with its unit, transforming it to a G4double
     * @param pKey Name of json tree in file
     * @param pParameter Name of parameter in json tree
     * @return G4double of the value and its unit
     */
    G4double getValueWithUnit(G4String pKey, G4String pParameter);
    template <typename T>
    T getValue(G4String pKey, G4String pParameter)
    {
        const T lValue = mTable.at(pKey).get<T>(pParameter);
        return lValue;
    }

    /**
     * Appends information of json-file containing PMT/OM parameters to a vector
     * of ptrees
     * @param pFileName Name of file containing json
     */
    pt::ptree appendAndReturnTree(G4String pFileName);

    /**
     * Get values from a pTree with its unit, transforming it to a G4double
     * @param pKey Name of json tree in file
     * @param pParameter Name of parameter in json tree
     * @return G4double of the value and its unit
     */
    pt::ptree getJSONTree(G4String pKey);

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
    }

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
    }

private:
    std::map<G4String, boost::property_tree::ptree> mTable;
};

class InputDataManager : public ParameterTable
{
public:
    InputDataManager();

    /**
     * Get a G4Material. In order to get custom built materials, method
     * searchFolders() should have already been called. Standard materials from
     * Geant4 are also transfered directly (if found through
     * FindOrBuildMaterial()). Materials defined by arguments of the main should
     * start with "arg" and then we get the material name from the arrays
     * depending on the global integers (gGel, gGlass, gEnvironment..) .
     * @param pName name of the (custom) material or "argVesselGlass", "argGel",
     * "argWorld" for argument materials
     * @return G4Material
     */
    G4Material *getMaterial(G4String name);

    /**
     * Get a G4OpticalSurface. In order to get custom built materials, method
     * searchFolders() should have already been called.
     * @param pName name of the optical surface or argument reflectors
     * "argReflector"
     * @return G4OpticalSurface
     */
    G4OpticalSurface *getOpticalSurface(G4String pName);

    /**
     * @brief Reads numerical data from a file and returns it as a 2D vector.
     * Similar to numpy.loadtxt.
     *
     * @param pFilePath The path to the input file.
     * @param pUnpack Optional. If true, the returned data is transposed, i.e.,
     * unpacked into columns. Default is true.
     * @param pSkipRows Optional. The number of lines to skip at the beginning of
     * the file. Default is 0.
     * @param pDelimiter The character used to separate values in each line of the
     * input file.
     * @return A 2D vector of doubles. The outer vector groups all columns (or
     * rows if 'unpack' is false), and each inner vector represents one of the
     * columns (or one of the rows if 'unpack' is false) of data file.
     * @throws std::runtime_error if the file cannot be opened.
     */
    static std::vector<std::vector<double>> loadtxt(const std::string &pFilePath,
                                                    bool pUnpack = true,
                                                    size_t pSkipRows = 0,
                                                    char pDelimiter = ' ');
    void searchFolders();
    std::map<G4String, G4OpticalSurface *> mOpticalSurfaceMap;

private:
    /**
     * Scann for data files inside mDataDirectory and process files.
     * @param pName name of the material
     */
    void scannDataDirectory();

    /**
     * @brief Processes a data file based on its name prefix.
     *
     * The function identifies the type of data file (RefractionAndAbsorption,
     * RefractionOnly, NoOptics, IceCubeIce, ReflectiveSurface or others) based on
     * the prefix of the filename. It then constructs an object of the appropriate
     * class and invokes its information extraction method. For 'Refl' prefixed
     * files, it also updates the mOpticalSurfaceMap. For 'pmt_', 'om_', and
     * 'usr_' prefixed files, it invokes directly appendAndReturnTree without any
     * extra parsing into Geant4 objects.
     *
     * @param pFilePath Full path to the file.
     * @param pFileName Name of the file (without the path).
     */
    void processFile(const std::string &filePath, const std::string &fileName);
    G4String mDataDirectory;
};

#endif
//
