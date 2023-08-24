#ifndef mdomPrimaryGeneratorMessenger_h
#define mdomPrimaryGeneratorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class mdomPrimaryGeneratorAction;
class G4UIdirectory;
class G4UIcmdWithAnInteger;


class mdomPrimaryGeneratorMessenger: public G4UImessenger
{
  public:
    mdomPrimaryGeneratorMessenger(mdomPrimaryGeneratorAction*);
   ~mdomPrimaryGeneratorMessenger();
    
    virtual void SetNewValue(G4UIcommand*, G4String);
    
  private:
    mdomPrimaryGeneratorAction* Action;
    
    G4UIdirectory*        fDir;       
    G4UIcmdWithAnInteger* fSelectActionCmd;
};


#endif

