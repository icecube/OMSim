#ifndef OMSimInputData_h
#define OMSimInputData_h 1

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4SystemOfUnits.hh"
namespace pt = boost::property_tree;

// Helper class
class ParameterTable
{
public:
    ParameterTable(){}; // This class provides handling of table with json Trees
    G4bool checkIfKeyInTable(G4String pKey);
    G4double getValueWithUnit(G4String pKey, G4String pParameter);
    template <typename T>
    T getValue(G4String pKey, G4String pParameter)
    {
        const T lValue = mTable.at(pKey).get<T>(pParameter);
        return lValue;
    }
    pt::ptree appendAndReturnTree(G4String pFileName);
    pt::ptree getJSONTree(G4String pKey);

    template <typename T>
    void parseKeyContentToVector(std::vector<T> &pVector, pt::ptree pTree, std::basic_string<char> pKey, G4double pScaling, bool pInverse)
    {
        for (pt::ptree::value_type &ridx : pTree.get_child(pKey))
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
    void parseKeyContentToVector(std::vector<T> &pVector, std::basic_string<char> pMapKey, std::basic_string<char> pKey, G4double pScaling, bool pInverse)
    {
        for (pt::ptree::value_type &ridx : getJSONTree(pMapKey).get_child(pKey))
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
    G4Material *getMaterial(G4String name);
    G4OpticalSurface *getOpticalSurface(G4String pName);
    static std::vector<std::vector<double>> loadtxt(const std::string &pFilePath, bool pUnpack = true, size_t pSkipRows = 0, char pDelimiter = ' ');
    void searchFolders();
    std::map<G4String, G4OpticalSurface *> mOpticalSurfaceMap;

private:
    void scannDataDirectory();
    void processFile(const std::string& filePath, const std::string& fileName);
    G4String mDataDirectory;
};

#endif
//
