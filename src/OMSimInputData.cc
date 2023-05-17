/** @file OMSimInputData.cc
 *  @brief Input data from external files are read and saved to memory.
 *
 *  All materials for the modules, pmts and environment, as well as the shape parameters of the PMTs and Modules are in the ../data folder.
 *  This class read all files and parse them to tables.
 *  Methods are order as in the header. Please take a look there first!
 *
 *  @author Martin Unland, Cristian Lozano
 *  @date October 2021
 *
 *  @version Geant4 10.7
 *
 */

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "OMSimInputData.hh"
#include "OMSimDataFileTypes.hh"
#include "OMSimLogger.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4SystemOfUnits.hh"
#include <G4UnitsTable.hh>
#include <dirent.h>
#include <cmath>
#include "OMSimCommandArgsTable.hh"

namespace pt = boost::property_tree;

OMSimInputData::OMSimInputData()
{
}

/*
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*                                          Helper Class
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/


/**
 * Appends information of json-file containing PMT/OM parameters to a vector of ptrees
 * @param pFileName Name of file containing json
 */
pt::ptree ParameterTable::AppendAndReturnTree(G4String pFileName)
{
    pt::ptree lJsonTree;
    pt::read_json(pFileName, lJsonTree);
    const G4String lName = lJsonTree.get<G4String>("jName");
    mTable[lName] = lJsonTree;
    G4String mssg = lName + " added to dictionary...";
    debug(mssg);
    return lJsonTree;
}

/**
 * Get values from a pTree with its unit, transforming it to a G4double
 * @param pKey Name of json tree in file
 * @param pParameter Name of parameter in json tree
 * @return G4double of the value and its unit
 */
G4double ParameterTable::GetValueWithUnit(G4String pKey, G4String pParameter)
{
    
    try {
        const G4String lUnit = mTable.at(pKey).get<G4String>(pParameter + ".jUnit");
        const G4double lValue = mTable.at(pKey).get<G4double>(pParameter + ".jValue");
        if (lUnit == "NULL")
        {   
            return lValue;
        }
        else
        {   
            return lValue * G4UnitDefinition::GetValueOf(lUnit);
        }
    }
    catch (...)
    {
        const G4double lValue = mTable.at(pKey).get<G4double>(pParameter);
        return lValue;
    }
}

/**
 * Get values from a pTree with its unit, transforming it to a G4double
 * @param pKey Name of json tree in file
 * @param pParameter Name of parameter in json tree
 * @return G4double of the value and its unit
 */
pt::ptree ParameterTable::GetJSONTree(G4String pKey)
{
    if (CheckIfKeyInTable(pKey)) return mTable.at(pKey);
    else critical("Key not found in table");
}



/**
* Check if key in table
*/
G4bool ParameterTable::CheckIfKeyInTable(G4String pKey)
{
    const G4int lFound = mTable.count(pKey);
    if (lFound > 0) return true;
    else return false;
}
/*
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*                                Main class methods
* %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

/**
* Get a G4Material. In order to get custom built materials, method SearchFolders() should have already been called.
* Standard materials from Geant4 are also transfered directly (if found through FindOrBuildMaterial()).
* Materials defined by arguments of the main should start with "arg" and then we get the material name from the
* arrays depending on the global integers (gGel, gGlass, gEnvironment..) .
* @param pName name of the (custom) material or "argVesselGlass", "argGel", "argWorld" for argument materials
* @return G4Material
*/
G4Material* OMSimInputData::GetMaterial(G4String pName)
{

    //Check if requested material is an argument material
    if (pName.substr(0, 3) == "arg")
    {

        G4String lGlass[] = { "RiAbs_Glass_Vitrovex", "RiAbs_Glass_Chiba", "RiAbs_Glass_Benthos", "RiAbs_Glass_myVitrovex", "RiAbs_Glass_myChiba", "RiAbs_Glass_WOMQuartz" };
        G4String lGel[] = { "RiAbs_Gel_Wacker612Measured", "RiAbs_Gel_Shin-Etsu", "RiAbs_Gel_QGel900", "RiAbs_Gel_Wacker612Company", "Ri_Vacuum" };
        G4String lWorld[] = { "Ri_Air", "IceCubeICE", "IceCubeICE_SPICE" };

        //Get user argument parameters
        OMSimCommandArgsTable& lArgs = OMSimCommandArgsTable::getInstance();
        G4int lGlassIndex = lArgs.get<G4int>("glass");
        G4int lGelIndex = lArgs.get<G4int>("gel");
        G4int lEnvironmentIndex = lArgs.get<G4int>("environment");

        if (pName == "argVesselGlass")
            return GetMaterial(lGlass[lGlassIndex]);

        else if (pName == "argGel")
            return GetMaterial(lGel[lGelIndex]);

        else if (pName == "argWorld")
            return GetMaterial(lWorld[lEnvironmentIndex]);
    }

    //If it is not an argument material, the material is looked up.
    G4Material* lReturn = G4Material::GetMaterial(pName);

    if (!lReturn)
        lReturn = G4NistManager::Instance()->FindOrBuildMaterial(pName);
    return lReturn;
}

