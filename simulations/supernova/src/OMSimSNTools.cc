#include "OMSimSNTools.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4VTouchable.hh"
#include <stdlib.h>
#include "G4TransportationManager.hh"
#include "G4TouchableHistoryHandle.hh"
#include "OMSimCommandArgsTable.hh"
#include <TGraph.h>

/**
 * @brief Retrieves file names for neutrino and antineutrino fluxes based on a specified supernova model.
 *
 * This function provides the corresponding file names for neutrino and antineutrino fluxes
 * depending on the supernova model identified by the input p_value.
 * Files are located in "supernova/models"
 *
 * @param p_value The integer identifier representing a specific supernova model.
 * @return A pair containing the neutrino and antineutrino flux file names.
 */
std::pair<std::string, std::string> OMSimSNTools::getFileNames(int p_value)
{
    const std::string basePath = "../supernova/models/";

    const std::map<int, std::pair<std::string, std::string>> fileMap = {
        {0, {basePath + "Flux_Nu_Heavy_ls220.cfg", basePath + "Flux_Nubar_Heavy_ls220.cfg"}}, // 27 solar masses type II model
        {1, {basePath + "Flux_Nu_light_ls220.cfg", basePath + "Flux_Nubar_light_ls220.cfg"}}, // 9.6 solar masses type II model
        {2, {basePath + "nu_DDT.cfg", basePath + "nubar_DDT.cfg"}},                          // type 1 SN
        {3, {basePath + "nu_GCD.cfg", basePath + "nubar_GCD.cfg"}},                          // type 1 SN
        {4, {basePath + "Flux_Nu_tailSN.cfg", basePath + "Flux_Nubar_tailSN.cfg"}}           // long tailed type II
    };

    auto it = fileMap.find(p_value);
    if (it != fileMap.end())
    {
      log_debug("Using SN model files {} {}" , it->second.first, it->second.second);
      return it->second;
    }
    else
    {
        log_error("ERROR!! Choose a valid SN model");
        throw std::invalid_argument("Invalid p_value for SN model");
    }
  }
/**
 * @brief Checks if a given position is within any of the modules.
 *
 * This method determines whether the provided position lies in the generation medium (ice) It evaluates the position
 * in the context of the global environment and examines the history depth to make this determination.
 *
 * @param p_position The 3D position vector to be checked against module boundaries.
 * @warning This relies on the generation module being a single volume (generally, the ice) and the mother volume of everything else
 * If the ice is modified to be composed of different subvolumes (like, for example, if the bubble column is added)
 * this method would need to be modify!
 *
 * @return True if the position is inside a module; otherwise, false.
 */
bool OMSimSNTools::checkVolumeForOMs(G4ThreeVector p_position)
{
  G4Navigator* navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  navigator->LocateGlobalPointAndSetup(p_position);
  G4TouchableHistoryHandle touchable = navigator->CreateTouchableHistoryHandle();
  G4int historyDepth = touchable->GetHistoryDepth();
  return historyDepth > 0;
}

/**
 * @brief Randomly determines a vertex position within the generation volume.
 *
 * The vertex is considered valid only if it is located within the ice, ensuring
 * it is not situated inside the module or other predefined volumes.
 *
 * @return A valid vertex position within the ice.
 */
G4ThreeVector OMSimSNTools::randomPosition()
{
  // Assume that the world is a cylinder
  double height = OMSimCommandArgsTable::getInstance().get<G4double>("wheight") * m;
  double radius = OMSimCommandArgsTable::getInstance().get<G4double>("wradius") * m;

  // Maximum length of generation cylinder "maxR"
  G4double maxR = radius * sqrt(3); // Using sqrt() here

  G4double xPos, yPos, zPos;
  G4ThreeVector position;
  G4double distanceToCentre;
  G4double verticalDistance;
  bool positionInOM = true;

  while (positionInOM || distanceToCentre >= maxR || verticalDistance >= radius)
  {
    // Condense random sign generation
    G4double xSign = (G4UniformRand() < 0.5) ? -1 : 1;
    G4double ySign = (G4UniformRand() < 0.5) ? -1 : 1;
    G4double zSign = (G4UniformRand() < 0.5) ? -1 : 1;

    zPos = zSign * (G4UniformRand() * height);
    xPos = xSign * (G4UniformRand() * radius);
    yPos = ySign * (G4UniformRand() * radius);

    position.set(xPos, yPos, zPos);

    verticalDistance = sqrt(xPos * xPos + yPos * yPos);
    distanceToCentre = sqrt(xPos * xPos + yPos * yPos + zPos * zPos);

    positionInOM = checkVolumeForOMs(position);
  }

  return position;
}

