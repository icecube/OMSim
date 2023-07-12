#include "OMSimEventAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimTrackingAction.hh"

#include "OMSimAnalysisManager.hh"

#include "G4Event.hh"
#include "G4EventManager.hh"
#include "G4TrajectoryContainer.hh"
#include "G4Trajectory.hh"
#include "G4ios.hh"
#include "G4RunManager.hh"
//#include "TH1.h"


OMSimEventAction::OMSimEventAction()
{}

OMSimEventAction::~OMSimEventAction()
{}

void OMSimEventAction::BeginOfEventAction(const G4Event* evt)
{
	OMSimAnalysisManager::getInstance().current_event_id = evt->GetEventID();
}

void OMSimEventAction::EndOfEventAction(const G4Event* evt)
{

}
