/**
 * @file
 * @brief Provides the definition for the `OMSimSNTools` class, centralizing utility methods for Supernova (SN) neutrino interactions.
 * 
 * This class serves as a central utility to house common methods needed in the generation 
 * of SN neutrino interactions.
 *
 * @author Cristian Jesus Lozano Mariscal
 * @ingroup sngroup
 */

#ifndef OMSimSNTools_h
#define OMSimSNTools_h 1

#include "globals.hh"
#include <vector>
#include "G4ThreeVector.hh"

/**
 * @class OMSimSNTools
 * @brief Provides utility methods for generating ENES and IBD interactions.
 *
 * This class serves as a repository for common functionalities associated with the generation 
 * of ENES and IBD interactions within the Supernova (SN) context.
 * 
 * @ingroup sngroup
 */
class OMSimSNTools
{
public:

  	OMSimSNTools(){};   
  	~OMSimSNTools(){};
	
	G4ThreeVector RandomPosition();
	std::pair<std::string, std::string> getFileNames(int value);
	G4double InverseCumul(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints);
	G4double InverseCumulAlgorithm(std::vector<G4double>  x, std::vector<G4double>  f, std::vector<G4double>  a, std::vector<G4double>  Fc, G4int  nPoints);
	G4int findtime(G4double time, std::vector<G4double> timearray);
	G4double EnergyDistribution(G4double Emean, G4double Emean2, G4double& alpha);
	G4double linealinterpolation(G4double x, G4double x1, G4double x2, G4double y1, G4double y2);
	G4double GetAlpha(G4double Emean,G4double Emean2);
	G4double WeighMe(G4double sigma, G4double NTargets);

private:     
	std::vector <G4double> mdompos;
	G4double Rmin;
	bool CheckVolumeFormDOMs(G4ThreeVector position);
};


void GetSlopes(std::vector<G4double>  xvals, std::vector<G4double>  yvals, G4int nPoints, std::vector<G4double>&  x, std::vector<G4double>&  f, std::vector<G4double>&  a, std::vector<G4double>&  Fc);
G4double NumberOfTargets(G4int targetPerMolecule);
void MakeEnergyDistribution(G4double Emean, G4double alpha, G4int nPoints, std::vector<G4double>& x, std::vector<G4double>& f);

#endif
