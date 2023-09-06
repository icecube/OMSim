#include "OMSimEventAction.hh"
#include "OMSimDecaysAnalysis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

#include <G4RunManager.hh>

OMSimEventAction::OMSimEventAction()
{
}

OMSimEventAction::~OMSimEventAction()
{
}

void OMSimEventAction::BeginOfEventAction(const G4Event *evt)
{
	EventInfoManager::getInstance().setCurrentEventID(evt->GetEventID());
}

void OMSimEventAction::EndOfEventAction(const G4Event *evt)
{
	OMSimDecaysAnalysis &lAnalysisManager = OMSimDecaysAnalysis::getInstance();
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	if (!lArgs.get<bool>("multiplicity_study"))
	{
		lAnalysisManager.writeHitInformation();
		lAnalysisManager.writeDecayInformation();
		lHitManager.reset();
		lAnalysisManager.reset();
	}
}
