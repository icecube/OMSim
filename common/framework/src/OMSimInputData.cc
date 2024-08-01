#include "OMSimInputData.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimDataFileTypes.hh"

#include <G4UnitsTable.hh>
#include <dirent.h>

namespace pt = boost::property_tree;

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                          Helper Class
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 * Appends information of json-file containing PMT/OM parameters to a dictionary
 * of ptrees
 * @param pFileName Name of file containing json
 */
pt::ptree ParameterTable::appendAndReturnTree(G4String pFileName)
{
    pt::ptree lJsonTree;
    pt::read_json(pFileName, lJsonTree);
    const G4String lName = lJsonTree.get<G4String>("jName");
    mTable[lName] = lJsonTree;
    mKeyFileOrigin[lName] = pFileName;
    log_trace("Key {} added to dictionary from file {}.", lName, pFileName);
    return lJsonTree;
}

/**
 * @brief Fetches the value associated with a given key and parameter.
 *
 * @param pKey The main key.
 * @param pParameter The parameter within the main key's tree.
 * @return The value associated with the key and parameter, with units converted to Geant4-compatible units.
 */
G4double ParameterTable::getValueWithUnit(G4String pKey, G4String pParameter)
{
    if (!checkIfKeyInTable(pKey))
    {
        log_error("Key {} not found in parameter table!", pKey);
        throw std::runtime_error("Key not found in parameter table!");
    }

    // Get the sub-tree for the provided key
    const auto &lSubTree = mTable.at(pKey);

    if (!lSubTree.get_child_optional(pParameter))
    {
        G4String lErrorLog = "Table in key '" + pKey + "' has no parameter '" + pParameter + "'. Check file '" + mKeyFileOrigin.at(pKey) + "'";
        log_error(lErrorLog);
        throw std::runtime_error(lErrorLog);
    }

    // Try getting the value with unit first
    if (lSubTree.get_optional<G4String>(pParameter + ".jUnit") && lSubTree.get_optional<G4String>(pParameter + ".jValue"))
    {
        const G4String lUnit = lSubTree.get<G4String>(pParameter + ".jUnit");
        const G4double lValue = lSubTree.get<G4double>(pParameter + ".jValue");

        if (lUnit == "NULL")
        {
            return lValue;
        }

        // Check if the unit should be inverted
        bool invertUnit = lSubTree.get_child_optional(pParameter + ".jInvertUnit").has_value();

        // Return the value with or without inverted unit, ternary operators in c++: "variable = (condition) ? expressionTrue : expressionFalse;"
        return invertUnit
                   ? lValue / G4UnitDefinition::GetValueOf(lUnit)
                   : lValue * G4UnitDefinition::GetValueOf(lUnit);
    }
    else
    {
        return lSubTree.get<G4double>(pParameter);
    }
}

/**
 * Get tree object saved in table
 * @param pKey Key of tree ("jName" in corresponding json file)
 * @return pt::ptree
 */
pt::ptree ParameterTable::getJSONTree(G4String pKey)
{
    if (checkIfKeyInTable(pKey))
        return mTable.at(pKey);
    else
        log_critical("Key not found in table");
        return boost::property_tree::ptree(); // Return an empty ptree
}

/**
 * @brief Checks if a key exists within the table.
 *
 * @param pKey The key to check.
 * @return true if the key exists, false otherwise.
 */
