#include "OMSimEventAction.hh"
#include <G4RunManager.hh>

/**
 * @brief Custom actions at the beginning of the event.
 * This function sets the current event ID in the EventInfoManager.
 * @param evt Pointer to the current event.
 */
void OMSimEventAction::BeginOfEventAction(const G4Event* evt)
{
	EventInfoManager::getInstance().setCurrentEventID(evt->GetEventID());
}

/**
 * @brief Custom actions at the end of the event.
 * @param evt Pointer to the current event.
 */
void OMSimEventAction::EndOfEventAction(const G4Event* evt)
{

}
