#ifndef OMSimInputData_h
#define OMSimInputData_h 1

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4SystemOfUnits.hh"

// Helper class
class ParameterTable
{
public:
    ParameterTable() {}; // This class provides handling of table with json Trees
    G4bool CheckIfKeyInTable(G4String pKey);
    G4double GetValue(G4String pKey, G4String pParameter);
    G4String GetString(G4String pKey, G4String pParameter);
    void SetValue(G4String pKey, G4String pParameter, G4double pValue);
    void AppendParameterTable(G4String pFileName);
private:
    std::map<G4String, boost::property_tree::ptree> mTable;
};


class OMSimInputData : public ParameterTable
{
public:
    OMSimInputData();
    G4Material* GetMaterial(G4String name);
    G4OpticalSurface* GetOpticalSurface(G4String pName);
    // G4double GetValue(G4String pKey, G4String pParameter);
    // G4String GetString(G4String pKey, G4String pParameter);
    // void SetValue(G4String pKey, G4String pParameter, G4double pValue);
    void SearchFolders();
    std::map<G4String, G4OpticalSurface*> mOpticalSurfaceMap;

private:
    void ScannDataDirectory();
    G4String mDataDirectory;
};

#endif
//
