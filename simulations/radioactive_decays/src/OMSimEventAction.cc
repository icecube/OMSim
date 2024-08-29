#include "OMSimEventAction.hh"
#include "OMSimDecaysAnalysis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

#include <G4RunManager.hh>

void OMSimEventAction::BeginOfEventAction(const G4Event *evt)
{
}

/**
 * @brief Custom actions at the end of the event.
 * 
 * Depending on the arguments set, this function will write hit and decay 
 * information with the analysis manager and reset hit and analysis data for the next event.
 * @param p_event Pointer to the current event.
 */
void OMSimEventAction::EndOfEventAction(const G4Event *p_event)
{
	if (!OMSimCommandArgsTable::getInstance().get<bool>("multiplicity_study"))
	{
		log_debug("End of event, saving information and reseting (thread {})", G4Threading::G4GetThreadId());
		OMSimDecaysAnalysis &analysisManager = OMSimDecaysAnalysis::getInstance();
		analysisManager.writeThreadHitInformation();
		analysisManager.writeThreadDecayInformation();
		analysisManager.reset();
	}
}
