#include "OMSimEventAction.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimAnalysisManager.hh"
#include "G4TrackingManager.hh"
#include "G4Track.hh"
#include "G4ThreeVector.hh"
#include "G4SystemOfUnits.hh"
#include "OMSimPMTResponse.hh"
#include <TGraph.h>

#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4OpticalParameters.hh"
#include "G4MaterialPropertyVector.hh"

std::vector<G4String> explode(G4String s, char d) {
	std::vector<G4String> o;
	int i,j;
	i = s.find_first_of("#");
	if (i == 0) return o;
 	while (s.size() > 0) {
		i = s.find_first_of(d);
		j = s.find_last_of(d);
		o.push_back(s.substr(0, i));
		if (i == j) {
			o.push_back(s.substr(j+1));
			break;
		}
		s.erase(0,i+1);
 	}
	return o;// o beinhaltet s ohne d
}

std::vector<G4String> explode(char* cs, char d) {
	std::vector<G4String> o;
	G4String s = cs;
	return explode(s,d);
}

OMSimTrackingAction::OMSimTrackingAction()
:G4UserTrackingAction()
{
}

OMSimTrackingAction::~OMSimTrackingAction()
{
}

void OMSimTrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{	
}

void OMSimTrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{
    if ( aTrack->GetDefinition()->GetParticleName() == "opticalphoton" ) {

        if ((aTrack->GetVolume()->GetName()).substr(0,12) == "Photocathode"){
        G4double lEkin = aTrack->GetKineticEnergy();

        G4double h = 4.135667696E-15*eV*s;
        G4double c = 2.99792458E17*nm/s;

        std::vector<G4String> n = explode(aTrack->GetStep()->GetPreStepPoint()->GetTouchableHandle()->GetVolume(2)->GetName(),'_');

        OMSimPMTResponse& lPhotocathodeResponse = OMSimPMTResponse::getInstance();
	    OMSimAnalysisManager& lAnalysisManager = OMSimAnalysisManager::getInstance();
        G4ThreeVector lGlobalPosition = aTrack->GetPosition();
        G4ThreeVector lLocalPosition = aTrack->GetStep()->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(lGlobalPosition);

        G4double x = lLocalPosition.x()/mm;
        G4double y = lLocalPosition.y()/mm;
        G4double lR = std::sqrt(x*x+y*y);

        G4ThreeVector lDeltaPos = aTrack->GetVertexPosition() - lGlobalPosition;

        lAnalysisManager.mHits.event_id.push_back(lAnalysisManager.mCurrentEventNumber);
        lAnalysisManager.mHits.hit_time.push_back(aTrack->GetGlobalTime());
        lAnalysisManager.mHits.photon_flight_time.push_back(aTrack->GetLocalTime());
        lAnalysisManager.mHits.photon_track_length.push_back(aTrack->GetTrackLength()/m);
        lAnalysisManager.mHits.photon_energy.push_back(lEkin/eV);
        lAnalysisManager.mHits.PMT_hit.push_back(atoi(n.at(1)));
        lAnalysisManager.mHits.photon_direction.push_back(aTrack->GetMomentumDirection());
        lAnalysisManager.mHits.photon_global_position.push_back(aTrack->GetPosition());
        lAnalysisManager.mHits.photon_local_position.push_back(lLocalPosition);
        lAnalysisManager.mHits.event_distance.push_back(lDeltaPos.mag()/m);
        lAnalysisManager.mHits.PMT_response.push_back(lPhotocathodeResponse.ProcessPhotocathodeHit(x, y, h*c/lEkin));
    }
    }
}

                
                
			
                
