/**
 * @file   OMSimDataFileTypes.hh
 * @brief  Collection of classes and methods to handle material creation from user data in json files.
 *
 * This file includes the class definitions for the abstract base classes (abcDataFile and abcMaterialData) and derived classes related to material properties.
 * The derived classes and functions are designed to handle different categories of materials and reflective surfaces with varying optical properties.
 *
 * Derived Classes:
 * - RefractionAndAbsorption: Handles materials with refraction index and absorption length defined.
 * - RefractionOnly: Handles materials with only refraction index defined.
 * - NoOptics: Handles materials without optical properties.
 * - IceCubeIce: Handles creation and property extraction of IceCube's ice.
 * - Surface: Defines a new reflective surface.
 *
 * Each class has an `extractInformation()` method that is responsible for creating the material or surface and extracting the necessary optical properties.
 *
 * @ingroup common
 */

#ifndef OMSimDataFileTypes_h
#define OMSimDataFileTypes_h 1

#include "OMSimOpBoundaryProcess.hh"

#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <boost/property_tree/json_parser.hpp>
#include <OMSimCommandArgsTable.hh>

class ParameterTable;

/**
 *  @class abcDataFile
 *
 *  @brief This is an abstract base class that provides an interface for handling data files in the simulation.
 *
 *  @details Derived classes should implement specific functionality for reading and writing different types of data files.
 *           This class contains a pointer to a ParameterTable, used for storing file data, and a string for the file name.
 *  @ingroup common
 */
class abcDataFile
{
public:
    abcDataFile(G4String pFileName);
    ~abcDataFile();
    G4String mFileName;
    G4String mObjectName;

protected:
    void sortVectorByReference(std::vector<G4double> &referenceVector, std::vector<G4double> &sortVector);

    virtual void extractInformation() = 0;     ///< abstract method you have to define for a derived class
    const G4double mHC_eVnm = 1239.84193 * eV; ///< h*c in eV * nm
    boost::property_tree::ptree mJsonTree;
    ParameterTable *mFileData;
};

/**
 * @class abcMaterialData
 * @brief Abstract base class for material data extraction from a json file.
 * abcMaterialData class is derived from abcDataFile class. It is designed to manage
 * the material data, such as refractive index and absorption length.
 * @ingroup common
 */
class abcMaterialData : public abcDataFile
{
public:
    abcMaterialData(G4String pFileName) : abcDataFile(pFileName){};
    G4Material *mMaterial;
    G4MaterialPropertiesTable *mMPT;

    void createMaterial();
    void extractAbsorptionLength();
    void extractRefractionIndex();

protected:
    G4State getState(G4String pState);
    virtual void extractInformation() = 0; ///< abstract method that has to be defined in derived classes
};

// Derived Classes

/**
 * @class   RefractionAndAbsorption
 * @brief   Materials with defined refractive index and absorption length.
 * @inherit abcMaterialData
 * @ingroup common
 */
class RefractionAndAbsorption : public abcMaterialData
{
public:
    RefractionAndAbsorption(G4String pFilename) : abcMaterialData(pFilename){};
    void extractInformation();
};

/**
 * @class   RefractionOnly
 * @brief   Materials only with refractive index defined.
 * @inherit abcMaterialData
 * @ingroup common
 */
class RefractionOnly : public abcMaterialData
{
public:
    RefractionOnly(G4String pFilename) : abcMaterialData(pFilename){};
    void extractInformation();
};

/**
 * @class   NoOptics
 * @brief   Materials without optical properties defined.
 * @inherit abcMaterialData
 * @ingroup common
 */
class NoOptics : public abcMaterialData
{
public:
    NoOptics(G4String pFilename) : abcMaterialData(pFilename){};
    void extractInformation();
};

/**
 * @class   IceCubeIce
 * @brief   Creation and extraction of IceCube's ice optical properties.
 * @inherit abcMaterialData
 * @ingroup common
 */
class IceCubeIce : public abcMaterialData
{
public:
    IceCubeIce(G4String pFilename) : abcMaterialData(pFilename){};

    void extractInformation();

private:
    int mSpiceDepth_pos;

    G4double spiceTemperature(G4double pDepth);
    G4double spiceAbsorption(G4double pLambd);
    G4double spiceRefraction(G4double pLambd);
    G4double mieScattering(G4double pLambd);

    std::vector<G4double> mSpice_be400inv;
    std::vector<G4double> mSpice_a400inv;
    std::vector<G4double> mSpiceDepth;
    const G4double mMieSpiceConst[3] = {0.972, 0.0000001, 1};
    G4double mInnerColumn_b_inv = 3 * cm; ///< Eff. scattering lenght of bubble column (if placed)
};

/**
 * @class   Surface
 * @brief   Reflective surfaces parsed from a JSON file.
 * @inherit abcDataFile
 * @ingroup common
 */
class Surface : public abcDataFile
{
public:
    Surface(G4String pFilename) : abcDataFile(pFilename){};
    G4OpticalSurface *mOpticalSurface;

    G4OpticalSurfaceFinish getOpticalSurfaceFinish(G4String pFinish);
    G4OpticalSurfaceModel getOpticalSurfaceModel(G4String pModel);
    G4SurfaceType getSurfaceType(G4String pType);
    void extractInformation();
};

/**
 * @class ScintillationProperties
 * @brief Scintillation properties extraction for existing materials.
 * @ingroup common
 */
class ScintillationProperties : public abcDataFile
{
public:
    ScintillationProperties(G4String pFilename) : abcDataFile(pFilename){};
    void extractInformation();

private:
    void extractSpectrum();
    void extractLifeTimes(G4String pTemperature);
    void getLifeTimeTemperatureRange(double &pMinTemp, double &pMaxTemp);
    std::pair<std::vector<G4double>, std::vector<G4double>> extractLifeTimesForTemperature(G4String pTemperature);
    void weightLifeTimesAmplitudes(std::vector<G4double> &pAmplitudes, double pT1, double pT2);
    void extractYieldAlpha(G4String pTemperature);
    void extractYieldElectron(G4String pTemperature);
    void extractYield(G4String pTemperature, G4String pYieldPropertyName, G4String pArgKey, G4String pTreeKeyTemperature, G4String pTreeKeyYield);
    void findMPT();
    G4MaterialPropertiesTable *mMPT;
};

/**
 * @class   CustomProperties
 * @brief   Adds user defined properties to already defined materials
 * @ingroup common
 */
class CustomProperties : public abcDataFile
{
public:
    CustomProperties(G4String pFilename) : abcDataFile(pFilename){};
    void extractInformation();

private:
    void extractConstProperties();
    void extractProperties();
    void findMPT();
    G4MaterialPropertiesTable *mMPT;
};

#endif
//
