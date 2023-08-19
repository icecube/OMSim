/**
 * @file mdomSNTools.hh
 * @brief Provides the definition for the `mdomSNTools` class, centralizing utility methods for Supernova (SN) neutrino interactions.
 * 
 * This class serves as a central utility to house common methods needed in the generation 
 * of SN neutrino interactions.
 *
 * @author Cristian Jesus Lozano Mariscal
 * @version 10.1
 * @ingroup SN
 */

#ifndef mdomSNTools_h
#define mdomSNTools_h 1

#include "globals.hh"
#include <vector>
#include "G4ThreeVector.hh"

/**
 * @class mdomSNTools
 * @brief Provides utility methods for generating ENES and IBD interactions.
 *
 * This class serves as a repository for common functionalities associated with the generation 
 * of ENES and IBD interactions within the Supernova (SN) context.
 * 
 * @ingroup SN
 */
class mdomSNTools
{
public:
	/**
   	 * @brief Default constructor.
   	 */
  	mdomSNTools();   

  	/**
   	 * @brief Default destructor.
   	 */ 
  	~mdomSNTools();

	/**
   	 * @brief Randomly determines a vertex position within the generation volume.
	 *
	 * The vertex is considered valid only if it is located within the ice, ensuring 
	 * it is not situated inside the module or other predefined volumes.
	 * 
	 * @return A valid vertex position within the ice.
   	 */
	G4ThreeVector RandomPosition();

	/**
	 * @brief Samples a value from a given distribution using the Inverse CDF method.
	 *
	 * This method preprocesses the provided data and leverages the inverse cumulative 
	 * distribution function (Inverse CDF) to obtain samples from a specified distribution. 
	 * It facilitates the generation of random values that adhere to the characteristics 
	 * of the given distribution.
	 *
	 * @param xvals The x values representing the domain of the distribution.
	 * @param yvals The corresponding y values representing the distribution's actual values.
	 * @param nPoints The number of data points in the distribution.
	 * @todo Reconsider the requirement for the 'nPoints' parameter.
	 * @warning Assumes that the generation volume is a cylinder!
	 * @return A randomly chosen x-value that's sampled based on the distribution's profile.
	 */
	G4double InverseCumul(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints);

	/**
	 * @brief Samples values from a given distribution using the Inverse CDF algorithm.
	 *
	 * This method employs the inverse cumulative distribution function (Inverse CDF) technique 
	 * to derive samples from a specified distribution. It's useful for generating random 
	 * values in accordance with the underlying distribution shape.
	 *
	 * @param x The x values representing the distribution's domain.
	 * @param f The corresponding y values, or f(x), representing the distribution's values.
	 * @param Fc The slopes of the f(x) curve, usually derived from the GetSlopes method.
	 * @param Fc The cumulative values of f(x), usually derived from the GetSlopes method.
	 * @param nPoints The length or number of data points in the distribution.
	 * @todo Re-evaluate the necessity of the 'nPoints' parameter.
	 * @return A randomly selected x-value sampled in line with the provided distribution.
	 */
	G4double InverseCumulAlgorithm(std::vector<G4double>  x, std::vector<G4double>  f, std::vector<G4double>  a, std::vector<G4double>  Fc, G4int  nPoints);

	/**
	 * @brief Finds the nearest (or next largest) value in an array to a specified input.
	 *
	 * This method is utilized to identify neutrino parameters corresponding to a 
	 * randomly sampled time. These identified parameters are subsequently interpolated 
	 * to ensure that the model doesn't rely solely on fixed tabulated values from the input neutrino model.
	 *
	 * @param time The reference value for which a close match is sought in the array.
	 * @param timearray The array in which to search for the closest match.
	 * @todo Consider renaming 'time' to a more generic term.
	 * @return The index in the array that holds the value closest (or next largest) to the specified input.
	 */
	G4int findtime(G4double time, std::vector<G4double> timearray);

	/**
	 * @brief Determines the energy value using the inverse cumulative algorithm based on a given distribution.
	 *
	 * Build the energy distribution Fe(E) given the input parameters, and uses the inverse CDF
	 * algorithm to sample a value for the neutrino energy from Fe(E). The method prepares the
	 * data to use 'MakeEnergyDistribution'
	 * 
	 * @param Emean The primary mean energy value from the supernova model.
	 * @param Emean2 The primary squared mean energy value from the supernova model.
	 * @param alpha Pinching parameter of the energy spectrum.
	 * @return The sampled energy value based on the given model.
	 */
	G4double EnergyDistribution(G4double Emean, G4double Emean2, G4double& alpha);

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
	 * @return The interpolated y-value for the given x based on the two provided data points.
	 */
	G4double mdomSNTools::linealinterpolation(G4double x, G4double x1, G4double x2, G4double y1, G4double y2);

