#include "OMSimSNTools.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4VTouchable.hh"
#include <stdlib.h>
#include "G4Navigator.hh"
#include "G4TouchableHistoryHandle.hh"
#include "OMSimCommandArgsTable.hh"

extern G4Navigator *aNavigator;

/**
 * @brief Retrieves file names for neutrino and antineutrino fluxes based on a specified supernova model.
 *
 * This function provides the corresponding file names for neutrino and antineutrino fluxes
 * depending on the supernova model identified by the input value.
 * Files are located in "supernova/models"
 *
 * @param value The integer identifier representing a specific supernova model.
 * @todo Handle error cases more robustly than just console output.
 * @return A pair containing the neutrino and antineutrino flux file names.
 */
std::pair<std::string, std::string> OMSimSNTools::getFileNames(int value)
{
  // TODO: make this more robust?
  std::string basePath = "../supernova/models/";
  switch (value)
  {
  case 0:
    return {basePath + "Flux_Nu_Heavy_ls220.cfg", basePath + "Flux_Nubar_Heavy_ls220.cfg"}; // 27 solar masses type II model
  case 1:
    return {basePath + "Flux_Nu_light_ls220.cfg", basePath + "Flux_Nubar_light_ls220.cfg"}; // 9.6 solar masses type II model
  case 2:
    return {basePath + "nu_DDT.cfg", basePath + "nubar_DDT.cfg"}; // type 1 SN
  case 3:
    return {basePath + "nu_GCD.cfg", basePath + "nubar_GCD.cfg"}; // type 1 SN
  case 4:
    return {basePath + "Flux_Nu_tailSN.cfg", basePath + "Flux_Nubar_tailSN.cfg"}; // long tailed type II
  default:
    // TODO handle error properly
    G4cout << "ERROR!! Choose a valid SN model" << G4endl;
    return {"", ""};
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
  aNavigator->LocateGlobalPointAndSetup(pPosition);
  G4TouchableHistoryHandle aTouchable = aNavigator->CreateTouchableHistoryHandle();
  G4int HistoryDepth = aTouchable->GetHistoryDepth();
  if (HistoryDepth > 0)
  {
    return true;
  }
  return false;
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
    G4double lMaxR = lRadius * sqrt(3);  // Using sqrt() here

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
 * @brief Performs linear interpolation between two points.
 *
 * This method calculates the y-value for a given x-value based on linear interpolation
 * between two known data points (x1, y1) and (x2, y2).
 *
 * @param x The x-value for which the y-value is desired.
 * @param x1 The x-value of the first data point.
 * @param x2 The x-value of the second data point.
 * @param y1 The y-value corresponding to x1.
 * @param y2 The y-value corresponding to x2.
 * @return The interpolated y-value for the given pX based on the two provided data points.
 */
G4double OMSimSNTools::linearInterpolation(G4double x, G4double x1, G4double x2, G4double y1, G4double y2)
{
  G4double lSlope = (y2 - y1) / (x2 - x1);
  return (lSlope * (x - x1) + y1);
}

/**
 * @brief Determines the energy value using the inverse cumulative algorithm based on a given distribution.
 *
 * Build the energy distribution Fe(E) given the input parameters, and uses the inverse CDF
 * algorithm to sample a value for the neutrino energy from Fe(E). The method prepares the
 * data to use 'energyDistributionVector'
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
  std::vector<G4double> lX;
  std::vector<G4double> lY;
  energyDistributionVector(pMeanEnergy, pAlpha, lNrPoints, lX, lY);
  return sampleValueFromDistribution(lX, lY, lNrPoints);
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
 * @brief Samples a value from a given distribution using the Inverse CDF method.
 *
 * This method preprocesses the provided data and leverages the inverse cumulative
 * distribution function (Inverse CDF) to obtain samples from a specified distribution.
 * It facilitates the generation of random values that adhere to the characteristics
 * of the given distribution.
 *
 * @param pXvals The pX values representing the domain of the distribution.
 * @param pYvals The corresponding y values representing the distribution's actual values.
 * @param pNrPoints The number of data points in the distribution.
 * @todo Reconsider the requirement for the 'pNrPoints' parameter.
 * @return A randomly chosen x-value that's sampled based on the distribution's profile.
 */
G4double OMSimSNTools::sampleValueFromDistribution(std::vector<G4double> pXvals, std::vector<G4double> pYvals, G4int pNrPoints)
{
  std::vector<G4double> lX;
  std::vector<G4double> lY;  // pY(pX)
  std::vector<G4double> lSlopes;  // slopes
  std::vector<G4double> lCDF; // cumulative of pY
  getSlopes(pXvals, pYvals, pNrPoints, lX, lY, lSlopes, lCDF);
  return inverseCDFmethod(lX, lY, lSlopes, lCDF);
}

/**
 * @brief Finds the nearest (or next largest) value in an array to a specified input.
 *
 * This method is utilized to identify neutrino parameters corresponding to a
 * randomly sampled time. These identified parameters are subsequently interpolated
 * to ensure that the model doesn't rely solely on fixed tabulated values from the input neutrino model.
 *
 * @param pTime The reference value for which a close match is sought in the array.
 * @param pTimeArray The array in which to search for the closest match.
 * @todo Consider renaming 'time' to a more generic term.
 * @return The index in the array that holds the value closest (or next largest) to the specified input.
 */
G4int OMSimSNTools::findTime(G4double pTime, std::vector<G4double> pTimeArray)
{
  for (unsigned int j = 0; j < pTimeArray.size(); j++)
  {
    if (pTime <= pTimeArray.at(j))
    {
      return j;
    };
  };
  G4cerr << "FATAL ERROR -> Not posible to find time of spectrum!!!" << G4endl;
  return 0;
}

/**
 * @brief Samples values from a given distribution using the Inverse CDF algorithm.
 *
 * This method employs the inverse cumulative distribution function (Inverse CDF) technique
 * to derive samples from a specified distribution. It's useful for generating random
 * values in accordance with the underlying distribution shape.
 *
 * @param pX The pX values representing the distribution's domain.
 * @param pY The corresponding y values, or f(pX), representing the distribution's values.
 * @param pSlopes The slopes of the f(pX) curve, usually derived from the getSlopes method.
 * @param pCDF The cumulative values of f(pX), usually derived from the getSlopes method.
 * @return A randomly selected pX-value sampled in line with the provided distribution.
 */
G4double OMSimSNTools::inverseCDFmethod(const std::vector<G4double>& pX, const std::vector<G4double>& pY, const std::vector<G4double>& pSlopes, const std::vector<G4double>& pCDF)
{
    // Total number of points
    G4int pNrPoints = static_cast<G4int>(pX.size());

    // Randomly choose a y-value from the CDF's range
    G4double lYrndm = G4UniformRand() * pCDF[pNrPoints - 1];

    // Find the bin corresponding to the y-value
    G4int j = pNrPoints - 2;
    while (pCDF[j] > lYrndm && j > 0)
        j--;

    // Convert random y-value to a random x-value
    // The conversion is based on the fact that the CDF is represented as a second-order polynomial
    G4double lXrndm = pX[j];
    G4double lSlope = pSlopes[j];
    
    if (lSlope != 0.)
    {
        G4double b = pY[j] / lSlope;
        G4double c = 2 * (lYrndm - pCDF[j]) / lSlope;
        G4double lDelta = b * b + c;
        G4int lSign = (lSlope < 0.) ? -1 : 1;
        lXrndm += lSign * std::sqrt(lDelta) - b;
    }
    else if (pY[j] > 0.)
    {
        lXrndm += (lYrndm - pCDF[j]) / pY[j];
    }

    return lXrndm;
}


/**
 * @brief Prepares data for the inverse cumulative algorithm.
 *
 * This helper function calculates the slopes and cumulative function of a given dataset, facilitating
 * its use in the inverse cumulative algorithm. The function also creates a copy of the input arrays
 * to avoid overwriting the original data.
 *
 * @param pXvals   Input pX-values of the dataset.
 * @param pYvals   Input y-values of the dataset.
 * @param pNrPoints Number of data points in the dataset.
 * @param pX       Output vector to store the copied pX-values.
 * @param pY       Output vector to store the copied y-values.
 * @param pSlopes       Output vector to store the computed slopes.
 * @param pCDF      Output vector to store the computed cumulative function.
 * @todo check whether the copy can just be eliminated
 */
void getSlopes(std::vector<G4double> pXvals, std::vector<G4double> pYvals, G4int pNrPoints, std::vector<G4double> &pX, std::vector<G4double> &pY, std::vector<G4double> &pSlopes, std::vector<G4double> &pCDF)
{
    pX = pXvals;
    pY = pYvals;

    pSlopes.resize(pNrPoints, 0.0);  // Initialize with zeros
    pCDF.resize(pNrPoints, 0.0);     // Initialize with zeros

    for (G4int j = 0; j < pNrPoints - 1; j++)
    {
        // Compute slopes
        pSlopes[j] = (pY[j + 1] - pY[j]) / (pX[j + 1] - pX[j]);

        // Compute cumulative function
        if (j > 0)
        {
            pCDF[j] = pCDF[j - 1] + 0.5 * (pY[j] + pY[j - 1]) * (pX[j] - pX[j - 1]);
        }
    }
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
G4double numberOfTargets(G4int pNrTargetPerMolecule)
{
  G4double lDensity = 921.6 * kg / m3;    // Density of ice at -50 celsius degrees
  G4double lMolarMass = 18.01528e-3 * kg; // kg per mol
  G4double lAvogadroNumber = 6.022140857e23;
  G4double lNrMolecules = lDensity / lMolarMass * lAvogadroNumber; // molecules/m^3 of ice
  G4double lNrTargets = lNrMolecules * pNrTargetPerMolecule;
  return lNrTargets;
}

/**
 * @brief Generates an energy distribution array based on a predefined formula.
 *
 * The energy distribution is determined by the formula: \( F(e) = e^{\alpha} \times exp(-(\alpha+1) \times e/pMeanEnergy) \).
 * The method generates energy values (`pX`) within a set range [0, 80] MeV, divided into `pNrPoints` intervals.
 * The corresponding function values (`f`) are then computed using the provided energy distribution formula.
 *
 * The underlying formulas for Fe(E) is inspired by:
 * Irene Tamborra et al., "High-resolution supernova neutrino spectra represented by a simple fit," PHYSICAL REVIEW D 86, 125031 (2012).
 * https://arxiv.org/abs/1211.3920
 *
 * @param pMeanEnergy Mean energy value used in the distribution function.
 * @param pAlpha Exponent value used in the distribution function.
 * @param pNrPoints Total number of points for the distribution.
 * @param[out] pX Vector to store the computed energy values.
 * @param[out] pY Vector to store the computed function values based on the energy distribution.
 */
void energyDistributionVector(G4double pMeanEnergy, G4double pAlpha, G4int pNrPoints, std::vector<G4double> &pX, std::vector<G4double> &pY)
{
    const G4double min = 0.;
    const G4double max = 80.;
    const G4double delta = (max - min) / G4double(pNrPoints - 1);
    
    pX.resize(pNrPoints);
    pY.resize(pNrPoints);

    for (G4int i = 0; i < pNrPoints; i++)
    {
        pX[i] = (min + i * delta) * MeV; // Energy
        pY[i] = pow(pX[i], pAlpha) * exp(-(pAlpha + 1.) * pX[i] / pMeanEnergy); // F(e), energy dist. function
    }
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
G4double OMSimSNTools::weight(G4double pSigma, G4double pNrTargets)
{
  return pSigma * pNrTargets * (2 * OMSimCommandArgsTable::getInstance().get<G4double>("wheight") * m);
}
