/** @file OMSimPrimaryGeneratorMessenger.cc
 *  @brief Primary generator messenger to choose the corresponding primary generator.
 */

#include "OMSimPrimaryGeneratorMessenger.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithAnInteger.hh"


OMSimPrimaryGeneratorMessenger::OMSimPrimaryGeneratorMessenger
                                                  (OMSimPrimaryGeneratorAction* Gun)
:G4UImessenger(),
 m_action(Gun),
 m_directory(0), 
 m_selectedActionCmd(0)
{
  m_directory = new G4UIdirectory("/selectGun/");

m_selectedActionCmd = new G4UIcmdWithAnInteger("/selectGun",this);
  m_selectedActionCmd->SetGuidance("Select primary generator action");
  m_selectedActionCmd->SetGuidance("0 Inverse Beta Decay");
  //m_selectedActionCmd->SetGuidance("1 Elastic Scattering");
  m_selectedActionCmd->SetParameterName("id",false);
  m_selectedActionCmd->SetRange("id>=0 && id<4");
}


OMSimPrimaryGeneratorMessenger::~OMSimPrimaryGeneratorMessenger()
{
  delete m_selectedActionCmd;
  delete m_directory;
}

void OMSimPrimaryGeneratorMessenger::SetNewValue(G4UIcommand* command,
                                               G4String newValue)
{ 
  if (command == m_selectedActionCmd)
    m_action->SelectAction(m_selectedActionCmd->GetNewIntValue(newValue));      
}


