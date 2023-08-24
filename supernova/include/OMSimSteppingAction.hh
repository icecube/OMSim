#ifndef OMSimSteppingAction_h
#define OMSimSteppingAction_h 1

#include <G4UserSteppingAction.hh>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class OMSimSteppingAction : public G4UserSteppingAction
{
  public:
    OMSimSteppingAction(){};
   ~OMSimSteppingAction(){};

    void UserSteppingAction(const G4Step*);
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