	/**
	 * @brief Calculates pinching parameter of energy spectrum
	 *
	 * Given the mean energy and squared mean energy of the neutrino event, calculates the pinching parameter following
	 * equation 3 in
	 * Irene Tamborra et al., "High-resolution supernova neutrino spectra represented by a simple fit," PHYSICAL REVIEW D 86, 125031 (2012).
	 * https://arxiv.org/abs/1211.3920
	 * 
	 * @param Emean The primary mean energy value from the supernova model.
	 * @param Emean2 The primary squared mean energy value from the supernova model.
	 * @return alpha Pinching parameter of the energy spectrum.
	 */
	G4double GetAlpha(G4double Emean,G4double Emean2);

	/**
	 * @brief Calculates the interaction weight based on the cross section and number of targets.
	 *
	 * This method computes the interaction weight of neutrinos in the ice using the formula:
	 * `weight = sigma * Nt * r`, 
	 * where:
	 * - `sigma` is the cross section,
	 * - `Nt` is the number of targets in the ice (considered as electrons),
	 * - `r` is the distance the neutrino travels in the ice.
	 * 
	 * @param sigma The cross section value.
	 * @param NTargets The number of targets present in the ice for the corresponding interaction.
	 * @warning It's crucial to note that this formulation is specifically valid for SN neutrinos 
	 * approaching from the Z axis in a world defined as a cylinder. For any other geometry or conditions,
	 * a different approach or adjustments might be required.
	 * @return The computed interaction weight for the neutrinos based on the provided parameters.
	 */
	G4double WeighMe(G4double sigma, G4double NTargets);

private:     
	std::vector <G4double> mdompos;
	G4double Rmin;

	/**
	 * @brief Checks if a given position is within any of the modules.
	 * 
	 * This method determines whether the provided position lies in the generation medium (ice) It evaluates the position 
	 * in the context of the global environment and examines the history depth to make this determination.
	 * 
	 * @param position The 3D position vector to be checked against module boundaries.
	 * @warning This relies on the generation module being a single volume (generally, the ice) and the mother volume of everything else
	 * If the ice is modified to be composed of different subvolumes (like, for example, if the bubble column is added)
	 * this method would need to be modify!
	 * 
	 * @return True if the position is inside a module; otherwise, false.
	 */
	bool CheckVolumeFormDOMs(G4ThreeVector position);
};

/**
 * @brief Prepares data for the inverse cumulative algorithm.
 * 
 * This helper function calculates the slopes and cumulative function of a given dataset, facilitating 
 * its use in the inverse cumulative algorithm. The function also creates a copy of the input arrays 
 * to avoid overwriting the original data.
 *
 * @param xvals   Input x-values of the dataset.
 * @param yvals   Input y-values of the dataset.
 * @param nPoints Number of data points in the dataset.
 * @param x       Output vector to store the copied x-values.
 * @param f       Output vector to store the copied y-values.
 * @param a       Output vector to store the computed slopes.
 * @param Fc      Output vector to store the computed cumulative function.
 * @todo check whether the copy can just be eliminated
 */
void GetSlopes(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints, std::vector<G4double>&  x, std::vector<G4double>&  f, std::vector<G4double>&  a, std::vector<G4double>&  Fc);

/**
 * @brief Calculates the number of target particles in ice per cubic meter.
 * 
 * This function determines the number of target particles, given the number of targets per molecule, 
 * assuming the ice is pure H2O. The density and molar mass values are based on ice at -50°C.
 *
 * @param targetPerMolecule Number of target particles per H2O molecule.
 * @return Total number of target particles per cubic meter in ice.
 */
G4double NumberOfTargets(G4int targetPerMolecule);

/**
 * @brief Generates an energy distribution array based on a predefined formula.
 * 
 * The energy distribution is determined by the formula: \( F(e) = e^{\alpha} \times exp(-(\alpha+1) \times e/Emean) \). 
 * The method generates energy values (`x`) within a set range [0, 80] MeV, divided into `nPoints` intervals. 
 * The corresponding function values (`f`) are then computed using the provided energy distribution formula.
 *
 * The underlying formulas for Fe(E) is inspired by:
 * Irene Tamborra et al., "High-resolution supernova neutrino spectra represented by a simple fit," PHYSICAL REVIEW D 86, 125031 (2012).
 * https://arxiv.org/abs/1211.3920
 * 
 * @param Emean Mean energy value used in the distribution function.
 * @param alpha Exponent value used in the distribution function.
 * @param nPoints Total number of points for the distribution.
 * @param[out] x Vector to store the computed energy values.
 * @param[out] f Vector to store the computed function values based on the energy distribution.
 */
void MakeEnergyDistribution(G4double Emean, G4double alpha, G4int nPoints, std::vector<G4double>& x, std::vector<G4double>& f);
#endif
