#pragma once

#include <G4UserTrackingAction.hh>

class OMSimTrackingAction : public G4UserTrackingAction
{
	public:
		OMSimTrackingAction(){};
		~OMSimTrackingAction(){};
	
		void PreUserTrackingAction(const G4Track*){};
		void PostUserTrackingAction(const G4Track*){};
		
	private:
};

