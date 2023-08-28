/** @file mdomPrimaryGeneratorMessenger.cc
 *  @brief Primary generator messenger to choose the corresponding primary generator.
 * 
 *  @author Cristian Jesus Lozano Mariscal (c.lozano@wwu.de)
 * 
 *  @version Geant4 10.7
 */

#include "mdomPrimaryGeneratorMessenger.hh"
#include "mdomPrimaryGeneratorAction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"


mdomPrimaryGeneratorMessenger::mdomPrimaryGeneratorMessenger
                                                  (mdomPrimaryGeneratorAction* Gun)
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


mdomPrimaryGeneratorMessenger::~mdomPrimaryGeneratorMessenger()
{
  delete fSelectActionCmd;
  delete fDir;
}

void mdomPrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command,
                                               G4String newValue)
{ 
  if (command == fSelectActionCmd)
    Action->SelectAction(fSelectActionCmd->GetNewIntValue(newValue));      
}


