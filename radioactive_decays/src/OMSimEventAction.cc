#include "OMSimEventAction.hh"
#include "OMSimAnalysisManager.hh"
#include <G4RunManager.hh>

OMSimEventAction::OMSimEventAction()
{}

OMSimEventAction::~OMSimEventAction()
{}

void OMSimEventAction::BeginOfEventAction(const G4Event* evt)
{
	OMSimHitManager::getInstance().mCurrentEventNumber = evt->GetEventID();
}

void OMSimEventAction::EndOfEventAction(const G4Event* evt)
{

}
