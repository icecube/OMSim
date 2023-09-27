/** @file OMSimPrimaryGeneratorMessenger.cc
 *  @brief Primary generator messenger to choose the corresponding primary generator.
 * 
 *  @author Cristian Jesus Lozano Mariscal (c.lozano@wwu.de)
 * 
 *  @version Geant4 10.7
 */

#include "OMSimPrimaryGeneratorMessenger.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"


OMSimPrimaryGeneratorMessenger::OMSimPrimaryGeneratorMessenger
                                                  (OMSimPrimaryGeneratorAction* Gun)
:G4UImessenger(),
 Action(Gun),
 fDir(0), 
 fSelectActionCmd(0)
{
  fDir = new G4UIdirectory("/selectGun/");

fSelectActionCmd = new G4UIcmdWithAnInteger("/selectGun",this);
  fSelectActionCmd->SetGuidance("Select primary generator action");
  fSelectActionCmd->SetGuidance("0 Inverse Beta Decay");
  //fSelectActionCmd->SetGuidance("1 Elastic Scattering");
  fSelectActionCmd->SetParameterName("id",false);
  fSelectActionCmd->SetRange("id>=0 && id<4");
}


OMSimPrimaryGeneratorMessenger::~OMSimPrimaryGeneratorMessenger()
{
  delete fSelectActionCmd;
  delete fDir;
}

void OMSimPrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command,
                                               G4String newValue)
{ 
  if (command == fSelectActionCmd)
    Action->SelectAction(fSelectActionCmd->GetNewIntValue(newValue));      
}


