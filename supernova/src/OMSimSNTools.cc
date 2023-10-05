#include "OMSimSNTools.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"
#include "G4VTouchable.hh"
#include <stdlib.h>
#include "G4Navigator.hh"
#include "G4TouchableHistoryHandle.hh"
#include "OMSimCommandArgsTable.hh"


extern G4Navigator* aNavigator;


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
std::pair<std::string, std::string> OMSimSNTools::getFileNames(int value) {
    //TODO: make this more robust?
    std::string basePath = "../supernova/models/";
    switch(value) {
        case 0:
            return {basePath + "Flux_Nu_Heavy_ls220.cfg", basePath + "Flux_Nubar_Heavy_ls220.cfg"}; //27 solar masses type II model
        case 1:
            return {basePath + "Flux_Nu_light_ls220.cfg", basePath + "Flux_Nubar_light_ls220.cfg"}; //9.6 solar masses type II model
        case 2:
            return {basePath + "nu_DDT.cfg", basePath + "nubar_DDT.cfg"}; //type 1 SN
        case 3:
            return {basePath + "nu_GCD.cfg", basePath + "nubar_GCD.cfg"}; //type 1 SN
        case 4:
            return {basePath + "Flux_Nu_tailSN.cfg", basePath + "Flux_Nubar_tailSN.cfg"}; //long tailed type II
        default:
            //TODO handle error properly
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
	 * @param position The 3D position vector to be checked against module boundaries.
	 * @warning This relies on the generation module being a single volume (generally, the ice) and the mother volume of everything else
	 * If the ice is modified to be composed of different subvolumes (like, for example, if the bubble column is added)
	 * this method would need to be modify!
	 * 
	 * @return True if the position is inside a module; otherwise, false.
	 */
bool OMSimSNTools::CheckVolumeFormDOMs(G4ThreeVector position){
    aNavigator->LocateGlobalPointAndSetup(position);
    G4TouchableHistoryHandle aTouchable = aNavigator->CreateTouchableHistoryHandle();
    G4int HistoryDepth = aTouchable->GetHistoryDepth();
    if (HistoryDepth > 0) {return true;}
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
G4ThreeVector OMSimSNTools::RandomPosition() {
  //now we assume that the world is a cylinder!
  double gHeight = OMSimCommandArgsTable::getInstance().get<G4double>("wheight")*m;
  double gRadius = OMSimCommandArgsTable::getInstance().get<G4double>("wradius")*m;

  //maximum lenght of generation cylinder "Rmax"
  //Note that this is the distance from the center of the cylinder to the corner of the circumscribed rectangle around the cylinder
  G4double Rmax = pow(3,1./2.)*gRadius; 
  G4double Rmax2 = gRadius; //radius of generation cylinder
  
  G4double posz;
  G4double posx;
  G4double posy;
  G4ThreeVector Position;
  G4double R3; //distance from center
  G4double R2; //distance wrt z axis
  R3 = 0*m;
  R2 = 0*m;
  G4bool boolparameter = true;
  while ( ( boolparameter==true ) || (R3 >= Rmax) || (R2 >= Rmax2)) {
    G4double posornegX = 1;
    if (G4UniformRand()<0.5) { posornegX = -1;}
            G4double posornegY = 1;
    if (G4UniformRand()<0.5) { posornegY = -1;}
            G4double posornegZ = 1;
    if (G4UniformRand()<0.5) { posornegZ = -1;}
    posz = posornegZ*(G4UniformRand()*gHeight);
    posx = posornegX*(G4UniformRand()*gRadius);
    posy = posornegY*(G4UniformRand()*gRadius);
    Position = G4ThreeVector(posx,posy,posz);
    R3 = pow(pow(Position[0],2)+pow(Position[1],2)+pow(Position[2],2),1./2.);
    R2 = pow(pow(Position[0],2)+pow(Position[1],2),1./2.);
    boolparameter = CheckVolumeFormDOMs(Position);
  } 
  return Position;
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
	 * @return The interpolated y-value for the given x based on the two provided data points.
	 */
G4double OMSimSNTools::linealinterpolation(G4double x, G4double x1, G4double x2, G4double y1, G4double y2) {
    G4double slope = (y2 - y1) / (x2 - x1);
    G4double result = (slope * (x - x1) + y1);
    return result;
}

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
G4double OMSimSNTools::EnergyDistribution(G4double Emean, G4double Emean2, G4double& alpha)
{   
  G4int nPoints1 = 500;
  if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false) {
	  alpha = GetAlpha(Emean, Emean2);
  } 
  std::vector<G4double> x1;
  std::vector<G4double> f1;
  MakeEnergyDistribution(Emean, alpha, nPoints1, x1 , f1);
	G4double choosenenergy = InverseCumul(x1, f1, nPoints1);
	return choosenenergy;
}
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
G4double OMSimSNTools::GetAlpha(G4double Emean,G4double Emean2)
{
	G4double alpha = (2*pow(Emean,2)-Emean2)/(Emean2-pow(Emean,2));
	return alpha;
}

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
G4double OMSimSNTools::InverseCumul(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints)
{
  std::vector<G4double>  x_g;
  std::vector<G4double>  f_g;           //f(x)
  std::vector<G4double>  a_g;           //slopes
  std::vector<G4double>  Fc_g;          //cumulative of f
  GetSlopes(xvals, yvals, nPoints, x_g, f_g, a_g, Fc_g);
  G4double x_rndm =  InverseCumulAlgorithm(x_g, f_g, a_g, Fc_g, nPoints);
  return x_rndm;
}

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
G4int OMSimSNTools::findtime(G4double time, std::vector<G4double> timearray)
{
  for (unsigned int j=0; j<timearray.size(); j++) {
    if (time <= timearray.at(j)) {
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
	 * @param x The x values representing the distribution's domain.
	 * @param f The corresponding y values, or f(x), representing the distribution's values.
	 * @param Fc The slopes of the f(x) curve, usually derived from the GetSlopes method.
	 * @param Fc The cumulative values of f(x), usually derived from the GetSlopes method.
	 * @param nPoints The length or number of data points in the distribution.
	 * @todo Re-evaluate the necessity of the 'nPoints' parameter.
	 * @return A randomly selected x-value sampled in line with the provided distribution.
	 */
G4double OMSimSNTools::InverseCumulAlgorithm(std::vector<G4double>  x, std::vector<G4double>  f, std::vector<G4double>  a, std::vector<G4double>  Fc, G4int  nPoints)
{
  //choose y randomly
  G4double y_rndm = G4UniformRand()*Fc[nPoints-1];
  //find bin
  G4int j = nPoints-2;
  while ((Fc[j] > y_rndm) && (j > 0)) j--;
  //y_rndm --> x_rndm :  Fc(x) is second order polynomial
  G4double x_rndm = x[j];
  G4double aa = a[j];
  if (aa != 0.) {
    G4double b = f[j]/aa, c = 2*(y_rndm - Fc[j])/aa;
    G4double delta = b*b + c;
    G4int sign = 1; if (aa < 0.) sign = -1;
    x_rndm += sign*std::sqrt(delta) - b;    
  } else if (f[j] > 0.) {
    x_rndm += (y_rndm - Fc[j])/f[j];
  };
  return x_rndm;
}


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
void GetSlopes(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints, std::vector<G4double>&  x, std::vector<G4double>&  f, std::vector<G4double>&  a, std::vector<G4double>&  Fc)
{
  // create a copy of the array. Really not necessary...
  x.resize(nPoints); f.resize(nPoints);
  for (G4int j=0; j<nPoints; j++) {
    x[j] = xvals.at(j); f[j] = yvals.at(j);
  };
  //compute slopes
  //
  a.resize(nPoints);
  for (G4int j=0; j<nPoints-1; j++) { 
    a[j] = (f[j+1] - f[j])/(x[j+1] - x[j]);
  };
  //compute cumulative function
  //
  Fc.resize(nPoints);  
  Fc[0] = 0.;
  for (G4int j=1; j<nPoints; j++) {
    Fc[j] = Fc[j-1] + 0.5*(f[j] + f[j-1])*(x[j] - x[j-1]);
  };     
}

/**
 * @brief Calculates the number of target particles in ice per cubic meter.
 * 
 * This function determines the number of target particles, given the number of targets per molecule, 
 * assuming the ice is pure H2O. The density and molar mass values are based on ice at -50°C.
 *
 * @param targetPerMolecule Number of target particles per H2O molecule.
 * @return Total number of target particles per cubic meter in ice.
 */
G4double NumberOfTargets(G4int targetPerMolecule) {
  G4double Density = 921.6*kg/m3; //Density of ice at -50 celsius degrees
  G4double MolarMass = 18.01528e-3*kg; //kg per mol
  G4double Na = 6.022140857e23;
  G4double Nm = Density/MolarMass*Na;//molecules/m^3 of ice
  G4double Nt = Nm*targetPerMolecule;
  return Nt;
}

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
void MakeEnergyDistribution(G4double Emean, G4double alpha, G4int nPoints, std::vector<G4double>& x, std::vector<G4double>& f)
{
  G4double min = 0.; G4double max = 80.; 
  G4double delta = (max-min)/G4double(nPoints-1);
  x.resize(nPoints); f.resize(nPoints);

  for (G4int i=0; i<nPoints; i++) {
    x[i] = (min + i*delta)*MeV; //Energy
  }

  for (G4int j=0; j<nPoints; j++) {
  f[j] = pow(x[j],alpha)*exp(-(alpha+1.)*x[j]/Emean); // F(e), energy dist. function
  }
}

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
G4double OMSimSNTools::WeighMe(G4double sigma, G4double NTargets) {
  //Assuming a cylinder!
  double weigh = sigma*NTargets*(2*OMSimCommandArgsTable::getInstance().get<G4double>("wheight")*m);
  return weigh;
}


