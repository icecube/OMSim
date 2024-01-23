#include "OMSimInputData.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimDataFileTypes.hh"

#include <G4UnitsTable.hh>
#include <dirent.h>

namespace pt = boost::property_tree;

InputDataManager::InputDataManager()
{
    log_debug("Starting InputDataManager");
}

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
    try
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
    catch (...)
    {
        // Fallback to getting just the value if any of the above fails
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
}

/**
 * @brief Checks if a key exists within the table.
 *
 * @param pKey The key to check.
 * @return true if the key exists, false otherwise.
 */
G4bool ParameterTable::checkIfKeyInTable(G4String pKey)
{
    const G4int lFound = mTable.count(pKey);
    if (lFound > 0)
        return true;
    else
        return false;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Main class methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */
/**
 * @brief Reads numerical data from a file and returns it as a 2D vector.
 * Similar to numpy.loadtxt.
 *
 * @param pFilePath The path to the input file.
 * @param pUnpack Optional. If true, the returned data is transposed, i.e.,
 * unpacked into columns. Default is true.
 * @param pSkipRows Optional. The number of lines to skip at the beginning of
 * the file. Default is 0.
 * @param pDelimiter The character used to separate values in each line of the
 * input file.
 * @return A 2D vector of doubles. The outer vector groups all columns (or
 * rows if 'unpack' is false), and each inner vector represents one of the
 * columns (or one of the rows if 'unpack' is false) of data file.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::vector<std::vector<double>>
InputDataManager::loadtxt(const std::string &pFilePath, bool pUnpack,
                          size_t pSkipRows, char pDelimiter)
{
    std::vector<std::vector<double>> lData;
    std::ifstream lInFile(pFilePath);

    if (!lInFile.is_open())
    {
        log_error("Could not open file {}",pFilePath);
        throw std::runtime_error("Could not open file " + pFilePath);
    }

    std::string lLine;
    size_t lRowCounter = 0;

    while (getline(lInFile, lLine))
    {
        if (lRowCounter++ < pSkipRows)
            continue;
        std::vector<double> lRow;
        std::stringstream lSs(lLine);
        std::string lItem;
        while (getline(lSs, lItem, pDelimiter))
        {
            lRow.push_back(stod(lItem));
        }
        lData.push_back(lRow);
    }

    if (pUnpack)
    {
        size_t lNumCols = lData[0].size();
        std::vector<std::vector<double>> lTransposedData(
            lNumCols, std::vector<double>(lData.size()));

        for (size_t i = 0; i < lData.size(); ++i)
        {
            for (size_t j = 0; j < lNumCols; ++j)
            {
                lTransposedData[j][i] = lData[i][j];
            }
        }
        return lTransposedData;
    }
    else
    {
        return lData;
    }
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
G4Material *InputDataManager::getMaterial(G4String pName)
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
G4OpticalSurface *InputDataManager::getOpticalSurface(G4String pName)
{

    // Check if requested material is an argument surface
    if (pName.substr(0, 12) == "argReflector")
    {
        G4String lRefCones[] = {"Refl_V95Gel", "Refl_V98Gel", "Refl_Aluminium",
                                "Refl_Total98"};
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
            G4String mssg = "Requested Optical Surface " + pName +
                            " not found. This will cause a segmentation fault. "
                            "Please check the name!!";
            log_critical(mssg);
        }
    }
}

/**
 * @brief Searches through predefined folders for input data files.
 */
void InputDataManager::searchFolders()
{
    log_debug("Searching folders for data json files...");
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
 * RefractionOnly, NoOptics, IceCubeIce, ReflectiveSurface or others) based on
 * the prefix of the filename. It then constructs an object of the appropriate
 * class and invokes its information extraction method. For 'Refl' prefixed
 * files, it also updates the mOpticalSurfaceMap. For 'pmt_', 'om_', and
 * 'usr_' prefixed files, it invokes directly appendAndReturnTree without any
 * extra parsing into Geant4 objects.
 *
 * @param pFilePath Full path to the file.
 * @param pFileName Name of the file (without the path).
 */
void InputDataManager::processFile(const std::string &pFilePath,
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
    else if ((pFileName.substr(0, 4) == "Refl"))
    {
        ReflectiveSurface lDataFile(pFilePath);
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
void InputDataManager::scannDataDirectory()
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
