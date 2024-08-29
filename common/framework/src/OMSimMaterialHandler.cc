#include "OMSimLogger.hh"
#include "OMSimTools.hh"
#include "OMSimMaterialHandler.hh"
#include "OMSimInputData.hh"
#include <unordered_map>

OMSimMaterialHandler::OMSimMaterialHandler(const G4String &p_filePath)
    : m_filePath(p_filePath), m_material(nullptr), m_MPT(new G4MaterialPropertiesTable()),
      m_opticalSurface(nullptr), m_fileData(new ParameterTable())
{
  m_jsonTree = m_fileData->appendAndReturnTree(p_filePath);
  m_objectName = m_jsonTree.get<G4String>("jName");
}

OMSimMaterialHandler::~OMSimMaterialHandler()
{
  delete m_fileData;
  m_fileData = nullptr;
}

/**
 * @brief Process the material defined in the input file.
 */
void OMSimMaterialHandler::processMaterial()
{
  createMaterial();
  if (m_fileData->checkIfKeyInTree(m_objectName, "jProperties"))
    processProperties();
  if (m_fileData->checkIfKeyInTree(m_objectName, "jConstProperties"))
    processConstProperties();

  if (m_material && m_MPT)
  {
    m_material->SetMaterialPropertiesTable(m_MPT);
  }
}

/**
 * @brief Process and return an optical surface defined in the input file.
 * @return Pointer to the created G4OpticalSurface.
 */
G4OpticalSurface *OMSimMaterialHandler::processSurface()
{
  m_opticalSurface = new G4OpticalSurface(m_objectName);

  if (m_jsonTree.get_optional<G4String>("jModel"))
  {
    m_opticalSurface->SetModel(
        getOpticalSurfaceModel(m_jsonTree.get<std::string>("jModel")));
  }

  if (m_jsonTree.get_optional<G4String>("jType"))
  {
    m_opticalSurface->SetType(
        getSurfaceType(m_jsonTree.get<std::string>("jType")));
  }

  if (m_jsonTree.get_optional<G4String>("jFinish"))
  {
    m_opticalSurface->SetFinish(
        getOpticalSurfaceFinish(m_jsonTree.get<std::string>("jFinish")));
  }

  if (m_jsonTree.get_optional<G4double>("jSigmaAlpha"))
  {
    m_opticalSurface->SetSigmaAlpha(m_jsonTree.get<G4double>("jSigmaAlpha"));
  }
  if (m_fileData->checkIfKeyInTree(m_objectName, "jProperties"))
    processProperties();
  if (m_fileData->checkIfKeyInTree(m_objectName, "jConstProperties"))
    processConstProperties();

  m_opticalSurface->SetMaterialPropertiesTable(m_MPT);
  return m_opticalSurface;
}

/**
 * @brief Process special material types using a provided function.
 * @param p_processor Function pointer to the processor function.
 *
 * This method is used for processing special material types like IceCube ice
 * or scintillators, which require custom handling.
 */
void OMSimMaterialHandler::processSpecial(ProcessorFunction *p_processor)
{
  if (m_fileData->checkIfKeyInTree(m_objectName, "jMaterialName"))
    findMaterialPropertyTable(m_jsonTree.get<G4String>("jMaterialName"));

  p_processor(m_fileData, m_jsonTree, m_MPT);
}

/**
 * @brief Create a new Geant4 material based on the input data.
 */
void OMSimMaterialHandler::createMaterial()
{
  log_trace("Creating new material from file {} with name {}", m_filePath,
            m_objectName);

  G4NistManager *nistManager = G4NistManager::Instance();
  G4double density = m_fileData->getValueWithUnit(m_objectName, "jDensity");
  G4State state = getState(m_jsonTree.get<std::string>("jState"));

  m_material = new G4Material(
      m_objectName, density, m_jsonTree.get_child("jComponents").size(), state);

  for (const auto &component : m_jsonTree.get_child("jComponents"))
  {
    G4String componentName = component.first;
    double fraction = component.second.get_value<double>();
    G4Material *componentMaterial =
        nistManager->FindOrBuildMaterial(componentName);

    if (componentMaterial)
    {
      m_material->AddMaterial(componentMaterial, fraction);
    }
    else
    {
      Tools::throwError("Component material not found: " + componentName);
    }
  }
  log_trace("New Material defined: {}", m_material->GetName());
}

