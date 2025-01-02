/**
 * @file
 * @brief Defines the OMSimEventAction and EventInfoManager classes for the radioactive decays simulation.
 * @ingroup radioactive
 */
#ifndef OMSimEventAction_h
#define OMSimEventAction_h 1

#include "G4UserEventAction.hh"
#include <string>
#include "G4Types.hh"

class G4Event;


/**
 * @class OMSimEventAction
 * @brief Handles custom actions at the beginning and end of each event.
 * 
 * This class defines custom actions that are performed at the start and end
 * of each Geant4 event simulation. It interacts with analysis and hit managers
 * to handle, record, and reset event information.
 * @ingroup radioactive
 */
class OMSimEventAction : public G4UserEventAction
{
	public:
		OMSimEventAction(){};
		~OMSimEventAction(){};

	public:

		void BeginOfEventAction(const G4Event*);
		void EndOfEventAction(const G4Event*);

	private:
};

/**
 * @class EventInfoManager
 * @brief Singleton class for managing event-specific information.
 * 
 * This class provides a mechanism to set and get the current event ID 
 * across different parts of the simulation without having to pass event objects around.
 * @ingroup radioactive
 */
class EventInfoManager {
public:
    static EventInfoManager& getInstance() {
        static EventInfoManager instance;
        return instance;
    }

    void setCurrentEventID(G4int id) {
        currentEventID = id;
    }

    G4int getCurrentEventID() const {
        return currentEventID;
    }

private:
    EventInfoManager() : currentEventID(-1) {}

    G4int currentEventID;
};

#endif
