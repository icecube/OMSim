#pragma once

#include "G4UserEventAction.hh"
#include <string>
#include "G4Types.hh"

class G4Event;
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