/**
 * @brief Process the properties defined in the input file and add them to the material.
 *
 * In the json file there is a jProperties key containing one ore more keys (which will be the name of the property in the material property table).
 * Each of these keys contains two arrays, one in key jWavelength and the other in jValues.
 */
void OMSimMaterialHandler::processProperties()
{
  log_trace("Extracting properties for material {} from file {}",
            m_objectName, m_filePath);

  for (const auto &property : m_jsonTree.get_child("jProperties"))
  {
    G4String key = property.first;
    G4double yAxisUnit = (key == "ABSLENGTH") ? mm : 1.0;
    addProperty(key, 1 / m_hc_eVnm, yAxisUnit, true, property.second);
  }
  log_trace("Finished extracting properties for material {} from file {}",
            m_objectName, m_filePath);
}

/**
 * @brief Process constant properties defined in the input file and add them to the material.
 *
 * The json files that contain a key "jConstProperties", will have a list of properties with a constant
 */
void OMSimMaterialHandler::processConstProperties()
{
  for (const auto &property : m_jsonTree.get_child("jConstProperties"))
  {
    G4String key = property.first;
    G4double value = m_fileData->getValueWithUnit(m_jsonTree.get<G4String>("jName"), "jConstProperties." + key);
    m_MPT->AddConstProperty(key.c_str(), value, true);
  }
}

/**
 * @brief Add a property to the material properties table.
 * @param p_propertyName Name of the property.
 * @param p_xUnit Unit conversion factor for energy values.
 * @param p_yUnit Unit conversion factor for property values.
 * @param p_invertX Whether to invert the x-axis values.
 * @param p_propertyTree Property tree containing the property data.
 */
void OMSimMaterialHandler::addProperty(const G4String &p_propertyName,
                                       G4double p_xUnit,
                                       G4double p_yUnit,
                                       bool p_invertX,
                                       const boost::property_tree::ptree &p_propertyTree)
{
  std::vector<G4double> xValues, yValues;

  for (const auto &item : p_propertyTree.get_child("jWavelength"))
  {
    xValues.push_back(item.second.get_value<G4double>() * p_xUnit);
  }

  for (const auto &item : p_propertyTree.get_child("jValue"))
  {
    yValues.push_back(item.second.get_value<G4double>() * p_yUnit);
  }

  if (xValues.size() != yValues.size())
  {
    Tools::throwError("Mismatch in property vector sizes for " + p_propertyName);
  }

  if (p_invertX)
  {
    for (auto &value : xValues)
    {
      value = 1 / value;
    }
  }

  Tools::sortVectorByReference(xValues, yValues);

  m_MPT->AddProperty(p_propertyName.c_str(), &xValues[0], &yValues[0], yValues.size());
}

/**
 * @brief Find the material property table for an existing material.
 * @param p_name Name of the material whose property table is to be found.
 */
void OMSimMaterialHandler::findMaterialPropertyTable(G4String p_name)
{
  G4Material *material = G4Material::GetMaterial(p_name);
  if (!material)
  {
    log_error("Trying to get material that does not exist...");
    return;
  }

  m_MPT = material->GetMaterialPropertiesTable();
}

/**
 * @brief Process extra properties for an existing material.
 *
 * This method is used to add additional properties to a material that has
 * already been defined.
 */
void OMSimMaterialHandler::processExtraProperties()
{
  G4String materialName = m_jsonTree.get<G4String>("jMaterialName");
  findMaterialPropertyTable(materialName);

  if (m_fileData->checkIfKeyInTree(m_objectName, "jProperties"))
    processProperties();
  if (m_fileData->checkIfKeyInTree(m_objectName, "jConstProperties"))
    processConstProperties();
}

/**
 * @brief Get the G4OpticalSurfaceFinish enum value from a string.
 * @param finish String representation of the finish.
 */
