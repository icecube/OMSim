#ifndef OMSimDataFileTypes_h
#define OMSimDataFileTypes_h 1

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4SystemOfUnits.hh"



class ParameterTable;

/**
 *  @class abcDataFile
 *  
 *  @brief This is an abstract base class that provides an interface for handling data files in the simulation.
 *  
 *  @details Derived classes should implement specific functionality for reading and writing different types of data files.
 *           This class contains a pointer to a ParameterTable, used for storing file data, and a string for the file name.
 *           
 *  @note This class is not meant to be instantiated directly, but should be subclassed. Methods in this class are to be
 *        overridden in the subclass for specific behavior.
 *  @ingroup common
 */
class abcDataFile
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


/**
 * @class abcMaterialData
 * @brief Abstract base class for material data extraction from a json file.
 * 
 * abcMaterialData class is derived from abcDataFile class. It is designed to manage
 * the material data, such as refractive index and absorption length.
 * 
 * @note This class is abstract and cannot be instantiated directly. Instead, use one
 * of the derived classes: RefractionAndAbsorption, RefractionOnly, NoOptics, or IceCubeIce.
 * 
 * The method extractInformation is a pure virtual function that should be implemented by the derived classes.
 * It is intended for the specific extraction of data required by each derived class.
 * @ingroup common
 */
class abcMaterialData : public abcDataFile
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


/**
 * @class   RefractionAndAbsorption
 * @brief   This class is responsible for handling materials with both a defined refractive index and absorption length.
 * @inherit abcMaterialData
 * @ingroup common
 */
class RefractionAndAbsorption : public abcMaterialData
{
public:
    RefractionAndAbsorption(G4String pFilename) : abcMaterialData(pFilename) {};
    void extractInformation();
};

/**
 * @class   RefractionOnly
 * @brief   This class is responsible for handling materials with only defined refractive index.
 * @inherit abcMaterialData
 * @ingroup common
 */
class RefractionOnly : public abcMaterialData
{
public:
    RefractionOnly(G4String pFilename) : abcMaterialData(pFilename) {};
    void extractInformation();
};

/**
 * @class   NoOptics
 * @brief   This class is responsible for handling materials without defined optical properties.
 * @inherit abcMaterialData
 * @ingroup common
 */
class NoOptics : public abcMaterialData
{
public:
    NoOptics(G4String pFilename) : abcMaterialData(pFilename) {};
    void extractInformation();
};

/**
 * @class   IceCubeIce
 * @brief   This class is responsible for the creation and property extraction of IceCube's ice.
 * @inherit abcMaterialData
 * @ingroup common
 */
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

/**
 * @class   ReflectiveSurface
 * @brief   This class is responsible for defining new reflective surfaces using data parsed from a JSON file.
 * @inherit abcDataFile
 * @ingroup common
 */
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
