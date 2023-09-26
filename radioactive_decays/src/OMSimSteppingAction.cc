#include "OMSimSteppingAction.hh"

#include "G4Step.hh"
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
        G4cout << "Particle stuck   " <<  aTrack->GetDefinition()->GetParticleName()  << " " << 1239.84193/(aTrack->GetKineticEnergy()/eV)<< G4endl;
        if ( aTrack->GetTrackStatus() != fStopAndKill ) {
            aTrack->SetTrackStatus(fStopAndKill);
        }
    }    

}