G4OpticalSurfaceFinish OMSimMaterialHandler::getOpticalSurfaceFinish(const std::string &p_finish)
{
  static const std::unordered_map<std::string, G4OpticalSurfaceFinish> finishMap = {
      {"polished", polished},
      {"polishedfrontpainted", polishedfrontpainted},
      {"polishedbackpainted", polishedbackpainted},
      {"ground", ground},
      {"groundfrontpainted", groundfrontpainted},
      {"groundbackpainted", groundbackpainted},
      {"polishedlumirrorair", polishedlumirrorair},
      {"polishedlumirrorglue", polishedlumirrorglue},
      {"polishedair", polishedair},
      {"polishedteflonair", polishedteflonair},
      {"polishedtioair", polishedtioair},
      {"polishedtyvekair", polishedtyvekair},
      {"polishedvm2000air", polishedvm2000air},
      {"polishedvm2000glue", polishedvm2000glue},
      {"etchedlumirrorair", etchedlumirrorair},
      {"etchedlumirrorglue", etchedlumirrorglue},
      {"etchedair", etchedair},
      {"etchedteflonair", etchedteflonair},
      {"etchedtioair", etchedtioair},
      {"etchedtyvekair", etchedtyvekair},
      {"etchedvm2000air", etchedvm2000air},
      {"etchedvm2000glue", etchedvm2000glue},
      {"groundlumirrorair", groundlumirrorair},
      {"groundlumirrorglue", groundlumirrorglue},
      {"groundair", groundair},
      {"groundteflonair", groundteflonair},
      {"groundtioair", groundtioair},
      {"groundtyvekair", groundtyvekair},
      {"groundvm2000air", groundvm2000air},
      {"groundvm2000glue", groundvm2000glue}};

  auto it = finishMap.find(p_finish);
  G4OpticalSurfaceFinish toReturn;
  if (it != finishMap.end())
  {
    toReturn = it->second;
  }
  else
  {
    Tools::throwError("Unknown G4OpticalSurfaceFinish: " + p_finish);
  }
  return toReturn;
}

/**
 * @brief Get the G4OpticalSurfaceModel enum value from a string.
 * @param p_model String representation of the model.
 */
G4OpticalSurfaceModel OMSimMaterialHandler::getOpticalSurfaceModel(const std::string &p_model)
{
  G4OpticalSurfaceModel lModel;
  if (p_model == "glisur")
    lModel = glisur;
  else if (p_model == "unified")
    lModel = unified;
  else if (p_model == "LUT")
    lModel = LUT;
  else
    Tools::throwError("Unknown G4OpticalSurfaceModel: " + p_model);

  return lModel;
}

/**
 * @brief Get the G4SurfaceType enum value from a string.
 * @param p_type String representation of the surface type.
 */
G4SurfaceType OMSimMaterialHandler::getSurfaceType(const std::string &p_type)
{
  static const std::unordered_map<std::string, G4SurfaceType> typeMap = {
      {"dielectric_metal", dielectric_metal},
      {"dielectric_dielectric", dielectric_dielectric},
      {"dielectric_LUT", dielectric_LUT},
      {"firsov", firsov},
      {"x_ray", x_ray},
      {"coated", coated}};
  G4SurfaceType toReturn;
  auto it = typeMap.find(p_type);
  if (it != typeMap.end())
  {
    toReturn = it->second;
  }
  else
  {
    Tools::throwError("Unknown G4SurfaceType: " + p_type);
  }
  return toReturn;
}

/**
 * @brief Get the G4State enum value from a string.
 * @param p_state String representation of the state.
 */
G4State OMSimMaterialHandler::getState(const std::string &p_state)
{
  if (p_state == "kStateSolid")
    return kStateSolid;
  else if (p_state == "kStateLiquid")
    return kStateLiquid;
  else if (p_state == "kStateGas")
    return kStateGas;
  else
    return kStateUndefined;
}

/**
 * @namespace ScintillationProcessor
 * @brief Namespace containing functions for processing files with scintillation properties.
 */
