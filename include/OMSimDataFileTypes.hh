#ifndef OMSimDataFileTypes_h
#define OMSimDataFileTypes_h 1

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4SystemOfUnits.hh"



class ParameterTable;

class abcDataFile // Base abstract class for data file template
{
public:
    abcDataFile(G4String pFileName);
    G4String mFileName;
    G4String mObjectName;

protected:
    void sortVectorByReference(std::vector<G4double>& referenceVector, std::vector<G4double>& sortVector);
    virtual void extractInformation() = 0;     // abstract method you have to define for a derived class
    const G4double mHC_eVnm = 1239.84193 * eV; // h*c in eV * nm
    boost::property_tree::ptree mJsonTree;
    ParameterTable* mFileData;
};

class abcMaterialData : public abcDataFile // Base abstract class for material data file template
{
public:
    abcMaterialData(G4String pFileName) : abcDataFile(pFileName) {};
    G4Material* mMaterial;
    G4MaterialPropertiesTable* mMPT;
    G4NistManager* mMatDatBase;
    void createMaterial();
    void extractAbsorptionLength();
    void extractRefractionIndex();
protected:

    G4State GetState(G4String pState);
    virtual void extractInformation() = 0; // abstract method
};

// Derived Classes
class RefractionAndAbsorption : public abcMaterialData
{
public:
    RefractionAndAbsorption(G4String pFilename) : abcMaterialData(pFilename) {};
    void extractInformation();
};
class RefractionOnly : public abcMaterialData
{
public:
    RefractionOnly(G4String pFilename) : abcMaterialData(pFilename) {};
    void extractInformation();
};
class NoOptics : public abcMaterialData
{
public:
    NoOptics(G4String pFilename) : abcMaterialData(pFilename) {};
    void extractInformation();
};

class IceCubeIce : public abcMaterialData
{
public:
    IceCubeIce(G4String pFilename) : abcMaterialData(pFilename) {};
    void extractInformation();
private:
    G4int mSpiceDepth_pos = 65; //88 depth = 2278.2 m, very clean ice, 
    G4double spiceTemperature(G4double depth);
    G4double spiceAbsorption(G4double pLambd);
    G4double spiceRefraction(G4double pLambd);
    G4double mieScattering(G4double pLambd);
    std::vector<G4double> mSpice_be400inv;
    std::vector<G4double> mSpice_a400inv;
    std::vector<G4double> mSpiceDepth;
    const G4double mMieSpiceConst[3] = { 0.972, 0.0000001, 1 };
    G4double mInnerColumn_b_inv = 3*cm; //Eff. scattering lenght of bubble column (if placed)
};


class ReflectiveSurface : public abcDataFile
{
public:
    ReflectiveSurface(G4String pFilename) : abcDataFile(pFilename) {};
    G4OpticalSurface* mOpticalSurface;
    G4OpticalSurfaceFinish getOpticalSurfaceFinish(G4String pFinish);
    G4OpticalSurfaceModel getOpticalSurfaceModel(G4String pModel);
    G4SurfaceType getSurfaceType(G4String pType);
    void extractInformation();
};

#endif
//
