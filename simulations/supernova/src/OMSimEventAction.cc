#include "OMSimEventAction.hh"
#include "OMSimSNAnalysis.hh"
#include <G4RunManager.hh>
#include "OMSimLogger.hh"
OMSimEventAction::OMSimEventAction()
{}

OMSimEventAction::~OMSimEventAction()
{}

void OMSimEventAction::BeginOfEventAction(const G4Event* evt)
{

}

void OMSimEventAction::EndOfEventAction(const G4Event* evt)
{	
	OMSimSNAnalysis::getInstance().processEvent();
	OMSimHitManager::getInstance().reset();
}
