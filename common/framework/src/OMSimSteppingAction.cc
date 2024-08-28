#include "OMSimSteppingAction.hh"
#include "OMSimLogger.hh"

#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4SteppingManager.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"


#include <G4SystemOfUnits.hh>


/**
* @brief Custom actions during a tracking step.
* This function checks for particles that seem to be stuck (e.g., in a loop in the pressure vessel)
* and kills them if they exceed a specified number of tracking steps. 
* @note This should not happen if you have an absorption length defined for all materials (in case of photons)
* @param aStep Pointer to the current tracking step.
*/
void OMSimSteppingAction::UserSteppingAction(const G4Step* p_step)
{    G4Track* track = p_step->GetTrack();
    
    //kill particles that are stuck... e.g. doing a loop in the pressure vessel
    if ( track-> GetCurrentStepNumber() > 100000) {
        log_info("Particle {} with energy {} eV stuck, will be killed!", track->GetDefinition()->GetParticleName(), 1239.84193/(track->GetKineticEnergy()/eV));
        if ( track->GetTrackStatus() != fStopAndKill ) {
            track->SetTrackStatus(fStopAndKill);
        }
    }    

}
