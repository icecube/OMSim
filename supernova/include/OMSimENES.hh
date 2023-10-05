/**
 * @file OMSimENES.hh
 * @brief This file contains the class in charge of generating electrons from Electron-electron_Neutrino Elastic Scattering (ENES)
 *  of supernova antineutrino interactions
 *
 * @author Cristian Jesus Lozano Mariscal
 * @version 10.1
 * @ingroup SN
 */


#ifndef OMSimENES_h
#define OMSimENES_h 1
 
#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include <vector>
#include "OMSimSNTools.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
class G4ParticleGun;
class G4Event;

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
class OMSimENES : public G4VUserPrimaryGeneratorAction
{
public:

	OMSimENES(G4ParticleGun*);    
	~OMSimENES(){};

	void GeneratePrimaries(G4Event* anEvent);
     
    G4double NTargets;
    G4double me;
    G4double mGf = 1.166e-5*1e-6/(MeV*MeV);
    G4int                  nPoints_lum;     //tabulated function
    std::vector<G4double>  x_lum;
    std::vector<G4double>  f_lum;           //f(x)
    std::vector<G4double>  a_lum;           //slopes
    std::vector<G4double>  Fc_lum;          //cumulative of f
    G4int                  nPoints1;     //tabulated function
    std::vector<G4double>  fixFe_X;
    std::vector<G4double>  fixFe_Y;           //f(x)
    std::vector<G4double>  fixFe_a;           //slopes
    std::vector<G4double>  fixFe_Fc;          //cumulative of f
    G4int fixE_nPoints;
    G4int                  angdist_nPoints;    
    std::vector<G4double>  angdist_x;
    std::vector<G4double>  angdist_y;           

    //here model data will be stored
	std::vector<double> mNu_time;
	std::vector<double> mNu_luminosity;
	std::vector<double> mNu_meanenergy;
	std::vector<double> mNu_meanenergysquare;
	  
private:     
    G4ParticleGun* ParticleGun;


	void AngulasDistribution(G4double Enu);
    G4double ElectronEnergy(G4double nu_energy, G4double costheta);
    G4double TotalCrossSection(G4double energy);

    OMSimSNTools mSNToolBox;
    G4double mFixedenergy;
    G4double mFixedenergy2;
    G4double mAlpha;
};

#endif