namespace ScintillationProcessor
{
  /**
   * @brief Process scintillation properties and add them to the material properties table.
   *
   * This function extracts scintillation properties from the provided data file and
   * sets up the material properties for scintillation simulation.
   *
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing scintillation data.
   * @param p_MPT Pointer to the G4MaterialPropertiesTable to be modified.
   */
  void process(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT)
  {
    log_trace("Extracting scintillation properties");

    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
    G4String lTemperature = args.keyExists("temperature") ? args.get<std::string>("temperature") : "-20";

    extractSpectrum(p_dataFile, p_jsonTree, p_MPT);
    extractLifeTimes(p_dataFile, p_jsonTree, p_MPT, lTemperature);
    extractYieldAlpha(p_dataFile, p_jsonTree, p_MPT, lTemperature);
    extractYieldElectron(p_dataFile, p_jsonTree, p_MPT, lTemperature);

    p_MPT->AddConstProperty("RESOLUTIONSCALE", 1.0);
    log_info("Added scintillation properties to: {}. It will only scintillate if scintillation process is in PhysicsList.", p_jsonTree.get<G4String>("jMaterialName"));
  }
  /**
   * @brief Extracts the scintillation spectrum from the data file and adds it to the material properties table.
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing scintillation data.
   * @param p_MPT Pointer to the G4MaterialPropertiesTable to be modified.
   */
  void extractSpectrum(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT)
  {
    std::vector<G4double> lSpectrumIntensity, lEnergy;
    p_dataFile->parseKeyContentToVector(lSpectrumIntensity, p_jsonTree, "jSpectrumIntensity", 1, false);
    p_dataFile->parseKeyContentToVector(lEnergy, p_jsonTree, "jSpectrumWavelength", h_Planck * c_light / nm, true);
    Tools::sortVectorByReference(lEnergy, lSpectrumIntensity);
    p_MPT->AddProperty("SCINTILLATIONCOMPONENT1", &lEnergy[0], &lSpectrumIntensity[0], static_cast<int>(lSpectrumIntensity.size()));
  }
  /**
   * @brief Retrieves the range of temperatures available for scintillation lifetimes.
   * @param p_jsonTree The JSON property tree containing scintillation data.
   * @param p_minTemperature Reference variable to receive the minimum temperature value.
   * @param p_maxTemperature Reference variable to receive the maximum temperature value.
   */
  void getLifeTimeTemperatureRange(const boost::property_tree::ptree &p_jsonTree, double &p_minTemperature, double &p_maxTemperature)
  {
    const auto &lifeTimeTree = p_jsonTree.get_child("Lifetimes");
    p_minTemperature = std::numeric_limits<double>::max();
    p_maxTemperature = std::numeric_limits<double>::min();

    for (const auto &item : lifeTimeTree)
    {
      double T = std::stod(item.first);
      p_minTemperature = std::min(p_minTemperature, T);
      p_maxTemperature = std::max(p_maxTemperature, T);
    }
  }

  /**
   * @brief Extracts lifetimes and amplitudes for a given temperature from the data.
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing scintillation data.
   * @param p_temperature The temperature for which to extract lifetimes.
   * @return Pair of vectors containing lifetimes and their corresponding amplitudes.
   */
  std::pair<std::vector<G4double>, std::vector<G4double>> extractLifeTimesForTemperature(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4String p_temperature)
  {
    auto temperatureSubTree = p_jsonTree.get_child("Lifetimes." + p_temperature);
    std::vector<G4double> times, amplitudes;
    p_dataFile->parseKeyContentToVector(times, temperatureSubTree, "jTimes", 1 * s, false);
    p_dataFile->parseKeyContentToVector(amplitudes, temperatureSubTree, "jAmplitudes", 1, false);
    return {times, amplitudes};
  }

  /**
   * @brief Adjusts amplitudes of lifetimes based on distance to investigated temperature.
   *
   * @param p_amplitude Reference to the vector of amplitudes to be adjusted.
   * @param p_T1 Investigated temperature.
   * @param p_T2 Actual temperature from the data.
   */
  void weightLifeTimesAmplitudes(std::vector<G4double> &p_amplitude, double p_T1, double p_T2)
  {
    double weight = 1.0 / std::abs(p_T1 - p_T2);
    for (auto &amp : p_amplitude)
      amp *= weight;
  }