G4bool ParameterTable::checkIfKeyInTable(G4String pKey)
{
    log_trace("Checking if key {} is in table...", pKey);
    const G4int lFound = mTable.count(pKey);
    if (lFound > 0)
        return true;
    else
        log_trace("Key {} was not found...", pKey);
        return false;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Main class methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */


void OMSimInputData::init()
{
    if (!gOMSimInputData){
        gOMSimInputData = new OMSimInputData();
        gOMSimInputData->searchFolders();
    }
        
}

void OMSimInputData::shutdown()
{
    delete gOMSimInputData;
    gOMSimInputData = nullptr;
}

/**
 * @return A reference to the OMSimInputData instance.
 * @throw std::runtime_error if accessed before initialization or after shutdown.
 */
OMSimInputData& OMSimInputData::getInstance()
{
    if (!gOMSimInputData)
        throw std::runtime_error("OMSimInputData accessed before initialization or after shutdown!");
    return *gOMSimInputData;
}


/**
 * Get a G4Material. In order to get custom built materials, method
 * searchFolders() should have already been called. Standard materials from
 * Geant4 are also transfered directly (if found through
 * FindOrBuildMaterial()). Materials defined by arguments of the main should
 * start with "arg" and then we get the material name from the arrays
 * depending on the global integers (gGel, gGlass, gEnvironment..) .
 * @param pName name of the (custom) material or "argVesselGlass", "argGel",
 * "argWorld" for argument materials
 * @return G4Material
 */
G4Material *OMSimInputData::getMaterial(G4String pName)
{

    // Check if requested material is an argument material
    if (pName.substr(0, 3) == "arg")
    {

        G4String lGlass[] = {"RiAbs_Glass_Vitrovex", "RiAbs_Glass_Chiba",
                             "RiAbs_Glass_Benthos", "RiAbs_Glass_myVitrovex",
                             "RiAbs_Glass_myChiba", "RiAbs_Glass_WOMQuartz"};
        G4String lGel[] = {"RiAbs_Gel_Wacker612Measured", "RiAbs_Gel_Shin-Etsu",
                           "RiAbs_Gel_QGel900", "RiAbs_Gel_Wacker612Company",
                           "Ri_Vacuum"};
        G4String lWorld[] = {"Ri_Air", "IceCubeICE", "IceCubeICE_SPICE"};

        // Get user argument parameters
        OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
        G4int lGlassIndex = lArgs.get<G4int>("glass");
        G4int lGelIndex = lArgs.get<G4int>("gel");
        G4int lEnvironmentIndex = lArgs.get<G4int>("environment");

        if (pName == "argVesselGlass")
            return getMaterial(lGlass[lGlassIndex]);

        else if (pName == "argGel")
            return getMaterial(lGel[lGelIndex]);

        else if (pName == "argWorld")
            return getMaterial(lWorld[lEnvironmentIndex]);
    }

    // If it is not an argument material, the material is looked up.
    G4Material *lReturn = G4Material::GetMaterial(pName);

    if (!lReturn)
        lReturn = G4NistManager::Instance()->FindOrBuildMaterial(pName);
    return lReturn;
}

/**
 * Get a G4OpticalSurface. In order to get custom built materials, method
 * searchFolders() should have already been called.
 * @param pName name of the optical surface or argument reflectors
 * "argReflector"
 * @return G4OpticalSurface
 */
G4OpticalSurface *OMSimInputData::getOpticalSurface(G4String pName)
{

    // Check if requested material is an argument surface
    if (pName.substr(0, 12) == "argReflector")
    {
        G4String lRefCones[] = {"Surf_V95Gel", "Surf_V98Gel", "Surf_Aluminium",
                                "Surf_Total98"};
        G4int lReflectiveIndex =
            OMSimCommandArgsTable::getInstance().get<G4int>("reflective_surface");
        return getOpticalSurface(lRefCones[lReflectiveIndex]);
    }
    // If not, we look in the dictionary
    else
    {
        G4int lFound = mOpticalSurfaceMap.count(pName);
        if (lFound > 0)
        {
            return mOpticalSurfaceMap.at(pName);
        }
        else
        {
            log_critical("Requested Optical Surface {} not found. Please check the name for typos!!", pName);
            throw std::runtime_error("Requested Optical Surface not found: " + pName);
        }
    }
}

/**
 * @brief Searches through predefined folders for input data files.
 */
void OMSimInputData::searchFolders()
{
    log_trace("Searching folders for data json files...");
    std::vector<std::string> directories = {
        "../common/data/Materials", "../common/data/PMTs", "../common/data/scintillation",
        // ... you can add more directories here ...
    };

    for (const auto &directory : directories)
    {
        mDataDirectory = directory;
        scannDataDirectory();
    }
}

/**
 * @brief Processes a data file based on its name prefix.
 *
 * The function identifies the type of data file (RefractionAndAbsorption,
 * RefractionOnly, NoOptics, IceCubeIce, Surface or others) based on
 * the prefix of the filename. It then constructs an object of the appropriate
 * class and invokes its information extraction method. For 'Surf' prefixed
 * files, it also updates the mOpticalSurfaceMap. For 'pmt_', 'om_', and
 * 'usr_' prefixed files, it invokes directly appendAndReturnTree without any
 * extra parsing into Geant4 objects.
 *
 * @param pFilePath Full path to the file.
 * @param pFileName Name of the file (without the path).
 */
void OMSimInputData::processFile(const std::string &pFilePath,
                                   const std::string &pFileName)
{
    log_trace("Processing file {}", pFileName);

    if (pFileName.substr(0, 5) == "RiAbs")
    {
        RefractionAndAbsorption lDataFile(pFilePath);
        lDataFile.extractInformation();
    }
    else if (pFileName.substr(0, 2) == "Ri")
    {
        RefractionOnly lDataFile(pFilePath);
        lDataFile.extractInformation();
    }
    else if (pFileName.substr(0, 7) == "NoOptic")
    {
        NoOptics lDataFile(pFilePath);
        lDataFile.extractInformation();
    }
    else if (pFileName.substr(0, 10) == "IceCubeICE")
    {
        IceCubeIce lDataFile(pFilePath);
        lDataFile.extractInformation();
    }
    else if ((pFileName.substr(0, 4) == "Surf"))
    {
        Surface lDataFile(pFilePath);
        lDataFile.extractInformation();
        mOpticalSurfaceMap[lDataFile.mObjectName] = lDataFile.mOpticalSurface;
    }
    else if ((pFileName.substr(0, 5) == "Scint"))
    {
        ScintillationProperties lDataFile(pFilePath);
        lDataFile.extractInformation();
    }
    else if ((pFileName.substr(0, 6) == "Custom"))
    {
        CustomProperties lDataFile(pFilePath);
        lDataFile.extractInformation();
    }
    else if (pFileName.substr(0, 4) == "pmt_" ||
             pFileName.substr(0, 4) == "usr_")
    {
        appendAndReturnTree(pFilePath);
    }
}
/**
 * Scann for data files inside mDataDirectory and process files.
 * @param pName name of the material
 */
void OMSimInputData::scannDataDirectory()
{
    log_trace("Loading files in {}", mDataDirectory);
    DIR *lDirectory = opendir(mDataDirectory.data());
    if (lDirectory == NULL)
    {
        G4cerr << "Couldn't open directory" << mDataDirectory << G4endl;
        return;
    }

    dirent *lFile;
    while ((lFile = readdir(lDirectory)))
    {
        if (lFile->d_type != DT_REG) // ignore if not a regular file
        {
            continue;
        }

        const std::string pFileName = lFile->d_name;
        std::string filePath = mDataDirectory + "/" + pFileName;

        processFile(filePath, pFileName);
    }
    closedir(lDirectory);
}
