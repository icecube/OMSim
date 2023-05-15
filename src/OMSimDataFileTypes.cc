#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "OMSimDataFileTypes.hh"
#include "OMSimInputData.hh"
#include "OMSimLogger.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4SystemOfUnits.hh"
#include <G4UnitsTable.hh>
#include <dirent.h>
#include <cmath>
#include <numeric>

namespace pt = boost::property_tree;

extern std::vector<double> readColumnDouble (G4String fn, int col);
/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                  Base Abstract Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */
/**
 * Constructor. Saves file name to member.
 * @param pFileName
 */
abcDataFile::abcDataFile(G4String pFileName)
{   mFileData = new ParameterTable();
    mFileName = pFileName;
}

template <typename T>
/**
 * Transforms the values inside a pTree-array to a vector. The values can be also transformed to a G4double.
 * @param pVector  vector where the (transformed) values are saved
 * @param pTree    pTree containing json data
 * @param pKey     json attribute label where values are found
 * @param pScaling Values of array are multiplied by this factor. You can set the fisical unit with this.
 * @param pInverse In case you need the inverse of a value x, 1/x is appended (e.g. transforming from nm to eV)
 */
void abcDataFile::ParseToVector(std::vector<T> &pVector, pt::ptree pTree, std::basic_string<char> pKey, G4double pScaling, bool pInverse)
{
    for (pt::ptree::value_type &ridx : pTree.get_child(pKey))
    { //get array from element with key "pKey" of the json
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


/**
 * @brief Sorts two vectors (sortVector & referenceVector) based on the order of values in referenceVector.
 */
void abcDataFile::sortVectorByReference(std::vector<G4double>& referenceVector, std::vector<G4double>& sortVector) 
{
    // Check if the vectors have the same size
    if(referenceVector.size() != sortVector.size())
    {
        // Handle error
        throw std::invalid_argument("The two vectors must have the same size.");
    }

    // Create a vector of indices
    std::vector<std::size_t> indices(referenceVector.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Sort the indices based on the values in referenceVector
    std::sort(indices.begin(), indices.end(), 
       [&referenceVector](std::size_t i1, std::size_t i2) {return referenceVector[i1] < referenceVector[i2];});

    // Create temporary vectors to hold the sorted data
    std::vector<G4double> sortedSortVector(sortVector.size());
    std::vector<G4double> sortedReferenceVector(referenceVector.size());

    // Apply the sorted indices to the vectors
    for(std::size_t i = 0; i < indices.size(); ++i) {
        sortedSortVector[i] = sortVector[indices[i]];
        sortedReferenceVector[i] = referenceVector[indices[i]];
    }

    // Replace the original vectors with the sorted ones
    sortVector = std::move(sortedSortVector);
    referenceVector = std::move(sortedReferenceVector);
}



/**
 * Defines new material from data in json-file. 
 */
void abcMaterialData::CreateMaterial()
{

    pt::read_json(mFileName, mJsonTree); //read json file into mJsonTree

    mFileData->AppendParameterTable(mFileName);
    mMPT = new G4MaterialPropertiesTable();
    mMatDatBase = G4NistManager::Instance();

    mObjectName = mJsonTree.get<G4String>("jName");
    const G4String lDataType = mJsonTree.get<G4String>("jDataType");

    const G4double lDensity = mFileData->GetValue(mObjectName, "jDensity");

    const G4String lState_str = mJsonTree.get<G4String>("jState");
    const G4State lState = GetState(lState_str);

    //Defining the material with its density, number of components, state and name
    mMaterial = new G4Material(mObjectName, lDensity, mJsonTree.get_child("jComponents").size(), lState);

    //Construct material with fractional components (isotopes or G4-Materials)
    for (pt::ptree::value_type &key : mJsonTree.get_child("jComponents"))
    {
        std::string componentName = key.first;
        double componentFraction = key.second.get_value<double>();
        mMaterial->AddMaterial(mMatDatBase->FindOrBuildMaterial(componentName), componentFraction);
    }
    G4String mssg = "New Material defined: " + mMaterial->GetName();
    info(mssg);
}
/**
 * Extracts absorption length and adds it to the material property table 
 */
void abcMaterialData::ExtractAbsorptionLength()
{
    std::vector<G4double> lAbsLength;
    std::vector<G4double> lAbsLengthEnergy;
    ParseToVector(lAbsLength, mJsonTree, "jAbsLength", 1 * mm, false);
    ParseToVector(lAbsLengthEnergy, mJsonTree, "jAbsLengthWavelength", mHC_eVnm, true);
    sortVectorByReference(lAbsLengthEnergy, lAbsLength);
    mMPT->AddProperty("ABSLENGTH", &lAbsLengthEnergy[0], &lAbsLength[0], static_cast<int>(lAbsLength.size()));
}
/**
 * Extracts refraction index and adds it to the material property table 
 */
void abcMaterialData::ExtractRefractionIndex()
{
    std::vector<G4double> lRefractionIndex;
    std::vector<G4double> lRefractionIndexEnergy;
    ParseToVector(lRefractionIndex, mJsonTree, "jRefractiveIdx", 1., false);
    ParseToVector(lRefractionIndexEnergy, mJsonTree, "jRefractiveIdxWavelength", mHC_eVnm, true);
    sortVectorByReference(lRefractionIndexEnergy, lRefractionIndex);
    mMPT->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
}
/**
 * State in string to G4State
 * @param  G4String
 * @return G4State
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
 * Extracts and creates material for material with refraction index and absorption length defined.
 */
void RefractionAndAbsorption::ExtractInformation()
{
    CreateMaterial();
    ExtractAbsorptionLength();
    ExtractRefractionIndex();
    
    mMaterial->SetMaterialPropertiesTable(mMPT);
}
/**
 * Extracts and creates material for material with refraction index defined.
 */
void RefractionOnly::ExtractInformation()
{
    CreateMaterial();
    ExtractRefractionIndex();
    mMaterial->SetMaterialPropertiesTable(mMPT);
}
/**
 * Extracts and creates material without optical properties.
 */
void NoOptics::ExtractInformation()
{
    CreateMaterial();
}
/**
 * Extracts and creates ice with optical properties from IceCube.
 */
void IceCubeIce::ExtractInformation()
{
    CreateMaterial(); //creates IceCubeICE

    G4Material *lIceMie = new G4Material("IceCubeICE_SPICE", mFileData->GetValue(mObjectName,"jDensity"), mMatDatBase->FindOrBuildMaterial("G4_WATER"), kStateSolid); //create IceCubeICE_SPICE
    G4Material *lBubleColumnMie = new G4Material("Mat_BubColumn", mFileData->GetValue(mObjectName,"jDensity"), mMatDatBase->FindOrBuildMaterial("G4_WATER"), kStateSolid); //create IceCubeICE_SPICE
    std::vector<G4double> lMieScatteringLength;
    std::vector<G4double> lMieScatteringLength_BubleColumn;
    std::vector<G4double> lWavelength;
    std::vector<G4double> lRefractionIndex;
    std::vector<G4double> lRefractionIndexEnergy;
    std::vector<G4double> lAbsLength;
    ParseToVector(lRefractionIndexEnergy, mJsonTree, "jWavelength_spice", mHC_eVnm, true);
    ParseToVector(lWavelength, mJsonTree, "jWavelength_spice", 1 * nm, false);
    ParseToVector(mSpice_be400inv, mJsonTree, "jbe400inv_spice", 1 * m, false);
    ParseToVector(mSpice_a400inv, mJsonTree, "ja400inv_spice", 1 * m, false);
    ParseToVector(mSpice_Depth, mJsonTree, "jDepth_spice", 1 * m, false);

    for (int u = 0; u < static_cast<int>(lRefractionIndexEnergy.size()); u++)
    {
        lRefractionIndex.push_back(Spice_Refraction(lWavelength.at(u)));
        lAbsLength.push_back(Spice_Absorption(lWavelength.at(u)));
        lMieScatteringLength.push_back(Mie_Scattering(lWavelength.at(u)));
        lMieScatteringLength_BubleColumn.push_back(mInnercolumn_b_inv);
    }
    //give refractive index to IceCubeICE. This is used also for IceCubeICE_SPICE
    mMPT->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
    mMaterial->SetMaterialPropertiesTable(mMPT);
    
    //give properties to IceCubeICE_SPICE
    G4MaterialPropertiesTable* lMPT_spice = new G4MaterialPropertiesTable();
    lMPT_spice->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
    lMPT_spice->AddProperty("ABSLENGTH", &lRefractionIndexEnergy[0], &lAbsLength[0], static_cast<int>(lAbsLength.size()));
    lMPT_spice->AddProperty("MIEHG", &lRefractionIndexEnergy[0], &lMieScatteringLength[0], static_cast<int>(lRefractionIndex.size()));//->SetSpline(true);
    lMPT_spice->AddConstProperty("MIEHG_FORWARD", mMIE_spice_const[0]);
    lMPT_spice->AddConstProperty("MIEHG_BACKWARD", mMIE_spice_const[1]);
    lMPT_spice->AddConstProperty("MIEHG_FORWARD_RATIO", mMIE_spice_const[2]);
    lIceMie->SetMaterialPropertiesTable(lMPT_spice);
    G4String mssg = "Ice properties at depth " + std::to_string(mSpice_Depth[mSpiceDepth_pos] / m) + " m.";
    notice(mssg);
    //now give the properties to the bubble column, which are basically the same ones but with the chosen scattering lenght
    G4MaterialPropertiesTable* lMPT_holeice = new G4MaterialPropertiesTable();
    lMPT_holeice->AddProperty("RINDEX", &lRefractionIndexEnergy[0], &lRefractionIndex[0], static_cast<int>(lRefractionIndex.size()));
    lMPT_holeice->AddProperty("ABSLENGTH", &lRefractionIndexEnergy[0], &lAbsLength[0], static_cast<int>(lAbsLength.size()));
    lMPT_holeice->AddProperty("MIEHG", &lRefractionIndexEnergy[0], &lMieScatteringLength_BubleColumn[0], static_cast<int>(lRefractionIndex.size()));//->SetSpline(true);
    lMPT_holeice->AddConstProperty("MIEHG_FORWARD", mMIE_spice_const[0]);
    lMPT_holeice->AddConstProperty("MIEHG_BACKWARD", mMIE_spice_const[1]);
    lMPT_holeice->AddConstProperty("MIEHG_FORWARD_RATIO", mMIE_spice_const[2]);
    lBubleColumnMie->SetMaterialPropertiesTable(lMPT_holeice);
}
/*
 * %%%%%%%%%%%%%%%% Functions for icecube ice optical properties %%%%%%%%%%%%%%%%
 */
/**
 * This gives you temperature of ice depending on the depth.
 * Function needed for the calculation of scattering and absorption length of the ice. 
 * @param pDepth Depth in m from where we need the temperature
 * @return Temperature
 */
G4double IceCubeIce::Spice_Temperature(G4double pDepth)
{
    G4double spice_temp = 221.5 - 0.00045319 / m * pDepth + 5.822e-6 / m2 * pow(pDepth, 2.);
    return spice_temp;
}

/**
 * Calculation of the absorption length of IC-ice for a specific wavelength
 * @param pLambd Wavelength
 * @return Absorption length
 */
G4double IceCubeIce::Spice_Absorption(G4double pLambd)
{
    G4double lKappa = 1.08;
    G4double lParamA = 6954. / m;
    G4double lParamB = 6618 * nm;
    G4double lAdust = 1. / (mSpice_a400inv[mSpiceDepth_pos]) * pow(pLambd / (400. * nm), -lKappa);
    G4double lDeltaTau = Spice_Temperature(mSpice_Depth[mSpiceDepth_pos]) - Spice_Temperature(1730.);
    G4double la_inv = 1. / (lAdust + lParamA * exp(-lParamB / pLambd) * (1. + 0.01 * lDeltaTau));
    return la_inv;
}

/**
 * Calculation of the refraction index of IC-ice for a specific wavelength.
 * @param pLambd Wavelength
 * @return Refraction index
 */
G4double IceCubeIce::Spice_Refraction(G4double pLambd)
{
    // unknown depth. Parametrization by Thomas Kittler.
    G4double lLambd3 = pLambd * 1e-3;
    G4double lNphase = 1.55749 - 1.57988 / nm * lLambd3 + 3.99993 / (nm * nm) * pow(lLambd3, 2) - 4.68271 / (nm * nm * nm) * pow(lLambd3, 3) + 2.09354 / (nm * nm * nm * nm) * pow(lLambd3, 4);
    return lNphase; // using this now after discussion with Timo
}
/**
 * Calculation of the mie scattering length of IC-ice for a specific wavelength
 * @param pLambd Wavelength
 * @return Mie scattering length
 */
G4double IceCubeIce::Mie_Scattering(G4double pLambd)
{
    // depth_pos is the coordinate for the chosen depth in Depth_spice. For example to choose
    // depth=2278.2 m, we use depth_pos = 88
    G4double lAlpha = 0.90;
    G4double lAv_costheta = 0.9;
    G4double lBe_inv = 1. / (1. / (mSpice_be400inv[mSpiceDepth_pos]) * pow((pLambd / (400. * nm)), -lAlpha));
    G4double lB_inv = lBe_inv * (1. - lAv_costheta);
    return lB_inv;
}
/*
 * %%%%%%%%%%%%%%%% Functions of derived class ReflectiveSurface %%%%%%%%%%%%%%%%
 */
/**
 * Defines new reflective surface from data in json-file. 
 */
void ReflectiveSurface::ExtractInformation()
{

    pt::read_json(mFileName, mJsonTree); //read json file into mJsonTree

    mObjectName = mJsonTree.get<G4String>("jName");
    G4String lModelStr = mJsonTree.get<G4String>("jModel");
    G4String lFinishStr = mJsonTree.get<G4String>("jFinish");
    G4String lTypeStr = mJsonTree.get<G4String>("jType");

    G4OpticalSurfaceModel lModel = GetOpticalSurfaceModel(lModelStr);
    G4OpticalSurfaceFinish lFinish = GetOpticalSurfaceFinish(lFinishStr);
    G4SurfaceType lType = GetSurfaceType(lTypeStr);

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

    for (pt::ptree::value_type &key : mJsonTree.get_child("jProperties"))
    {
        G4String lKey = key.second.get_value<G4String>();
        std::vector<G4double> lPhotonEnergy;
        std::vector<G4double> lValues;
        ParseToVector(lValues, mJsonTree, "jValues_" + lKey, 1., false);
        ParseToVector(lPhotonEnergy, mJsonTree, "jWavelength_" + lKey, mHC_eVnm, true);
        sortVectorByReference(lPhotonEnergy, lValues);
        lMPT->AddProperty(lKey, &lPhotonEnergy[0], &lValues[0], static_cast<int>(lPhotonEnergy.size()));
    }

    mOpticalSurface->SetMaterialPropertiesTable(lMPT);
    G4String mssg = "New Optical Surface: " + mObjectName;
    info(mssg);
}

/**
 * OpticalSurfaceFinish in string to G4OpticalSurfaceFinish
 * @param  G4String
 * @return G4OpticalSurfaceFinish
 */
G4OpticalSurfaceFinish ReflectiveSurface::GetOpticalSurfaceFinish(G4String pFinish)
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
G4OpticalSurfaceModel ReflectiveSurface::GetOpticalSurfaceModel(G4String pModel)
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
G4SurfaceType ReflectiveSurface::GetSurfaceType(G4String pType)
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
    return lType;
}

