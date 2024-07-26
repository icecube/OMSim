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
 * depending on the supernova model identified by the input value.
 * Files are located in "supernova/models"
 *
 * @param value The integer identifier representing a specific supernova model.
 * @return A pair containing the neutrino and antineutrino flux file names.
 */
std::pair<std::string, std::string> OMSimSNTools::getFileNames(int value)
{
    const std::string basePath = "../supernova/models/";

    const std::map<int, std::pair<std::string, std::string>> fileMap = {
        {0, {basePath + "Flux_Nu_Heavy_ls220.cfg", basePath + "Flux_Nubar_Heavy_ls220.cfg"}}, // 27 solar masses type II model
        {1, {basePath + "Flux_Nu_light_ls220.cfg", basePath + "Flux_Nubar_light_ls220.cfg"}}, // 9.6 solar masses type II model
        {2, {basePath + "nu_DDT.cfg", basePath + "nubar_DDT.cfg"}},                          // type 1 SN
        {3, {basePath + "nu_GCD.cfg", basePath + "nubar_GCD.cfg"}},                          // type 1 SN
        {4, {basePath + "Flux_Nu_tailSN.cfg", basePath + "Flux_Nubar_tailSN.cfg"}}           // long tailed type II
    };

    auto it = fileMap.find(value);
    if (it != fileMap.end())
    {
      log_debug("Using SN model files {} {}" , it->second.first, it->second.second);
      return it->second;
    }
    else
    {
        log_error("ERROR!! Choose a valid SN model");
        throw std::invalid_argument("Invalid value for SN model");
    }
  }
/**
 * @brief Checks if a given position is within any of the modules.
 *
 * This method determines whether the provided position lies in the generation medium (ice) It evaluates the position
 * in the context of the global environment and examines the history depth to make this determination.
 *
 * @param pPosition The 3D position vector to be checked against module boundaries.
 * @warning This relies on the generation module being a single volume (generally, the ice) and the mother volume of everything else
 * If the ice is modified to be composed of different subvolumes (like, for example, if the bubble column is added)
 * this method would need to be modify!
 *
 * @return True if the position is inside a module; otherwise, false.
 */
bool OMSimSNTools::checkVolumeForOMs(G4ThreeVector pPosition)
{
  G4Navigator* lNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
  lNavigator->LocateGlobalPointAndSetup(pPosition);
  G4TouchableHistoryHandle lTouchable = lNavigator->CreateTouchableHistoryHandle();
  G4int lHistoryDepth = lTouchable->GetHistoryDepth();
  return lHistoryDepth > 0;
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
  double lHeight = OMSimCommandArgsTable::getInstance().get<G4double>("wheight") * m;
  double lRadius = OMSimCommandArgsTable::getInstance().get<G4double>("wradius") * m;

  // Maximum length of generation cylinder "lMaxR"
  G4double lMaxR = lRadius * sqrt(3); // Using sqrt() here

  G4double lPos_x, lPos_y, lPos_z;
  G4ThreeVector lPosition;
  G4double lDistanceToCentre;
  G4double lVerticalDistance;
  bool lPositionInOM = true;

  while (lPositionInOM || lDistanceToCentre >= lMaxR || lVerticalDistance >= lRadius)
  {
    // Condense random sign generation
    G4double lSign_x = (G4UniformRand() < 0.5) ? -1 : 1;
    G4double lSign_y = (G4UniformRand() < 0.5) ? -1 : 1;
    G4double lSign_z = (G4UniformRand() < 0.5) ? -1 : 1;

    lPos_z = lSign_z * (G4UniformRand() * lHeight);
    lPos_x = lSign_x * (G4UniformRand() * lRadius);
    lPos_y = lSign_y * (G4UniformRand() * lRadius);

    lPosition.set(lPos_x, lPos_y, lPos_z);

    lVerticalDistance = sqrt(lPos_x * lPos_x + lPos_y * lPos_y);
    lDistanceToCentre = sqrt(lPos_x * lPos_x + lPos_y * lPos_y + lPos_z * lPos_z);

    lPositionInOM = checkVolumeForOMs(lPosition);
  }

  return lPosition;
}

