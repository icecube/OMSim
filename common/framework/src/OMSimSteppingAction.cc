#include "OMSimSteppingAction.hh"
#include "OMSimLogger.hh"

#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4SteppingManager.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
//since Geant4.10: include units manually

#include <G4SystemOfUnits.hh>


/**
* @brief Custom actions during a tracking step.
* This function checks for particles that seem to be stuck (e.g., in a loop in the pressure vessel)
* and kills them if they exceed a specified number of tracking steps.
* @param aStep Pointer to the current tracking step.
*/
void OMSimSteppingAction::UserSteppingAction(const G4Step* aStep)
{    G4Track* aTrack = aStep->GetTrack();
    
    //kill particles that are stuck... e.g. doing a loop in the pressure vessel
    if ( aTrack-> GetCurrentStepNumber() > 100000) {
        log_info("Particle {} with energy {} eV stuck, will be killed!", aTrack->GetDefinition()->GetParticleName(), 1239.84193/(aTrack->GetKineticEnergy()/eV));
        if ( aTrack->GetTrackStatus() != fStopAndKill ) {
            aTrack->SetTrackStatus(fStopAndKill);
        }
    }    

}
