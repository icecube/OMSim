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
 * @ingroup EffectiveArea
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


#endif
