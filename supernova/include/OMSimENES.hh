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
 * @ingroup SN
 */
class OMSimENES : public G4VUserPrimaryGeneratorAction
{
public:
    /**
     * @brief Constructor of OMSimENES class
     *
     * The class constructor initializes various properties of the model based on given input 
     * parameters or data files.
     *
     * @param gun Pointer to the particle gun object used for simulation.
     */
	OMSimENES(G4ParticleGun*);    

    /**
     * @brief Destructor of OMSimIBD class
     */
	~OMSimENES();

    /**
     * @brief Generates primary events for the simulation.
     * 
     * This function sets the initial properties of the electronic neutrinos such as position,
     * and energy for the primary event based on given input parameters or data files. The energy is determined from a tabulated 
     * distribution or from a fixed mean energy, and the neutrino's incoming direction is assumed to be (0,0,-1). The function also 
     * computes the angle distribution and the electron's energy derived from the neutrino energy and its direction. Finally,
     * the function computes the total cross-section and sets weights accordingly, passing the information to the analysis manager.
     *
     * @param anEvent Pointer to the G4Event object where primaries will be generated.
     * @todo Consider breaking down this function into smaller, more specialized sub-functions for clarity and modularity.
     * @todo Ensure that any new parameters or member variables used are appropriately documented.
     */
	void GeneratePrimaries(G4Event* anEvent);
     
    G4double NTargets;
    G4double me;
    G4double Gf;
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

    /**
     * @brief Computes the angular distribution for electron scattering based on incident neutrino energy.
     * 
     * This function calculates the angular distribution for electrons scattered by neutrinos. The foundation for this 
     * computation is taken from the reference:
     * "Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press, Chapter 5, eq. 5.29".
     * 
     * @param Enu The energy of the incident neutrino.
     */
	void DistFunction(G4double Enu);

    /**
     * @brief Computes the electron energy from elastic scattering based on incident neutrino energy and scatter angle.
     * 
     * The computation utilizes the formula for the energy of the electron as a function of incident neutrino energy 
     * and the scatter angle, based on the formula provided in the reference:
     * "Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press, Chapter 5, eq. 5.27".
     * 
     * @param nu_energy The energy of the incident neutrino.
     * @param costheta The cosine of the scattering angle.
     * @return Calculated energy value of the electron post scattering.
     */
    G4double ElectronEnergy(G4double nu_energy, G4double costheta);

    /**
     * @brief Computes the total cross-section for ENES based on incident neutrino energy.
     * 
     * This function determines the total cross-section for the scattering of neutrinos with electrons given a specific 
     * incident neutrino energy. The computational foundation is derived from:
     * "M. Buchkremer, Electroweak Interactions: Neutral currents in neutrino-lepton elastic scattering experiments, 
     * Université Catholique de Louvain /CP3, 2011."
     * 
     * @param energy The energy of the incident neutrino.
     * @return Calculated total cross-section value for the given energy.
     */
    G4double TotalCrossSection(G4double energy);

    OMSimSNTools mSNToolBox;
    G4double mFixedenergy;
    G4double mFixedenergy2;
    G4double mAlpha;
};

#endif