/**
 * @brief Randomly samle the energy depending on the properties of the energy spectrum.
 *
 * Build the energy distribution Fe(E) given the input parameters, and uses the inverse CDF
 * algorithm to sample a value for the neutrino energy from Fe(E).
 *
 * @param pMeanEnergy The primary mean energy value from the supernova model.
 * @param pMeanESquared The primary squared mean energy value from the supernova model.
 * @param pAlpha Pinching parameter of the energy spectrum.
 * @return The sampled energy value based on the given model.
 */
G4double OMSimSNTools::sampleEnergy(G4double pMeanEnergy, G4double pMeanESquared, G4double &pAlpha)
{
    G4int lNrPoints = 500;
    
    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false)
    {
        pAlpha = getAlpha(pMeanEnergy, pMeanESquared);
    }

    const G4double lMin = 0.;
    const G4double lMax = 80.;
    const G4double delta = (lMax - lMin) / G4double(lNrPoints - 1);

    std::vector<G4double> lX(lNrPoints);
    std::vector<G4double> lY(lNrPoints);

    for (G4int i = 0; i < lNrPoints; i++)
    {
        lX[i] = (lMin + i * delta);                                             // Energy
        lY[i] = pow(lX[i], pAlpha) * exp(-(pAlpha + 1.) * lX[i] / pMeanEnergy); // F(e), energy dist. function
    }
    
    DistributionSampler lEnergyDistributionSampler;
    lEnergyDistributionSampler.setData(lX, lY, "EnergyDistributionSampler");
    lEnergyDistributionSampler.setUnits(1 * MeV, 1.);

    return lEnergyDistributionSampler.sampleFromDistribution();
}
/**
 * @brief Calculates pinching parameter of energy spectrum
 *
 * Given the mean energy and squared mean energy of the neutrino event, calculates the pinching parameter following
 * equation 3 in
 * Irene Tamborra et al., "High-resolution supernova neutrino spectra represented by a simple fit," PHYSICAL REVIEW D 86, 125031 (2012).
 * https://arxiv.org/abs/1211.3920
 *
 * @param pMeanEnergy The primary mean energy value from the supernova model.
 * @param pMeanESquared The primary squared mean energy value from the supernova model.
 * @return Pinching parameter of the energy spectrum.
 */
G4double OMSimSNTools::getAlpha(G4double pMeanEnergy, G4double pMeanESquared)
{
  return (2 * pow(pMeanEnergy, 2) - pMeanESquared) / (pMeanESquared - pow(pMeanEnergy, 2));
}


/**
 * @brief Calculates the number of target particles in ice per cubic meter.
 *
 * This function determines the number of target particles, given the number of targets per molecule,
 * assuming the ice is pure H2O. The density and molar mass values are based on ice at -50°C.
 *
 * @param pNrTargetPerMolecule Number of target particles per H2O molecule.
 * @return Total number of target particles per cubic meter in ice.
 */
G4double OMSimSNTools::numberOfTargets(G4int pNrTargetPerMolecule)
{
  G4double lDensity = 921.6 * kg / m3;    // Density of ice at -50 celsius degrees
  G4double lMolarMass = 18.01528e-3 * kg; // kg per mol
  G4double lAvogadroNumber = 6.022140857e23;
  G4double lNrMolecules = lDensity / lMolarMass * lAvogadroNumber; // molecules/m^3 of ice
  G4double lNrTargets = lNrMolecules * pNrTargetPerMolecule;
  return lNrTargets;
}



/**
 * @brief Calculates the interaction weight based on the cross section and number of targets.
 *
 * This method computes the interaction weight of neutrinos in the ice using the formula:
 * `weight = pSigma * Nt * r`,
 * where:
 * - `pSigma` is the cross section,
 * - `Nt` is the number of targets in the ice (considered as electrons),
 * - `r` is the distance the neutrino travels in the ice.
 *
 * @param pSigma The cross section value.
 * @param pNrTargets The number of targets present in the ice for the corresponding interaction.
 * @warning It's crucial to note that this formulation is specifically valid for SN neutrinos
 * approaching from the Z axis in a world defined as a cylinder. For any other geometry or conditions,
 * a different approach or adjustments might be required.
 * @return The computed interaction weight for the neutrinos based on the provided parameters.
 */
G4double OMSimSNTools::calculateWeight(G4double pSigma, G4double pNrTargets)
{
  return pSigma * pNrTargets * (2 * OMSimCommandArgsTable::getInstance().get<G4double>("wheight") * m);
}



