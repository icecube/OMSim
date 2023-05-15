#ifndef OMSimEventAction_h
#define OMSimEventAction_h 1

#include "G4UserEventAction.hh"

#include <string>

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

#endif