/**
* Get a G4OpticalSurface. In order to get custom built materials, method SearchFolders() should have already been called.
* @param pName name of the optical surface or argument reflectors "argReflector"
* @return G4OpticalSurface
*/
G4OpticalSurface* OMSimInputData::GetOpticalSurface(G4String pName)
{

    //Check if requested material is an argument surface
    if (pName.substr(0, 12) == "argReflector")
    {   
        G4String lRefCones[] = { "Refl_V95Gel", "Refl_V98Gel", "Refl_Aluminium", "Refl_Total98" };
        G4int lReflectiveIndex = OMSimCommandArgsTable::getInstance().get<G4int>("reflective_surface");
        return GetOpticalSurface(lRefCones[lReflectiveIndex]);
    }
    //If not, we look in the dictionary
    else
    {
        G4int lFound = mOpticalSurfaceMap.count(pName);
        if (lFound > 0)
        {
            return mOpticalSurfaceMap.at(pName);
        }
        else
        {   G4String mssg = "Requested Optical Surface " + pName + " not found. This will cause a segmentation fault. Please check the name!!";
            critical(mssg);
        }
    }
}

/**
* Calls ScannDataDirectory in the defined folders.
* @see ScannDataDirectory
*/
void OMSimInputData::SearchFolders()
{
    mDataDirectory = "../data/Materials";
    ScannDataDirectory();
    mDataDirectory = "../data/PMTs";
    ScannDataDirectory();
    mDataDirectory = "../data/SegmentedModules";
    ScannDataDirectory();
}

/**
* Scann for data files inside mDataDirectory and creates materials.
* @param pName name of the material
*/
void OMSimInputData::ScannDataDirectory()
{

    struct dirent* lFile = NULL;
    DIR* lDirectory = NULL;

    lDirectory = opendir(mDataDirectory.data());
    if (lDirectory == NULL)
    {
        G4cerr << "Couldn't open directory" << mDataDirectory << G4endl;
        return;
    }

    while ((lFile = readdir(lDirectory)))
    {
        const std::string fileName = lFile->d_name;
        
        if (lFile->d_type == 8)
        {
            if (fileName.substr(0, 5) == "RiAbs")
            {
                RefractionAndAbsorption* lDataFile = new RefractionAndAbsorption(mDataDirectory + "/" + fileName);
                lDataFile->ExtractInformation();
            }
            else if (fileName.substr(0, 2) == "Ri")
            {
                RefractionOnly* lDataFile = new RefractionOnly(mDataDirectory + "/" + fileName);
                lDataFile->ExtractInformation();
            }
            else if (fileName.substr(0, 7) == "NoOptic")
            {
                NoOptics* lDataFile = new NoOptics(mDataDirectory + "/" + fileName);
                lDataFile->ExtractInformation();
            }
            else if (fileName.substr(0, 10) == "IceCubeICE")
            {
                IceCubeIce* lDataFile = new IceCubeIce(mDataDirectory + "/" + fileName);
                lDataFile->ExtractInformation();
            }
            else if ((fileName.substr(0, 4) == "Refl"))
            {
                ReflectiveSurface* lDataFile = new ReflectiveSurface(mDataDirectory + "/" + fileName);
                lDataFile->ExtractInformation();
                mOpticalSurfaceMap[lDataFile->mObjectName] = lDataFile->mOpticalSurface;
            }
            else if ((fileName.substr(0, 4) == "pmt_"))
                AppendAndReturnTree(mDataDirectory + "/" + fileName);
            else if ((fileName.substr(0, 3) == "om_"))
                AppendAndReturnTree(mDataDirectory + "/" + fileName);
            else if ((fileName.substr(0, 4) == "usr_"))
                AppendAndReturnTree(mDataDirectory + "/" + fileName);
        }
    }
    closedir(lDirectory);
}
