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

        /*
        size_t idx_rindex1        = 0;
        const G4Material* aMaterial = aTrack->GetMaterial();
        G4MaterialPropertiesTable* aMaterialPropertiesTable = aMaterial->GetMaterialPropertiesTable();

        G4MaterialPropertyVector* RindexMPV = aMaterialPropertiesTable->GetProperty(kRINDEX);
        G4cout << RindexMPV->Value( aTrack->GetDynamicParticle()->GetTotalMomentum(), idx_rindex1)<< G4endl;
        */
        G4double lEkin = aTrack->GetKineticEnergy();

        G4double h = 4.135667696E-15*eV*s;
        G4double c = 2.99792458E17*nm/s;

        std::vector<G4String> n = explode(aTrack->GetStep()->GetPreStepPoint()->GetTouchableHandle()->GetVolume(2)->GetName(),'_');

        OMSimPMTResponse& lPhotocathodeResponse = OMSimPMTResponse::getInstance();
	    OMSimAnalysisManager& lAnalysisManager = OMSimAnalysisManager::getInstance();

        lAnalysisManager.stats_PMT_hit.push_back(atoi(n.at(1)));	
        G4ThreeVector lGlobalPosition = aTrack->GetPosition();
        G4ThreeVector lLocalPosition = aTrack->GetStep()->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(lGlobalPosition);

        G4double x = lLocalPosition.x()/mm;
        G4double y = lLocalPosition.y()/mm;
        G4double lR = std::sqrt(x*x+y*y);

        lAnalysisManager.lPulses.push_back(lPhotocathodeResponse.ProcessPhotocathodeHit(x, y, h*c/lEkin));

            G4ThreeVector lDeltaPos = aTrack->GetVertexPosition() - lGlobalPosition;
            lAnalysisManager.stats_photon_direction.push_back(aTrack->GetMomentumDirection());
            lAnalysisManager.stats_photon_position.push_back(aTrack->GetPosition());
            lAnalysisManager.stats_event_id.push_back(lAnalysisManager.current_event_id);
            lAnalysisManager.stats_photon_flight_time.push_back(aTrack->GetLocalTime());
            lAnalysisManager.stats_photon_track_length.push_back(aTrack->GetTrackLength()/m);
            lAnalysisManager.stats_hit_time.push_back(aTrack->GetGlobalTime());
            lAnalysisManager.stats_photon_energy.push_back(lEkin/eV);
            lAnalysisManager.stats_event_distance.push_back(lDeltaPos.mag()/m);
    }
    }
}

                
                
			
                
