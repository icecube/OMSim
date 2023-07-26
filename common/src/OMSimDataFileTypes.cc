/**
 * @file   OMSimDataFileTypes.cc
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
 * @author Martin Unland
 * @todo Check ReflectiveSurface::extractInformation(), code look suspicious.
 */


#include "OMSimDataFileTypes.hh"
#include "OMSimInputData.hh"
#include "OMSimLogger.hh"

#include <numeric>

namespace pt = boost::property_tree;

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                  Base Abstract Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 *  @brief The constructor for the abcDataFile class, which initializes a new instance of abcDataFile.
 *  @param pFileName The name of the file to be processed.
 */
abcDataFile::abcDataFile(G4String pFileName)
{
    mFileData = new ParameterTable();
    mFileName = pFileName;
}


/**
 *  @brief This method sorts two vectors (sortVector & referenceVector) based on the order of values in referenceVector.
 *  
 *  @param referenceVector The vector of reference values. The ordering of these values will determine the final order of both vectors.
 *  @param sortVector The vector to be sorted according to the referenceVector.
 *  
 *  @throws std::invalid_argument if the vectors do not have the same size.
 */
void abcDataFile::sortVectorByReference(std::vector<G4double> &referenceVector, std::vector<G4double> &sortVector)
{
    // Check if the vectors have the same size
    if (referenceVector.size() != sortVector.size())
    {
        // Handle error
        throw std::invalid_argument("The two vectors must have the same size.");
    }

    // Create a vector of indices
    std::vector<std::size_t> indices(referenceVector.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Sort the indices based on the values in referenceVector
    std::sort(indices.begin(), indices.end(),
              [&referenceVector](std::size_t i1, std::size_t i2)
              { return referenceVector[i1] < referenceVector[i2]; });

    // Create temporary vectors to hold the sorted data
    std::vector<G4double> sortedSortVector(sortVector.size());
    std::vector<G4double> sortedReferenceVector(referenceVector.size());

    // Apply the sorted indices to the vectors
    for (std::size_t i = 0; i < indices.size(); ++i)
    {
        sortedSortVector[i] = sortVector[indices[i]];
        sortedReferenceVector[i] = referenceVector[indices[i]];
    }

    // Replace the original vectors with the sorted ones
    sortVector = std::move(sortedSortVector);
    referenceVector = std::move(sortedReferenceVector);
}



/**
 *  @brief Defines a new material from data in a json-file.
 *  
 *  @details This method reads a JSON file containing material data, parses the data into a material properties table, 
 *           and creates a new material in the G4NistManager.
 */
void abcMaterialData::createMaterial()
{
    mJsonTree = mFileData->appendAndReturnTree(mFileName);
    
    mMPT = new G4MaterialPropertiesTable();
    mMatDatBase = G4NistManager::Instance();

    mObjectName = mJsonTree.get<G4String>("jName");
    const G4String lDataType = mJsonTree.get<G4String>("jDataType");

    const G4double lDensity = mFileData->getValueWithUnit(mObjectName, "jDensity");

    const G4String lState_str = mJsonTree.get<G4String>("jState");
    const G4State lState = GetState(lState_str);

    // Defining the material with its density, number of components, state and name
    mMaterial = new G4Material(mObjectName, lDensity, mJsonTree.get_child("jComponents").size(), lState);

    // Construct material with fractional components (isotopes or G4-Materials)
    for (pt::ptree::value_type &key : mJsonTree.get_child("jComponents"))
    {
        std::string componentName = key.first;
        double componentFraction = key.second.get_value<double>();
        mMaterial->AddMaterial(mMatDatBase->FindOrBuildMaterial(componentName), componentFraction);
    }
    G4String mssg = "New Material defined: " + mMaterial->GetName();
    log_debug(mssg);
}


/**
 *  @brief Extracts absorption length data from a json-file and adds it to the material property table.
 */
void abcMaterialData::extractAbsorptionLength()
{
    std::vector<G4double> lAbsLength;
    std::vector<G4double> lAbsLengthEnergy;
    mFileData->parseKeyContentToVector(lAbsLength, mJsonTree, "jAbsLength", 1 * mm, false);
    mFileData->parseKeyContentToVector(lAbsLengthEnergy, mJsonTree, "jAbsLengthWavelength", mHC_eVnm, true);
    sortVectorByReference(lAbsLengthEnergy, lAbsLength);
    mMPT->AddProperty("ABSLENGTH", &lAbsLengthEnergy[0], &lAbsLength[0], static_cast<int>(lAbsLength.size()));
}


/**
 *  @brief Extracts refraction index data from a json-file and adds it to the material property table.
 */
void abcMaterialData::extractRefractionIndex()
{
    std::vector<G4double> lRefractionIndex;
    std::vector<G4double> lRefractionIndexEnergy;
    mFileData->parseKeyContentToVector(lRefractionIndex, mJsonTree, "jRefractiveIdx", 1., false);
    mFileData->parseKeyContentToVector(lRefractionIndexEnergy, mJsonTree, "jRefractiveIdxWavelength", mHC_eVnm, true);
    sortVectorByReference(lRefractionIndexEnergy, lRefractionIndex);
    mMPT->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
}


/**
 *  @brief Converts a string representing a state of matter to a G4State.
 *  @param pState_str A string representing a state of matter. Should be one of "kStateSolid", "kStateLiquid", "kStateGas".
 *  @return A G4State representing the state of matter. Will be kStateUndefined if the input string does not match any known states.
 */
G4State abcMaterialData::GetState(G4String pState_str)
{
    G4State lState;
    if (pState_str == "kStateSolid")
        lState = kStateSolid;
    else if (pState_str == "kStateLiquid")
        lState = kStateLiquid;
    else if (pState_str == "kStateGas")
        lState = kStateGas;
    else
        lState = kStateUndefined;
    return lState;
}





/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                     Derived Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */


/**
 * @brief Extracts information and creates a material with refraction index and absorption length defined.
 */
void RefractionAndAbsorption::extractInformation()
{
    createMaterial();
    extractAbsorptionLength();
    extractRefractionIndex();

    mMaterial->SetMaterialPropertiesTable(mMPT);
}

/**
 * @brief Extracts information and creates a material with refraction index defined.
 *
 * This method is responsible for creating a material and extracting the refraction index, 
 * which is then set to the material's properties table.
 */
void RefractionOnly::extractInformation()
{
    createMaterial();
    extractRefractionIndex();
    mMaterial->SetMaterialPropertiesTable(mMPT);
}

/**
 * @brief Extracts information and creates a material without optical properties.
 *
 * This method is responsible for creating a material without any specific optical properties.
 */
void NoOptics::extractInformation()
{
    createMaterial();
}

/**
 * @brief Extracts information and creates ice with optical properties from IceCube.
 *
 * This method is responsible for creating a material based on the properties of IceCube's ice. 
 * It creates two additional materials "IceCubeICE_SPICE" and "Mat_BubColumn", sets their properties,
 * and also handles the calculations and assignments for the material's properties such as 
 * refractive index, absorption length, and mie scattering length.
 */
void IceCubeIce::extractInformation()
{
    createMaterial(); // creates IceCubeICE

    G4Material *lIceMie = new G4Material("IceCubeICE_SPICE", mFileData->getValueWithUnit(mObjectName, "jDensity"), mMatDatBase->FindOrBuildMaterial("G4_WATER"), kStateSolid);      // create IceCubeICE_SPICE
    G4Material *lBubleColumnMie = new G4Material("Mat_BubColumn", mFileData->getValueWithUnit(mObjectName, "jDensity"), mMatDatBase->FindOrBuildMaterial("G4_WATER"), kStateSolid); // create IceCubeICE_SPICE
    std::vector<G4double> lMieScatteringLength;
    std::vector<G4double> lMieScatteringLength_BubleColumn;
    std::vector<G4double> lWavelength;
    std::vector<G4double> lRefractionIndex;
    std::vector<G4double> lRefractionIndexEnergy;
    std::vector<G4double> lAbsLength;
    mFileData->parseKeyContentToVector(lRefractionIndexEnergy, mJsonTree, "jWavelength_spice", mHC_eVnm, true);
    mFileData->parseKeyContentToVector(lWavelength, mJsonTree, "jWavelength_spice", 1 * nm, false);
    mFileData->parseKeyContentToVector(mSpice_be400inv, mJsonTree, "jbe400inv_spice", 1 * m, false);
    mFileData->parseKeyContentToVector(mSpice_a400inv, mJsonTree, "ja400inv_spice", 1 * m, false);
    mFileData->parseKeyContentToVector(mSpiceDepth, mJsonTree, "jDepth_spice", 1 * m, false);

    for (int u = 0; u < static_cast<int>(lRefractionIndexEnergy.size()); u++)
    {
        lRefractionIndex.push_back(spiceRefraction(lWavelength.at(u)));
        lAbsLength.push_back(spiceAbsorption(lWavelength.at(u)));
        lMieScatteringLength.push_back(mieScattering(lWavelength.at(u)));
        lMieScatteringLength_BubleColumn.push_back(mInnerColumn_b_inv);
    }
    // give refractive index to IceCubeICE. This is used also for IceCubeICE_SPICE
    mMPT->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
    mMaterial->SetMaterialPropertiesTable(mMPT);

    // give properties to IceCubeICE_SPICE
    G4MaterialPropertiesTable *lMPT_spice = new G4MaterialPropertiesTable();
    lMPT_spice->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
    lMPT_spice->AddProperty("ABSLENGTH", &lRefractionIndexEnergy[0], &lAbsLength[0], static_cast<int>(lAbsLength.size()));
    lMPT_spice->AddProperty("MIEHG", &lRefractionIndexEnergy[0], &lMieScatteringLength[0], static_cast<int>(lRefractionIndex.size())); //->SetSpline(true);
    lMPT_spice->AddConstProperty("MIEHG_FORWARD", mMieSpiceConst[0]);
    lMPT_spice->AddConstProperty("MIEHG_BACKWARD", mMieSpiceConst[1]);
    lMPT_spice->AddConstProperty("MIEHG_FORWARD_RATIO", mMieSpiceConst[2]);
    lIceMie->SetMaterialPropertiesTable(lMPT_spice);
    G4String mssg = "Optical ice properties calculated for depth " + std::to_string(mSpiceDepth[mSpiceDepth_pos] / m) + " m.";
    log_info(mssg);
    // now give the properties to the bubble column, which are basically the same ones but with the chosen scattering lenght
    G4MaterialPropertiesTable *lMPT_holeice = new G4MaterialPropertiesTable();
    lMPT_holeice->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
    lMPT_holeice->AddProperty("ABSLENGTH", &lRefractionIndexEnergy[0], &lAbsLength[0], static_cast<int>(lAbsLength.size()));
    lMPT_holeice->AddProperty("MIEHG", &lRefractionIndexEnergy[0], &lMieScatteringLength_BubleColumn[0], static_cast<int>(lRefractionIndex.size())); //->SetSpline(true);
    lMPT_holeice->AddConstProperty("MIEHG_FORWARD", mMieSpiceConst[0]);
    lMPT_holeice->AddConstProperty("MIEHG_BACKWARD", mMieSpiceConst[1]);
    lMPT_holeice->AddConstProperty("MIEHG_FORWARD_RATIO", mMieSpiceConst[2]);
    lBubleColumnMie->SetMaterialPropertiesTable(lMPT_holeice);
}
/*
 * %%%%%%%%%%%%%%%% Functions for icecube ice optical properties %%%%%%%%%%%%%%%%
 */

/**
 * @brief Calculate temperature of ice depending on the depth.
 *
 * This function is needed for the calculation of scattering and absorption length of the ice.
 *
 * @param pDepth Depth in m from where we need the temperature.
 * @return Temperature in Kelvin.
 */
G4double IceCubeIce::spiceTemperature(G4double pDepth)
{
    G4double spice_temp = 221.5 - 0.00045319 / m * pDepth + 5.822e-6 / m2 * pow(pDepth, 2.);
    return spice_temp;
}

/**
 * @brief Calculate absorption length of IceCube's ice for a specific wavelength.
 *
 * @param pLambd Wavelength in nm.
 * @return Absorption length in m.
 */
G4double IceCubeIce::spiceAbsorption(G4double pLambd)
{
    G4double lKappa = 1.08;
    G4double lParamA = 6954. / m;
    G4double lParamB = 6618 * nm;
    G4double lAdust = 1. / (mSpice_a400inv[mSpiceDepth_pos]) * pow(pLambd / (400. * nm), -lKappa);
    G4double lDeltaTau = spiceTemperature(mSpiceDepth[mSpiceDepth_pos]) - spiceTemperature(1730.);
    G4double la_inv = 1. / (lAdust + lParamA * exp(-lParamB / pLambd) * (1. + 0.01 * lDeltaTau));
    return la_inv;
}

/**
 * @brief Calculate refraction index of IceCube's ice for a specific wavelength.
 *
 * @param pLambd Wavelength in nm.
 * @return Refraction index.
 */
G4double IceCubeIce::spiceRefraction(G4double pLambd)
{
    // unknown depth. Parametrization by Thomas Kittler.
    G4double lLambd3 = pLambd * 1e-3;
    G4double lNphase = 1.55749 - 1.57988 / nm * lLambd3 + 3.99993 / (nm * nm) * pow(lLambd3, 2) - 4.68271 / (nm * nm * nm) * pow(lLambd3, 3) + 2.09354 / (nm * nm * nm * nm) * pow(lLambd3, 4);
    return lNphase; // using this now after discussion with Timo
}


/**
 * @brief Calculate mie scattering length of IceCube's ice for a specific wavelength.
 *
 * @param pLambd Wavelength in nm.
 * @return Mie scattering length in m.
 */
G4double IceCubeIce::mieScattering(G4double pLambd)
{
    // depth_pos is the coordinate for the chosen depth in Depth_spice. For example to choose
    // depth=2278.2 m, we use depth_pos = 88
    G4double lAlpha = 0.90;
    G4double lAv_costheta = 0.9;
    G4double lBe_inv = 1. / (1. / (mSpice_be400inv[mSpiceDepth_pos]) * pow((pLambd / (400. * nm)), -lAlpha));
    G4double lB_inv = lBe_inv * (1. - lAv_costheta);
    return lB_inv;
}

/**
 * @brief Define a new reflective surface from data in a json-file.
 *
 * This method reads a json file, extracts information about an optical surface's properties, 
 * creates a new optical surface and sets the properties.
 */
void ReflectiveSurface::extractInformation()
{

    pt::read_json(mFileName, mJsonTree); // read json file into mJsonTree

    mObjectName = mJsonTree.get<G4String>("jName");
    G4String lModelStr = mJsonTree.get<G4String>("jModel");
    G4String lFinishStr = mJsonTree.get<G4String>("jFinish");
    G4String lTypeStr = mJsonTree.get<G4String>("jType");

    G4OpticalSurfaceModel lModel = getOpticalSurfaceModel(lModelStr);
    G4OpticalSurfaceFinish lFinish = getOpticalSurfaceFinish(lFinishStr);
    G4SurfaceType lType = getSurfaceType(lTypeStr);

    mOpticalSurface = new G4OpticalSurface(mObjectName, lModel, lFinish, lType);
    G4MaterialPropertiesTable *lMPT = new G4MaterialPropertiesTable();

    try // Only few materials have jSigmaAlpha defined
    {
        G4double lSigmaAlpha = mJsonTree.get<G4double>("jSigmaAlpha");
        mOpticalSurface->SetSigmaAlpha(lSigmaAlpha);
    }
    catch (...)
    {
    } // not very elegant, I know...

    // try
    // {
    //     for (pt::ptree::value_type &key : mJsonTree.get_child("jConstProperties"))
    //     {
    //         G4String lKey = key.second.get_value<G4String>();
    //         std::vector<G4double> lPhotonEnergy;
    //         std::vector<G4double> lValues;
    //         ParseToVector(lValues, mJsonTree, "jValues_" + lKey, 1., false);
    //         ParseToVector(lPhotonEnergy, mJsonTree, "jWavelength_" + lKey, mHC_eVnm, true);
    //         sortVectorByReference(lPhotonEnergy, lValues);
    //         lMPT->AddProperty(lKey, &lPhotonEnergy[0], &lValues[0], static_cast<int>(lPhotonEnergy.size()));
    //     }
    // }
    // catch (...) // Only few materials have jConstProperties defined
    // {
    // } // not very elegant, I know...

    for (pt::ptree::value_type &key : mJsonTree.get_child("jProperties"))
    {
        G4String lKey = key.second.get_value<G4String>();
        std::vector<G4double> lPhotonEnergy;
        std::vector<G4double> lValues;
        mFileData->parseKeyContentToVector(lValues, mJsonTree, "jValues_" + lKey, 1., false);
        mFileData->parseKeyContentToVector(lPhotonEnergy, mJsonTree, "jWavelength_" + lKey, mHC_eVnm, true);
        sortVectorByReference(lPhotonEnergy, lValues);
        lMPT->AddProperty(lKey, &lPhotonEnergy[0], &lValues[0], static_cast<int>(lPhotonEnergy.size()));
    }

    mOpticalSurface->SetMaterialPropertiesTable(lMPT);
    G4String mssg = "New Optical Surface: " + mObjectName;
    log_debug(mssg);
}

/**
 * @brief Convert a string to a G4OpticalSurfaceFinish.
 *
 * @param  pFinish Finish in G4String format.
 * @return Finish in G4OpticalSurfaceFinish format.
 */
G4OpticalSurfaceFinish ReflectiveSurface::getOpticalSurfaceFinish(G4String pFinish)
{
    G4OpticalSurfaceFinish lFinish;
    if (pFinish == "polished")
        lFinish = polished;
    else if (pFinish == "polishedfrontpainted")
        lFinish = polishedfrontpainted;
    else if (pFinish == "polishedbackpainted")
        lFinish = polishedbackpainted;
    else if (pFinish == "ground")
        lFinish = ground;
    else if (pFinish == "groundfrontpainted")
        lFinish = groundfrontpainted;
    else if (pFinish == "groundbackpainted")
        lFinish = groundbackpainted;
    else if (pFinish == "polishedlumirrorair")
        lFinish = polishedlumirrorair;
    else if (pFinish == "polishedlumirrorglue")
        lFinish = polishedlumirrorglue;
    else if (pFinish == "polishedair")
        lFinish = polishedair;
    else if (pFinish == "polishedteflonair")
        lFinish = polishedteflonair;
    else if (pFinish == "polishedtioair")
        lFinish = polishedtioair;
    else if (pFinish == "polishedtyvekair")
        lFinish = polishedtyvekair;
    else if (pFinish == "polishedvm2000air")
        lFinish = polishedvm2000air;
    else if (pFinish == "polishedvm2000glue")
        lFinish = polishedvm2000glue;
    else if (pFinish == "etchedlumirrorair")
        lFinish = etchedlumirrorair;
    else if (pFinish == "etchedlumirrorglue")
        lFinish = etchedlumirrorglue;
    else if (pFinish == "etchedair")
        lFinish = etchedair;
    else if (pFinish == "etchedteflonair")
        lFinish = etchedteflonair;
    else if (pFinish == "etchedtioair")
        lFinish = etchedtioair;
    else if (pFinish == "etchedtyvekair")
        lFinish = etchedtyvekair;
    else if (pFinish == "etchedvm2000air")
        lFinish = etchedvm2000air;
    else if (pFinish == "etchedvm2000glue")
        lFinish = etchedvm2000glue;
    else if (pFinish == "groundlumirrorair")
        lFinish = groundlumirrorair;
    else if (pFinish == "groundlumirrorglue")
        lFinish = groundlumirrorglue;
    else if (pFinish == "groundair")
        lFinish = groundair;
    else if (pFinish == "groundteflonair")
        lFinish = groundteflonair;
    else if (pFinish == "groundtioair")
        lFinish = groundtioair;
    else if (pFinish == "groundtyvekair")
        lFinish = groundtyvekair;
    else if (pFinish == "groundvm2000air")
        lFinish = groundvm2000air;
    else if (pFinish == "groundvm2000glue")
        lFinish = groundvm2000glue;
    return lFinish;
}
/**
 * OpticalSurfaceModel in string to G4OpticalSurfaceModel
 * @param  G4String
 * @return G4OpticalSurfaceModel
 */
G4OpticalSurfaceModel ReflectiveSurface::getOpticalSurfaceModel(G4String pModel)
{
    G4OpticalSurfaceModel lModel;
    if (pModel == "glisur")
        lModel = glisur;
    else if (pModel == "unified")
        lModel = unified;
    else if (pModel == "LUT")
        lModel = LUT;
    return lModel;
}
/**
 * SurfaceType in string to G4SurfaceType
 * @param  G4String
 * @return G4SurfaceType
 */
G4SurfaceType ReflectiveSurface::getSurfaceType(G4String pType)
{

    G4SurfaceType lType;
    if (pType == "dielectric_metal")
        lType = dielectric_metal;
    else if (pType == "dielectric_dielectric")
        lType = dielectric_dielectric;
    else if (pType == "dielectric_LUT")
        lType = dielectric_LUT;
    else if (pType == "firsov")
        lType = firsov;
    else if (pType == "x_ray")
        lType = x_ray;
    else if (pType == "coated")
        lType = coated;
    return lType;
}





























