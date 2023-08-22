/**
 * @file OMSimIBD.hh
 * @brief This file contains the class in charge of generating positrons from IBD of supernova antineutrino interactions
 *
 * @author Cristian Jesus Lozano Mariscal
 * @version 10.1
 * @ingroup SN
 */


#ifndef OMSimIBD
#define OMSimIBD 1
 
#include "G4VUserPrimaryGeneratorAction.hh"

#include "globals.hh"
#include <vector>
#include "G4ThreeVector.hh"

class G4ParticleGun;
class G4Event;

/**
 * @class OMSimIBD
 * @brief Class in charge of generating positrons from IBD of supernova antineutrino interactions
 *
 * Supported by OMSimSNTools, this class generates the corresponding positrons after the inverse beta decay
 * and weights the interactions corresponding to the cross section and the generated volume
 *
 * @ingroup SN
 */
class OMSimIBD : public G4VUserPrimaryGeneratorAction
{
public:
    /**
     * @brief Constructor of OMSimIBD class
     *
     * The class constructor initializes various properties of the model based on given input 
     * parameters or data files.
     *
     * @param gun Pointer to the particle gun object used for simulation.
     */
	OMSimIBD(G4ParticleGun*);    

    /**
     * @brief Destructor of OMSimIBD class
     */
	~OMSimIBD();

    /**
     * @brief Generates primary events for the simulation.
     * 
     * This function sets the initial properties of the electronic antineutrinos such as particle type, position,
     * and energy for the primary event based on given input parameters or data files. The energy is determined from a tabulated 
     * distribution or from a fixed mean energy, and the antineutrino's incoming direction is assumed to be (0,0,-1). The function also 
     * computes the angle distribution and the positron's energy derived from the antineutrino energy and its angle. Finally,
     * the function computes the total cross-section and sets weights accordingly, passing the information to the analysis manager.
     *
     * @param anEvent Pointer to the G4Event object where primaries will be generated.
     * @todo Consider breaking down this function into smaller, more specialized sub-functions for clarity and modularity.
     * @todo Ensure that any new parameters or member variables used are appropriately documented.
     */
	void GeneratePrimaries(G4Event* anEvent);

    G4double InverseCumul(int ControlParameter);  // ?
    G4int                  nPoints0;     //tabulated function
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
    
    G4double ThresholdEnergy;
    G4double Delta;
    G4double y2;
    G4double NTargets;
    G4double me;
    G4double mp;
    G4double mn;
    G4double Gf;
    G4double consg;
    
	std::vector<double> nubar_time;
	std::vector<double> nubar_luminosity;
	std::vector<double> nubar_meanenergy;
	std::vector<double> nubar_meanenergysquare;

private:     
    G4ParticleGun* ParticleGun;

    /**
     * @brief Computes the angular distribution of the reaction \( \bar{\nu}_e + p \rightarrow e^+ + n \).
     * 
     * This function evaluates the angular distribution of the positron based on the energy of the incident electronic antineutrino.
     * The calculation follows the theoretical approach outlined in "The angular distribution of the reaction \( \bar{\nu}_e + p \rightarrow e^+ + n \)"
     * by P. Vogel and J. F. Beacom (1999), specifically referencing Eq. 14. 
     * 
     * @param Enubar Energy of the incoming electronic antineutrino.
     * @note This function populates the angdist_x and angdist_y vectors, which later can be used in other parts of the simulation.
     */
	void DistFunction(G4double Enubar);

    /**
     * @brief Computes the energy of a positron resulting from the inverse beta decay.
     * 
     * Given the energy of the incident electronic antineutrino and the scatter angle, this function calculates the 
     * energy of the emitted positron following inverse beta decay. The theoretical basis for this calculation is 
     * given in "The angular distribution of the reaction \( \nu_e + p \rightarrow e^+ + n \)" by P. Vogel and J. F. Beacom (1999), specifically referencing Eq. 13.
     * 
     * @param Enubar Energy of the incoming electronic antineutrino.
     * @param costheta Cosine of the scatter angle between the direction of the antineutrino's momentum and the positron's momentum.
     * 
     * @return Energy of the emitted positron as a result of the inverse beta decay process.
     * @reference P. Vogel, J. F. Beacom. (1999). The angular distribution of the reaction \( \nu_e + p \rightarrow e^+ + n \). Phys.Rev., D60, 053003
     */
    G4double PositronEnergy(G4double Enubar, G4double costheta);

    /**
     * @brief Calculates the total cross-section of the inverse beta decay reaction for a given energy.
     * 
     * This function estimates the total cross-section of the inverse beta decay, which can be used 
     * to weigh each event. The theoretical basis for this calculation is presented in "Future detection 
     * of supernova neutrino burst and explosion mechanism" 
     * by T. Totani, K. Sato, H. E. Dalhed, and J. R. Wilson (1998), specifically referencing Equation 9.
     * 
     * @param energy Energy of the incoming electronic antineutrino.
     * 
     * @return Total cross-section for the given energy.
     * @reference T. Totani, K. Sato, H. E. Dalhed, J. R. Wilson, "Future detection of supernova neutrino burst 
     * and explosion mechanism", Astrophys. J. 496, 1998, 216–225, Preprint astro-ph/9710203, 1998.
     */
    G4double TotalCrossSection(G4double energy);

    G4double fixenergy;
    G4double fixenergy2;
    G4double alpha;
};

#endif