  /**
   * @brief Extracts the scintillation lifetimes from the file and weights them for a specific temperature.
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing lifetime data.
   * @param p_MPT Pointer to the G4MaterialPropertiesTable to be modified.
   * @param pTemperature The temperature for which to extract lifetimes.
   */
  void extractLifeTimes(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT, G4String p_temperature)
  {
    log_trace("Extracting lifetimes at temperature {}.", p_temperature);
    std::vector<G4double> allLifeTimes, allAmplitudes;
    double t1 = std::stod(p_temperature);
    double minTemperature, maxTemperature;
    getLifeTimeTemperatureRange(p_jsonTree, minTemperature, maxTemperature);

    if (t1 < minTemperature || t1 > maxTemperature)
    {
      log_error("Temperature is out of the range of measured temperatures!");
      return;
    }

    // Check if the reference temperature matches a measured temperature directly
    if (p_jsonTree.get_child_optional("Lifetimes." + p_temperature))
    {
      auto [times, amplitudes] = extractLifeTimesForTemperature(p_dataFile, p_jsonTree, p_temperature);
      allLifeTimes = times;
      allAmplitudes = amplitudes;
    }
    else
    {
      // Loop through all measured temperatures and weight amplitudes based on distance to reference temperature
      for (const auto &item : p_jsonTree.get_child("Lifetimes"))
      {
        G4String currentTemperature = item.first;
        double t2 = std::stod(currentTemperature);
        auto [times, amplitudes] = extractLifeTimesForTemperature(p_dataFile, p_jsonTree, currentTemperature);
        weightLifeTimesAmplitudes(amplitudes, t1, t2);
        allLifeTimes.insert(allLifeTimes.end(), times.begin(), times.end());
        allAmplitudes.insert(allAmplitudes.end(), amplitudes.begin(), amplitudes.end());
      }
    }

    Tools::sortVectorByReference(allAmplitudes, allLifeTimes);
    p_MPT->AddProperty("FRACTIONLIFETIMES", &allAmplitudes[0], &allLifeTimes[0], static_cast<int>(allLifeTimes.size()), true);
  }

  /**
   * @brief Extract the yield from json tree.
   *
   * The data is the yield at different temperatures and is interpolated with a TGraph to the investigated temperature. If a user argument with a yield, it will overwrite the extracted data.
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing yield data.
   * @param p_MPT Pointer to the G4MaterialPropertiesTable to be modified.
   * @param p_temperature Temperature for which the yield is interpolated.
   * @param p_yieldPropertyName Name of the yield property (Geant4 internal).
   * @param p_argKey Command arguments key.
   * @param p_treeKeyTemperature Key to access temperature array in the tree.
   * @param p_treeKeyYield Key to access yield array in the tree.
   */
  void extractYield(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT,
                    G4String p_temperature, G4String p_yieldPropertyName, G4String p_argKey, G4String p_treeKeyTemperature, G4String p_treeKeyYield)
  {
    log_trace("Calculating yield for temperature {}.", p_temperature);
    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
    if (args.keyExists(p_argKey))
    {
      log_warning("Given scintillation yield will override the yield for ALL scintillating materials defined via ScintillationProperties class!");
      p_MPT->AddConstProperty(p_yieldPropertyName, args.get<G4double>(p_argKey) / MeV, true);
    }
    else
    {
      std::vector<G4double> temperatures, yields;
      try
      {
        p_dataFile->parseKeyContentToVector(temperatures, p_jsonTree, p_treeKeyTemperature, 1, false);
        p_dataFile->parseKeyContentToVector(yields, p_jsonTree, p_treeKeyYield, 1, false);
      }
      catch (...)
      {
        log_error("Error parsing the JSON for key: {}", p_treeKeyYield);
        Tools::throwError("Error parsing the JSON for key: " + p_treeKeyYield);
      }

      TGraph m_YieldInterpolation(static_cast<int>(temperatures.size()), &temperatures[0], &yields[0]);
      p_MPT->AddConstProperty(p_yieldPropertyName, m_YieldInterpolation.Eval(std::stod(p_temperature)) / MeV, true);
    }
  }
  /**
   * @brief Extracts and interpolates the alpha particle scintillation yield for a given temperature.
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing yield data.
   * @param p_MPT Pointer to the G4MaterialPropertiesTable to be modified.
   * @param p_temperature The temperature for which to extract yield.
   */
  void extractYieldAlpha(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT, G4String p_temperature)
  {
    extractYield(p_dataFile, p_jsonTree, p_MPT, p_temperature, "SCINTILLATIONYIELD", "yield_alphas", "jYieldAlphaTemperature", "jYieldAlpha");
  }
  /**
   * @brief Extracts and interpolates the electron scintillation yield for a given temperature.
   * Uses the alpha yield as fallback if electron yield is not given.
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing yield data.
   * @param p_MPT Pointer to the G4MaterialPropertiesTable to be modified.
   * @param p_temperature The temperature for which to extract yield.
   */
  void extractYieldElectron(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT, G4String p_temperature)
  {
    try
    {
      extractYield(p_dataFile, p_jsonTree, p_MPT, p_temperature, "SCINTILLATIONYIELDELECTRONS", "yield_electrons", "jYieldElectronTemperature", "jYieldElectron");
    }
    catch (...)
    {
      log_warning("Electron yield not found, using alpha yield.");
      extractYield(p_dataFile, p_jsonTree, p_MPT, p_temperature, "SCINTILLATIONYIELDELECTRONS", "yield_alphas", "jYieldAlphaTemperature", "jYieldAlpha");
    }
  }

}

