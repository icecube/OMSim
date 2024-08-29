#pragma once

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
    OMSimPrimaryGeneratorAction* m_action;
    
    G4UIdirectory*        m_directory;       
    G4UIcmdWithAnInteger* m_selectedActionCmd;
};


