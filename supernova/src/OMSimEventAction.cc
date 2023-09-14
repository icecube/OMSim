#include "OMSimEventAction.hh"
#include <G4RunManager.hh>

OMSimEventAction::OMSimEventAction()
{}

OMSimEventAction::~OMSimEventAction()
{}

void OMSimEventAction::BeginOfEventAction(const G4Event* evt)
{
	EventInfoManager::getInstance().setCurrentEventID(evt->GetEventID());
}

void OMSimEventAction::EndOfEventAction(const G4Event* evt)
{

}