/**
 * @brief Randomly samle the energy depending on the properties of the energy spectrum.
 *
 * Build the energy distribution Fe(E) given the input parameters, and uses the inverse CDF
 * algorithm to sample a p_value for the neutrino energy from Fe(E).
 *
 * @param p_meanEnergy The primary mean energy p_value from the supernova model.
 * @param p_meanESquared The primary squared mean energy p_value from the supernova model.
 * @param p_alpha Pinching parameter of the energy spectrum.
 * @return The sampled energy p_value based on the given model.
 */
G4double OMSimSNTools::sampleEnergy(G4double p_meanEnergy, G4double p_meanESquared, G4double &p_alpha)
{
    G4int numberPoints = 500;
    
    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false)
    {
        p_alpha = getAlpha(p_meanEnergy, p_meanESquared);
    }

    const G4double min = 0.;
    const G4double max = 80.;
    const G4double delta = (max - min) / G4double(numberPoints - 1);

    std::vector<G4double> x(numberPoints);
    std::vector<G4double> y(numberPoints);

    for (G4int i = 0; i < numberPoints; i++)
    {
        x[i] = (min + i * delta);                                             // Energy
        y[i] = pow(x[i], p_alpha) * exp(-(p_alpha + 1.) * x[i] / p_meanEnergy); // F(e), energy dist. function
    }
    
    DistributionSampler energyDistributionSampler;
    energyDistributionSampler.setData(x, y, "EnergyDistributionSampler");
    energyDistributionSampler.setUnits(1 * MeV, 1.);

    return energyDistributionSampler.sampleFromDistribution();
}
/**
 * @brief Calculates pinching parameter of energy spectrum
 *
 * Given the mean energy and squared mean energy of the neutrino event, calculates the pinching parameter following
 * equation 3 in
 * Irene Tamborra et al., "High-resolution supernova neutrino spectra represented by a simple fit," PHYSICAL REVIEW D 86, 125031 (2012).
 * https://arxiv.org/abs/1211.3920
 *
 * @param p_meanEnergy The primary mean energy p_value from the supernova model.
 * @param p_meanESquared The primary squared mean energy p_value from the supernova model.
 * @return Pinching parameter of the energy spectrum.
 */
G4double OMSimSNTools::getAlpha(G4double p_meanEnergy, G4double p_meanESquared)
{
  return (2 * pow(p_meanEnergy, 2) - p_meanESquared) / (p_meanESquared - pow(p_meanEnergy, 2));
}


/**
 * @brief Calculates the number of target particles in ice per cubic meter.
 *
 * This function determines the number of target particles, given the number of targets per molecule,
 * assuming the ice is pure H2O. The density and molar mass values are based on ice at -50°C.
 *
 * @param p_numberTargetPerMolecule Number of target particles per H2O molecule.
 * @return Total number of target particles per cubic meter in ice.
 */
G4double OMSimSNTools::numberOfTargets(G4int p_numberTargetPerMolecule)
{
  G4double density = 921.6 * kg / m3;    // Density of ice at -50 celsius degrees
  G4double molarMass = 18.01528e-3 * kg; // kg per mol
  G4double avogadroNumber = 6.022140857e23;
  G4double numberMolecules = density / molarMass * avogadroNumber; // molecules/m^3 of ice
  G4double numberTargets = numberMolecules * p_numberTargetPerMolecule;
  return numberTargets;
}



/**
 * @brief Calculates the interaction weight based on the cross section and number of targets.
 *
 * This method computes the interaction weight of neutrinos in the ice using the formula:
 * `weight = p_sigma * Nt * r`,
 * where:
 * - `p_sigma` is the cross section,
 * - `Nt` is the number of targets in the ice (considered as electrons),
 * - `r` is the distance the neutrino travels in the ice.
 *
 * @param p_sigma The cross section p_value.
 * @param p_numberTargs The number of targets present in the ice for the corresponding interaction.
 * @warning It's crucial to note that this formulation is specifically valid for SN neutrinos
 * approaching from the Z axis in a world defined as a cylinder. For any other geometry or conditions,
 * a different approach or adjustments might be required.
 * @return The computed interaction weight for the neutrinos based on the provided parameters.
 */
G4double OMSimSNTools::calculateWeight(G4double p_sigma, G4double p_numberTargs)
{
  return p_sigma * p_numberTargs * (2 * OMSimCommandArgsTable::getInstance().get<G4double>("wheight") * m);
}



