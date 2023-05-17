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
    G4bool CheckIfKeyInTable(G4String pKey);
    G4double GetValueWithUnit(G4String pKey, G4String pParameter);
    template <typename T>
    T GetValue(G4String pKey, G4String pParameter)
    {
        const T lValue = mTable.at(pKey).get<T>(pParameter);
        return lValue;
    }
    pt::ptree AppendAndReturnTree(G4String pFileName);
    pt::ptree GetJSONTree(G4String pKey);

    template <typename T>
    void ParseKeyContentToVector(std::vector<T> &pVector, pt::ptree pTree, std::basic_string<char> pKey, G4double pScaling, bool pInverse)
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
    void ParseKeyContentToVector(std::vector<T> &pVector, std::basic_string<char> pMapKey, std::basic_string<char> pKey, G4double pScaling, bool pInverse)
    {
        for (pt::ptree::value_type &ridx : GetJSONTree(pMapKey).get_child(pKey))
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

class OMSimInputData : public ParameterTable
{
public:
    OMSimInputData();
    G4Material *GetMaterial(G4String name);
    G4OpticalSurface *GetOpticalSurface(G4String pName);
    void SearchFolders();
    std::map<G4String, G4OpticalSurface *> mOpticalSurfaceMap;

private:
    void ScannDataDirectory();
    G4String mDataDirectory;
};

#endif
//