/**
 * @brief Samples values from a given distribution using the Inverse CDF algorithm.
 *
 * This method employs the inverse cumulative distribution function (Inverse CDF) technique
 * to derive samples from a specified distribution. It's useful for generating random
 * values in accordance with the underlying distribution shape.
 *
 * @return A randomly selected x-value sampled in line with the provided distribution.
 */
G4double DistributionSampler::sampleFromDistribution()
{
  // Total number of points
  G4int pNrPoints = static_cast<G4int>(mX.size());

  // Randomly choose a y-value from the CDF's range
  G4double lYrndm = G4UniformRand() * mCDF[pNrPoints - 1];
  // Find the bin corresponding to the y-value
  G4int j = pNrPoints - 2;
  while (mCDF[j] > lYrndm && j > 0)
    j--;

  // Convert random y-value to a random x-value
  // The conversion is based on the fact that the CDF is represented as a second-order polynomial
  G4double lXrndm = mX[j];
  G4double lSlope = mSlopes[j];

  if (lSlope != 0.)
  {
    G4double b = mY[j] / lSlope;
    G4double c = 2 * (lYrndm - mCDF[j]) / lSlope;
    G4double lDelta = b * b + c;
    G4int lSign = (lSlope < 0.) ? -1 : 1;
    lXrndm += lSign * std::sqrt(lDelta) - b;
  }
  else if (mY[j] > 0.)
  {
    lXrndm += (lYrndm - mCDF[j]) / mY[j];
  }

  return lXrndm*mXUnit;
}

/**
 * @brief Sets data for the distribution.
 * 
 * This method sets the x and y data points for the distribution and assigns a name to it for the TGraph (if used).
 * 
 * @param pX Vector containing x-values of the data.
 * @param pY Vector containing y-values of the data.
 * @param pName Name for the distribution, used for debugging and TGraph name assignment.
 */
void DistributionSampler::setData(const std::vector<G4double> &pX, const std::vector<G4double> &pY, G4String pName)
{
  mX = pX;
  mY = pY;
  mDistName = pName; // TGraph needs unique name, also good for debugging
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
  mInterpolator = new TGraph(static_cast<int>(mX.size()), mX.data(), mY.data());
  mInterpolator->SetName(mDistName);
}


/**
 * @brief Calculates the slopes and cumulative function of a dataset for the inverse cumulative algorithm.
 *
 * This function computes the slopes and cumulative function values for the given dataset.
 * These results are often used in the inverse cumulative algorithm.
 */
void DistributionSampler::calculateSlopesAndCDF()
{
  G4int lNrPoints = static_cast<G4int>(mX.size());
  mSlopes.resize(lNrPoints, 0.0); // Initialize with zeros
  mCDF.resize(lNrPoints, 0.0);    // Initialize with zeros
  for (G4int j = 0; j < lNrPoints; j++)
  {
    // Compute slopes for all except the last point
    if (j < lNrPoints - 1)
    {
      mSlopes[j] = (mY[j + 1] - mY[j]) / (mX[j + 1] - mX[j]);
    }

    // Compute cumulative function
    if (j > 0)
    {
      mCDF[j] = mCDF[j - 1] + 0.5 * (mY[j] + mY[j - 1]) * (mX[j] - mX[j - 1]);
    }
  }
}

/**
 * @brief Interpolates distribution using TGraph (ROOT)
 * @param x-value
 * @return The interpolated y-value for the given x.
 */
G4double DistributionSampler::interpolate(G4double pX)
{
  if (!mInterpolator)
  {
    log_error("Error: mInterpolator not initialized!, make sure running makeInterpolator() before interpolate()");
    throw("Error: mInterpolator not initialized!");
    return 0.0;
  }
  return mInterpolator->Eval(pX/mXUnit) * mYUnit;
}


/**
 * @brief Sets units for the x and y data points of the distribution.
 * @param pX Unit for x-values.
 * @param pY Unit for y-values.
 */
void DistributionSampler::setUnits(G4double pX, G4double pY)
{
  mYUnit = pY;
  mXUnit = pX;
}

DistributionSampler::~DistributionSampler()
{
  if (mInterpolator)
  {
    delete mInterpolator;
    mInterpolator = nullptr;
  }
};