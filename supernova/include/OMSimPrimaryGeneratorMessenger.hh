#ifndef OMSimPrimaryGeneratorMessenger_h
#define OMSimPrimaryGeneratorMessenger_h 1

#include "G4UImessenger.hh"
#include "globals.hh"

class OMSimPrimaryGeneratorAction;
class G4UIdirectory;
class G4UIcmdWithAnInteger;


class OMSimPrimaryGeneratorMessenger: public G4UImessenger
{
  public:
    OMSimPrimaryGeneratorMessenger(OMSimPrimaryGeneratorAction*);
   ~OMSimPrimaryGeneratorMessenger();
    
    virtual void SetNewValue(G4UIcommand*, G4String);
    
  private:
    OMSimPrimaryGeneratorAction* Action;
    
    G4UIdirectory*        fDir;       
    G4UIcmdWithAnInteger* fSelectActionCmd;
};


#endif