/**
 * @namespace IceProcessor
 * @brief Namespace containing functions for processing IceCube ice properties from file.
 */
namespace IceProcessor
{

  /**
   * @brief Process IceCube ice properties and create corresponding materials.
   *
   * This function extracts ice properties from the provided data, calculates
   * various optical properties, and sets up the material properties for
   * IceCube ice simulation.
   *
   * @param p_dataFile Pointer to the ParameterTable containing file data.
   * @param p_jsonTree The JSON property tree containing ice data.
   * @param p_MPT Pointer to the G4MaterialPropertiesTable to be modified.
   */
  void process(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT)

  {
    const G4double mieSpiceConstants[3] = {0.972, 0.0000001, 1};

    std::vector<G4double> mieScatteringLength, wavelength, refractionIndex, refractionIndexEnergy, absLength, spiceBe400inv, spiceA400inv, spiceDepth;

    p_dataFile->parseKeyContentToVector(refractionIndexEnergy, p_jsonTree, "jWavelength_spice", h_Planck * c_light / nm, true);
    p_dataFile->parseKeyContentToVector(wavelength, p_jsonTree, "jWavelength_spice", 1 * nm, false);
    p_dataFile->parseKeyContentToVector(spiceBe400inv, p_jsonTree, "jbe400inv_spice", 1 * m, false);
    p_dataFile->parseKeyContentToVector(spiceA400inv, p_jsonTree, "ja400inv_spice", 1 * m, false);
    p_dataFile->parseKeyContentToVector(spiceDepth, p_jsonTree, "jDepth_spice", 1 * m, false);
    for (int u = 0; u < static_cast<int>(refractionIndexEnergy.size()); u++)
    {
      refractionIndex.push_back(spiceRefraction(wavelength.at(u)));
      absLength.push_back(spiceAbsorption(wavelength.at(u), spiceA400inv, spiceDepth));
      mieScatteringLength.push_back(mieScattering(wavelength.at(u), spiceBe400inv));
    }

    // give refractive index to IceCubeICE. This is used also for IceCubeICE_SPICE
    p_MPT->AddProperty("RINDEX", &refractionIndexEnergy[0], &refractionIndex[0], static_cast<int>(refractionIndex.size()));

    G4NistManager *materialDataBase = G4NistManager::Instance();
    G4Material *spice = new G4Material("IceCubeICE_SPICE", p_dataFile->getValueWithUnit(p_jsonTree.get<G4String>("jName"), "jDensity"), materialDataBase->FindOrBuildMaterial("G4_WATER"), kStateSolid);
    // give properties to IceCubeICE_SPICE
    G4MaterialPropertiesTable *spiceMPT = new G4MaterialPropertiesTable();
    spiceMPT->AddProperty("RINDEX", &refractionIndexEnergy[0], &refractionIndex[0], static_cast<int>(refractionIndex.size()));
    spiceMPT->AddProperty("ABSLENGTH", &refractionIndexEnergy[0], &absLength[0], static_cast<int>(absLength.size()));
    spiceMPT->AddProperty("MIEHG", &refractionIndexEnergy[0], &mieScatteringLength[0], static_cast<int>(refractionIndex.size())); //->SetSpline(true);
    spiceMPT->AddConstProperty("MIEHG_FORWARD", mieSpiceConstants[0]);
    spiceMPT->AddConstProperty("MIEHG_BACKWARD", mieSpiceConstants[1]);
    spiceMPT->AddConstProperty("MIEHG_FORWARD_RATIO", mieSpiceConstants[2]);
    spice->SetMaterialPropertiesTable(spiceMPT);
  }
  /**
   * @brief Calculate temperature of ice depending on the depth.
   *
   * This function is needed for the calculation of scattering and absorption length of the ice.
   *
   * @param p_depth Depth in m from where we need the temperature.
   * @return Temperature in Kelvin.
   */
  G4double spiceTemperature(G4double p_depth)
  {
    G4double lSpiceTemperature = 221.5 - 0.00045319 / m * p_depth + 5.822e-6 / m2 * pow(p_depth, 2.);
    return lSpiceTemperature;
  }

