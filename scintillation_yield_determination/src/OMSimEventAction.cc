#include "OMSimEventAction.hh"
#include "OMSimDecaysAnalysis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

#include <G4RunManager.hh>

/**
 * @brief Custom actions at the beginning of the event.
 * This function sets the current event ID in the EventInfoManager.
 * @param evt Pointer to the current event.
 */
void OMSimEventAction::BeginOfEventAction(const G4Event *evt)
{
	EventInfoManager::getInstance().setCurrentEventID(evt->GetEventID());
}

/**
 * @brief Custom actions at the end of the event.
 * 
 * Depending on the arguments set, this function will write hit and decay 
 * information with the analysis manager and reset hit and analysis data for the next event.
 * @param evt Pointer to the current event.
 */
void OMSimEventAction::EndOfEventAction(const G4Event *evt)
{


}
