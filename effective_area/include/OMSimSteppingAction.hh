#ifndef OMSimSteppingAction_h
#define OMSimSteppingAction_h 1
#include "G4Types.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "G4UserSteppingAction.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class OMSimSteppingAction : public G4UserSteppingAction
{
  public:
    OMSimSteppingAction();
   ~OMSimSteppingAction(){};

    void UserSteppingAction(const G4Step*);
    bool QEcheck(G4double lambda);
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
