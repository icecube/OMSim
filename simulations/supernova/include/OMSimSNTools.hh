/**
 * @file
 * @brief Provides the definition for the `OMSimSNTools` class, centralizing utility methods for Supernova (SN) neutrino interactions.
 * 
 * This class serves as a central utility to house common methods needed in the generation 
 * of SN neutrino interactions.
 * @ingroup sngroup
 */

#pragma once

#include <globals.hh>
#include <vector>
#include <G4ThreeVector.hh>
#include <TGraph.h>


/**
 * @class DistributionSampler
 * @brief Utility class for sampling from a given distribution using the inverse cumulative function and interpolating with TGraph.
 * @ingroup sngroup
 */
class DistributionSampler
{	
	public:
		DistributionSampler(){};
		~DistributionSampler();
		G4double sampleFromDistribution();
		void setData(const std::vector<G4double>& pX, const std::vector<G4double>& pY, G4String pName);
		void makeInterpolator();
		void setUnits(G4double pX, G4double pY);
		G4double interpolate(G4double pX);

	private:
		void calculateSlopesAndCDF();
		G4double m_XUnit;
		G4double m_YUnit;
		std::vector<G4double> m_X;
		std::vector<G4double> m_Y;
		std::vector<G4double> m_slopes;
		std::vector<G4double> m_CDF;
		G4String m_distName;
		TGraph *m_interpolator = nullptr;
};

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

	G4ThreeVector randomPosition();
	std::pair<std::string, std::string> getFileNames(int value);
	G4double sampleEnergy(G4double Emean, G4double Emean2, G4double& alpha);
	G4double getAlpha(G4double Emean,G4double Emean2);
	G4double calculateWeight(G4double sigma, G4double NTargets);
	G4double numberOfTargets(G4int targetPerMolecule);
private:     
	std::vector <G4double> m_DOMpos;
	G4double m_rMin;
	bool checkVolumeForOMs(G4ThreeVector position);
};

