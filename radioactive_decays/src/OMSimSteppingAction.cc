#include "OMSimSteppingAction.hh"

#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "G4SteppingManager.hh"
#include "G4Step.hh"
#include "G4ThreeVector.hh"
//since Geant4.10: include units manually

#include "OMSimAnalysisManager.hh"
#include <G4SystemOfUnits.hh>

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