  /**
   * @brief Calculate absorption length of IceCube's ice for a specific wavelength.
   *
   * @param pLambd Wavelength in nm.
   * @return Absorption length in m.
   */
  G4double spiceAbsorption(G4double p_lambda, const std::vector<G4double> &p_spicea400inv, const std::vector<G4double> &p_spiceDepth)
  {
    int spiceDepthPosition = OMSimCommandArgsTable::getInstance().get<int>("depth_pos");
    G4double lKappa = 1.08;
    G4double lParamA = 6954. / m;
    G4double lParamB = 6618 * nm;
    G4double lAdust = 1. / (p_spicea400inv[spiceDepthPosition]) * pow(p_lambda / (400. * nm), -lKappa);
    G4double lDeltaTau = spiceTemperature(p_spiceDepth[spiceDepthPosition]) - spiceTemperature(1730.);
    G4double lAinv = 1. / (lAdust + lParamA * exp(-lParamB / p_lambda) * (1. + 0.01 * lDeltaTau));
    return lAinv;
  }

  /**
   * @brief Calculate refraction index of IceCube's ice for a specific wavelength.
   *
   * @param pLambd Wavelength in nm.
   * @return Refraction index.
   */
  G4double spiceRefraction(G4double p_lambda)
  {
    // unknown depth. Parametrization by Thomas Kittler.
    G4double lambda3 = p_lambda * 1e-3;
    G4double nPhase = 1.55749 - 1.57988 / nm * lambda3 + 3.99993 / (nm * nm) * pow(lambda3, 2) - 4.68271 / (nm * nm * nm) * pow(lambda3, 3) + 2.09354 / (nm * nm * nm * nm) * pow(lambda3, 4);
    return nPhase; // using this now after discussion with Timo
  }

  /**
   * @brief Calculate mie scattering length of IceCube's ice for a specific wavelength.
   *
   * @param p_lambda Wavelength in nm.
   * @return Mie scattering length in m.
   */
  G4double mieScattering(G4double p_lambda, const std::vector<G4double> &p_spicebe400inv)
  {
    // depth_pos is the coordinate for the chosen depth in Depth_spice. For example to choose
    // depth=2278.2 m, we use depth_pos = 88
    int spiceDepthPosition = OMSimCommandArgsTable::getInstance().get<int>("depth_pos");
    G4double lAlpha = 0.90;
    G4double lAv_costheta = 0.9;
    G4double lBe_inv = 1. / (1. / (p_spicebe400inv[spiceDepthPosition]) * pow((p_lambda / (400. * nm)), -lAlpha));
    G4double lB_inv = lBe_inv * (1. - lAv_costheta);
    return lB_inv;
  }

}