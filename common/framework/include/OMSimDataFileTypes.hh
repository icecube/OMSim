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
 * - ReflectiveSurface: Defines a new reflective surface.
 *
 * Each class has an `extractInformation()` method that is responsible for creating the material or surface and extracting the necessary optical properties.
 *
 * @ingroup common
 */

#ifndef OMSimDataFileTypes_h
#define OMSimDataFileTypes_h 1

#include <G4NistManager.hh>
#include <G4OpBoundaryProcess.hh>
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
    /**
     *  @brief This method sorts two vectors (sortVector & referenceVector) based on the order of values in referenceVector.
     *
     *  @param referenceVector The vector of reference values. The ordering of these values will determine the final order of both vectors.
     *  @param sortVector The vector to be sorted according to the referenceVector.
     *
     *  @throws std::invalid_argument if the vectors do not have the same size.
     */
    void sortVectorByReference(std::vector<G4double> &referenceVector, std::vector<G4double> &sortVector);

    virtual void extractInformation() = 0;     // abstract method you have to define for a derived class
    const G4double mHC_eVnm = 1239.84193 * eV; // h*c in eV * nm
    boost::property_tree::ptree mJsonTree;
    ParameterTable *mFileData;
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
    abcMaterialData(G4String pFileName) : abcDataFile(pFileName){};
    G4Material *mMaterial;
    G4MaterialPropertiesTable *mMPT;
    G4NistManager *mMatDatBase;

    /**
     *  @brief Defines a new material from data in a json-file.
     *
     *  @details This method reads a JSON file containing material data, parses the data into a material properties table,
     *           and creates a new material in the G4NistManager.
     */
    void createMaterial();

    /**
     *  @brief Extracts absorption length data from a json-file and adds it to the material property table.
     */
    void extractAbsorptionLength();

    /**
     *  @brief Extracts refraction index data from a json-file and adds it to the material property table.
     */
    void extractRefractionIndex();

protected:
    /**
     *  @brief Converts a string representing a state of matter to a G4State.
     *  @param pState_str A string representing a state of matter. Should be one of "kStateSolid", "kStateLiquid", "kStateGas".
     *  @return A G4State representing the state of matter. Will be kStateUndefined if the input string does not match any known states.
     */
    G4State getState(G4String pState);
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
    RefractionAndAbsorption(G4String pFilename) : abcMaterialData(pFilename){};
    /**
     * @brief Extracts information and creates a material with refraction index and absorption length defined.
     */
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
    RefractionOnly(G4String pFilename) : abcMaterialData(pFilename){};
    /**
     * @brief Extracts information and creates a material with refraction index defined.
     *
     * This method is responsible for creating a material and extracting the refraction index,
     * which is then set to the material's properties table.
     */
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
    NoOptics(G4String pFilename) : abcMaterialData(pFilename){};
    /**
     * @brief Extracts information and creates a material without optical properties.
     *
     * This method is responsible for creating a material without any specific optical properties.
     */
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
    IceCubeIce(G4String pFilename) : abcMaterialData(pFilename){};

    /**
     * @brief Extracts information and creates ice with optical properties from IceCube.
     *
     * This method is responsible for creating a material based on the properties of IceCube's ice.
     * It creates two additional materials "IceCubeICE_SPICE" and "Mat_BubColumn", sets their properties,
     * and also handles the calculations and assignments for the material's properties such as
     * refractive index, absorption length, and mie scattering length.
     */
    void extractInformation();

private:
    G4int mSpiceDepth_pos = OMSimCommandArgsTable::getInstance().get<G4double>("depth_pos");
    /**
     * @brief Calculate temperature of ice depending on the depth.
     *
     * This function is needed for the calculation of scattering and absorption length of the ice.
     *
     * @param pDepth Depth in m from where we need the temperature.
     * @return Temperature in Kelvin.
     */
    G4double spiceTemperature(G4double depth);

    /**
     * @brief Calculate absorption length of IceCube's ice for a specific wavelength.
     *
     * @param pLambd Wavelength in nm.
     * @return Absorption length in m.
     */
    G4double spiceAbsorption(G4double pLambd);

    /**
     * @brief Calculate refraction index of IceCube's ice for a specific wavelength.
     *
     * @param pLambd Wavelength in nm.
     * @return Refraction index.
     */
    G4double spiceRefraction(G4double pLambd);

    /**
     * @brief Calculate mie scattering length of IceCube's ice for a specific wavelength.
     *
     * @param pLambd Wavelength in nm.
     * @return Mie scattering length in m.
     */
    G4double mieScattering(G4double pLambd);
    std::vector<G4double> mSpice_be400inv;
    std::vector<G4double> mSpice_a400inv;
    std::vector<G4double> mSpiceDepth;
    const G4double mMieSpiceConst[3] = {0.972, 0.0000001, 1};
    G4double mInnerColumn_b_inv = 3 * cm; // Eff. scattering lenght of bubble column (if placed)
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
    ReflectiveSurface(G4String pFilename) : abcDataFile(pFilename){};
    G4OpticalSurface *mOpticalSurface;

    /**
     * @brief Convert a string to a G4OpticalSurfaceFinish.
     *
     * @param  pFinish Finish in G4String format.
     * @return Finish in G4OpticalSurfaceFinish format.
     */
    G4OpticalSurfaceFinish getOpticalSurfaceFinish(G4String pFinish);

    /**
     * OpticalSurfaceModel in string to G4OpticalSurfaceModel
     * @param  G4String
     * @return G4OpticalSurfaceModel
     */
    G4OpticalSurfaceModel getOpticalSurfaceModel(G4String pModel);

    /**
     * SurfaceType in string to G4SurfaceType
     * @param  G4String
     * @return G4SurfaceType
     */
    G4SurfaceType getSurfaceType(G4String pType);

    /**
     * @brief Define a new reflective surface from data in a json-file.
     *
     * This method reads a json file, extracts information about an optical surface's properties,
     * creates a new optical surface and sets the properties.
     */
    void extractInformation();
};

#endif
//
