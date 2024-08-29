#include "OMSimInputData.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimMaterialHandler.hh"

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
 * @param p_fileName Name of file containing json
 */
pt::ptree ParameterTable::appendAndReturnTree(G4String p_fileName)
{
    pt::ptree lJsonTree;
    pt::read_json(p_fileName, lJsonTree);
    const G4String lName = lJsonTree.get<G4String>("jName");
    m_table[lName] = lJsonTree;
    m_keyToFileName[lName] = p_fileName;
    log_trace("Key {} added to dictionary from file {}.", lName, p_fileName);
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
    if (!checkIfTreeNameInTable(pKey))
    {
        log_error("Key {} not found in parameter table!", pKey);
        throw std::runtime_error("Key not found in parameter table!");
    }

    // Get the sub-tree for the provided key
    const auto &lSubTree = m_table.at(pKey);

    if (!lSubTree.get_child_optional(pParameter))
    {
        G4String lErrorLog = "Table in key '" + pKey + "' has no parameter '" + pParameter + "'. Check file '" + m_keyToFileName.at(pKey) + "'";
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
    if (checkIfTreeNameInTable(pKey))
        return m_table.at(pKey);
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
G4bool ParameterTable::checkIfTreeNameInTable(G4String pKey)
{
    log_trace("Checking if tree {} is in table...", pKey);
    const G4int lFound = m_table.count(pKey);
    if (lFound > 0)
        return true;
    else
        log_trace("Key {} was not found...", pKey);
    return false;
}

/**
 * @brief Checks if a specific key exists in a given JSON tree.
 * @param p_treeName Name of the JSON tree to search in.
 * @param p_key Key to look for in the tree.
 * @return true if the key exists in the tree, false otherwise.
 */
G4bool ParameterTable::checkIfKeyInTree(G4String p_treeName, G4String p_key)
{
    log_trace("Checking if key {} is in tree {}...", p_key, p_treeName);
    pt::ptree tree = getJSONTree(p_treeName);
    try
    {
        tree.get<std::string>(p_key);
        return true;
    }
    catch (boost::property_tree::ptree_bad_path &)
    {
        return false;
    }
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Main class methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 * @brief Initializes the global instance of OMSimInputData and calls OMSimInputData::searchFolders to load data
 *
 * This method is normally called during construction of the base class OMSimDetectorConstruction.
 */
void OMSimInputData::init()
{
    if (!g_OMSimInputData)
    {
        g_OMSimInputData = new OMSimInputData();
        g_OMSimInputData->searchFolders();
    }
}

/**
 * @brief Deletes the global instance of OMSimInputData.
 *
 * This method is normally called in the destructor ~OMSimDetectorConstruction.
 */
void OMSimInputData::shutdown()
{
    delete g_OMSimInputData;
    g_OMSimInputData = nullptr;
}

/**
 * @return A reference to the OMSimInputData instance.
 * @throw std::runtime_error if accessed before initialization or after shutdown.
 */
OMSimInputData &OMSimInputData::getInstance()
{
    if (!g_OMSimInputData)
        throw std::runtime_error("OMSimInputData accessed before initialization or after shutdown!");
    return *g_OMSimInputData;
}

/**
 * @brief Retrieves a G4Material based on the given name.
 *
 * Handles both predefined and argument-based materials. For argument materials 
 * (prefixed with "arg"), it selects materials based on user-defined indices 
 * for glass, gel, and environment. For standard materials, it retrieves from 
 * Geant4's material database or NIST manager.
 *
 * @param pName Material name or argument material key ("argVesselGlass", "argGel", "argWorld").
 * @return Pointer to the requested G4Material, or nullptr if not found.
 *
 * @note Uses OMSimCommandArgsTable for argument-based material selection.
 * @warning May cause undefined behavior if argument indices are out of bounds.
 */
G4Material *OMSimInputData::getMaterial(G4String p_name)
{

    // Check if requested material is an argument material
    if (p_name.substr(0, 3) == "arg")
    {

        G4String glass[] = {"RiAbs_Glass_Vitrovex", "RiAbs_Glass_Chiba",
                             "RiAbs_Glass_Benthos", "RiAbs_Glass_myVitrovex",
                             "RiAbs_Glass_myChiba", "RiAbs_Glass_WOMQuartz"};
        G4String gel[] = {"RiAbs_Gel_Wacker612Measured", "RiAbs_Gel_Shin-Etsu",
                           "RiAbs_Gel_QGel900", "RiAbs_Gel_Wacker612Company",
                           "Ri_Vacuum"};
        G4String world[] = {"Ri_Air", "IceCubeICE", "IceCubeICE_SPICE"};

        // Get user argument parameters
        OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
        G4int glassIndex = args.get<G4int>("glass");
        G4int gelIndex = args.get<G4int>("gel");
        G4int environmentIndex = args.get<G4int>("environment");

        if (p_name == "argVesselGlass")
            return getMaterial(glass[glassIndex]);

        else if (p_name == "argGel")
            return getMaterial(gel[gelIndex]);

        else if (p_name == "argWorld")
            return getMaterial(world[environmentIndex]);
    }

    // If it is not an argument material, the material is looked up.
    G4Material *toReturn = G4Material::GetMaterial(p_name);

    if (!toReturn)
        toReturn = G4NistManager::Instance()->FindOrBuildMaterial(p_name);
    return toReturn;
}

/**
 * Get a G4OpticalSurface. In order to get custom built materials, method
 * searchFolders() should have already been called.
 * @param p_name name of the optical surface or argument reflectors
 * "argReflector"
 * @return G4OpticalSurface
 */
G4OpticalSurface *OMSimInputData::getOpticalSurface(G4String p_name)
{

    // Check if requested material is an argument surface
    if (p_name.substr(0, 12) == "argReflector")
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
        G4int lFound = m_opticalSurfaceMap.count(p_name);
        if (lFound > 0)
        {
            return m_opticalSurfaceMap.at(p_name);
        }
        else
        {
            log_critical("Requested Optical Surface {} not found. Please check the name for typos!!", p_name);
            throw std::runtime_error("Requested Optical Surface not found: " + p_name);
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
        m_dataDirectory = directory;
        scannDataDirectory();
    }
}

/**
 * @brief Processes a data file based on its name prefix.
 *
 * The function identifies the type of data file based on
 * the prefix of the filename. It then calls the correct extraction method. For 'Surf' prefixed
 * files, it also updates the m_opticalSurfaceMap. For 'pmt_', 'om_', and
 * 'usr_' prefixed files, it invokes directly appendAndReturnTree without any
 * extra parsing into Geant4 objects.
 *
 * @param p_filePath Full path to the file.
 * @param p_fileName Name of the file (without the path).
 */
void OMSimInputData::processFile(const std::string &p_filePath,
                                 const std::string &p_fileName)
{
    log_trace("Processing file {}", p_fileName);
    OMSimMaterialHandler dataFile(p_filePath);
    switch (getFileType(p_fileName))
    {
    case FileType::IceCubeICE:
        dataFile.processMaterial();
        dataFile.processSpecial(IceProcessor::process);
        break;
    case FileType::Scintillator:
        dataFile.processSpecial(ScintillationProcessor::process);
        break;
    case FileType::Extra:
        dataFile.processExtraProperties();
        break;
    case FileType::Table:
        appendAndReturnTree(p_filePath);
        break;
    case FileType::Surface:
        m_opticalSurfaceMap[dataFile.GetName()] = dataFile.processSurface();
        break;
    case FileType::Material:
        dataFile.processMaterial();
        break;
    }
}

void OMSimInputData::scannDataDirectory()
{
    log_trace("Loading files in {}", m_dataDirectory);
    DIR *directory = opendir(m_dataDirectory.data());
    if (directory == NULL)
    {
        G4cerr << "Couldn't open directory" << m_dataDirectory << G4endl;
        return;
    }

    dirent *file;
    while ((file = readdir(directory)))
    {
        if (file->d_type != DT_REG) // ignore if not a regular file
        {
            continue;
        }

        const std::string p_fileName = file->d_name;
        std::string filePath = m_dataDirectory + "/" + p_fileName;

        processFile(filePath, p_fileName);
    }
    closedir(directory);
}

const std::unordered_map<std::string, OMSimInputData::FileType> OMSimInputData::fileTypePrefixes = {
    {"IceCubeICE", FileType::IceCubeICE},
    {"Scint", FileType::Scintillator},
    {"Extra", FileType::Extra},
    {"pmt_", FileType::Table},
    {"usr_", FileType::Table},
    {"Surf", FileType::Surface}};

OMSimInputData::FileType OMSimInputData::getFileType(const std::string &p_fileName) const
{
    for (const auto &[prefix, type] : fileTypePrefixes)
    {
        if (p_fileName.substr(0, prefix.length()) == prefix)
        {
            return type;
        }
    }
    return FileType::Material; // Default type if no prefix matches
}