/**
 * @brief Samples values from a given distribution using the Inverse CDF algorithm.
 *
 * This method employs the inverse cumulative distribution function (Inverse CDF) technique
 * to derive samples from a specified distribution. It's useful for generating random
 * values in accordance with the underlying distribution shape.
 *
 * @return A randomly selected x-p_value sampled in line with the provided distribution.
 */
G4double DistributionSampler::sampleFromDistribution()
{
  // Total number of points
  G4int p_numberPoints = static_cast<G4int>(m_X.size());

  // Randomly choose a y-p_value from the CDF's range
  G4double yRandom = G4UniformRand() * m_CDF[p_numberPoints - 1];
  // Find the bin corresponding to the y-p_value
  G4int j = p_numberPoints - 2;
  while (m_CDF[j] > yRandom && j > 0)
    j--;

  // Convert random y-p_value to a random x-p_value
  // The conversion is based on the fact that the CDF is represented as a second-order polynomial
  G4double xRandom = m_X[j];
  G4double slope = m_slopes[j];

  if (slope != 0.)
  {
    G4double b = m_Y[j] / slope;
    G4double c = 2 * (yRandom - m_CDF[j]) / slope;
    G4double delta = b * b + c;
    G4int sign = (slope < 0.) ? -1 : 1;
    xRandom += sign * std::sqrt(delta) - b;
  }
  else if (m_Y[j] > 0.)
  {
    xRandom += (yRandom - m_CDF[j]) / m_Y[j];
  }

  return xRandom*m_XUnit;
}

/**
 * @brief Sets data for the distribution.
 * 
 * This method sets the x and y data points for the distribution and assigns a name to it for the TGraph (if used).
 * 
 * @param p_x Vector containing x-values of the data.
 * @param p_y Vector containing y-values of the data.
 * @param p_name Name for the distribution, used for debugging and TGraph name assignment.
 */
void DistributionSampler::setData(const std::vector<G4double> &p_x, const std::vector<G4double> &p_y, G4String p_name)
{
  m_X = p_x;
  m_Y = p_y;
  m_distName = p_name; // TGraph needs unique name, also good for debugging
  calculateSlopesAndCDF();
}


/**
 * @brief Initializes the interpolator using TGraph.
 * 
 * This method creates a new TGraph instance with the stored data points 
 * and sets its name to the name of the distribution.
 */
void DistributionSampler::makeInterpolator()
{
  m_interpolator = new TGraph(static_cast<int>(m_X.size()), m_X.data(), m_Y.data());
  m_interpolator->SetName(m_distName);
}


/**
 * @brief Calculates the slopes and cumulative function of a dataset for the inverse cumulative algorithm.
 *
 * This function computes the slopes and cumulative function values for the given dataset.
 * These results are often used in the inverse cumulative algorithm.
 */
void DistributionSampler::calculateSlopesAndCDF()
{
  G4int numberPoints = static_cast<G4int>(m_X.size());
  m_slopes.resize(numberPoints, 0.0); // Initialize with zeros
  m_CDF.resize(numberPoints, 0.0);    // Initialize with zeros
  for (G4int j = 0; j < numberPoints; j++)
  {
    // Compute slopes for all except the last point
    if (j < numberPoints - 1)
    {
      m_slopes[j] = (m_Y[j + 1] - m_Y[j]) / (m_X[j + 1] - m_X[j]);
    }

    // Compute cumulative function
    if (j > 0)
    {
      m_CDF[j] = m_CDF[j - 1] + 0.5 * (m_Y[j] + m_Y[j - 1]) * (m_X[j] - m_X[j - 1]);
    }
  }
}

/**
 * @brief Interpolates distribution using TGraph (ROOT)
 * @param x-p_value
 * @return The interpolated y-p_value for the given x.
 */
G4double DistributionSampler::interpolate(G4double p_x)
{
  if (!m_interpolator)
  {
    log_error("Error: m_interpolator not initialized!, make sure running makeInterpolator() before interpolate()");
    throw("Error: m_interpolator not initialized!");
    return 0.0;
  }
  return m_interpolator->Eval(p_x/m_XUnit) * m_YUnit;
}


/**
 * @brief Sets units for the x and y data points of the distribution.
 * @param p_x Unit for x-values.
 * @param p_y Unit for y-values.
 */
void DistributionSampler::setUnits(G4double p_x, G4double p_y)
{
  m_YUnit = p_y;
  m_XUnit = p_x;
}

DistributionSampler::~DistributionSampler()
{
  if (m_interpolator)
  {
    delete m_interpolator;
    m_interpolator = nullptr;
  }
};