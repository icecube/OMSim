#ifndef OMSimEventAction_h
#define OMSimEventAction_h 1

#include "G4UserEventAction.hh"
#include <string>
#include "G4Types.hh"

class G4Event;

class OMSimEventAction : public G4UserEventAction
{
	public:
		OMSimEventAction();
		~OMSimEventAction();

	public:
		void BeginOfEventAction(const G4Event*);
		void EndOfEventAction(const G4Event*);

	private:
};


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
