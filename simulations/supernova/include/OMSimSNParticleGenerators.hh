/**
 * @file 
 * @brief This file contains the classes in charge of particle generation 
 * @ingroup sngroup
 */

#pragma once

#include "G4VUserPrimaryGeneratorAction.hh"
#include "OMSimPrimaryGeneratorMessenger.hh"
#include "OMSimSNTools.hh"

#include <globals.hh>
#include <vector>
#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>

class G4ParticleGun;
class G4Event;

/**
 * @class 
 * @brief Base class for SN particle generators
 * @ingroup sngroup
 */
class SNBaseParticleGenerator : public G4VUserPrimaryGeneratorAction
{
public:

	SNBaseParticleGenerator(G4ParticleGun*);    
	~SNBaseParticleGenerator(){};

	void GeneratePrimaries(G4Event* anEvent);

protected:    
    G4ParticleGun* m_particleGun;
    OMSimSNTools m_SNToolBox;

    void initialiseDistribution(int column, int targets);
    G4double calculateNeutrinoEnergy();
    G4ThreeVector calculateMomentumDirection(G4double pNeutrinoEnergy);
    G4double calculateWeight(G4double pNuEnergy);

    const G4double m_Gf = 1.166e-5 * 1e-6 / (MeV * MeV);
    const G4double m_Consg = 1.26;
    const G4double m_deltaMass = neutron_mass_c2 - proton_mass_c2;
    const G4double m_massSquaredDifference = (pow(m_deltaMass, 2) - pow(electron_mass_c2, 2)) / 2.;

    G4double m_NrTargets;      
    DistributionSampler m_timeDistribution;
	DistributionSampler m_meanEnergyDistribution;
    DistributionSampler m_meanEnergySquaredDistribution;

    virtual void initialiseParticle() = 0;
	virtual DistributionSampler angularDistribution(G4double Enu) = 0;
    virtual G4double calculateSecondaryParticleEnergy(G4double nu_energy, G4double costheta) = 0;
    virtual G4double totalCrossSection(G4double energy) = 0;
};



/**
 * @class OMSimENES
 * @brief Class in charge of generating electrons from Electron-electron_Neutrino Elastic Scattering (ENES)
 * nu_e+e -> nu_e+e of supernova antineutrino interactions
 *
 * Supported by OMSimSNTools, this class generates the corresponding electrons after the ENES
 * and weights the interactions corresponding to the cross section and the generated volume
 *
 * @ingroup sngroup
 */
class OMSimENES : public SNBaseParticleGenerator
{
public:

	OMSimENES(G4ParticleGun*);    
	~OMSimENES(){};
	  
private:     
    void initialiseParticle();
	DistributionSampler angularDistribution(G4double Enu);
    G4double calculateSecondaryParticleEnergy(G4double nu_energy, G4double costheta);
    G4double totalCrossSection(G4double energy);
};



/**
 * @class OMSimIBD
 * @brief Class in charge of generating positrons from IBD of supernova antineutrino interactions
 *
 * Supported by OMSimSNTools, this class generates the corresponding positrons after the inverse beta decay
 * and weights the interactions corresponding to the cross section and the generated volume
 *
 * @ingroup sngroup
 */
class OMSimIBD : public SNBaseParticleGenerator
{
public:

	OMSimIBD(G4ParticleGun*);    
	~OMSimIBD(){};
	  
private:     
    void initialiseParticle();
	DistributionSampler angularDistribution(G4double Enu);
    G4double calculateSecondaryParticleEnergy(G4double nu_energy, G4double costheta);
    G4double totalCrossSection(G4double energy);
};


#endif


