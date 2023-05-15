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

extern std::vector<G4String> explode(G4String s, char d);
extern OMSimAnalysisManager gAnalysisManager;
extern OMSimPMTResponse* gPhotocathodeResponse;
extern G4String	gHittype;



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

        gAnalysisManager.stats_PMT_hit.push_back(atoi(n.at(1)));	
        G4ThreeVector lGlobalPosition = aTrack->GetPosition();
        G4ThreeVector lLocalPosition = aTrack->GetStep()->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(lGlobalPosition);

        G4double x = lLocalPosition.x()/mm;
        G4double y = lLocalPosition.y()/mm;
        G4double lR = std::sqrt(x*x+y*y);

        gAnalysisManager.lPulses.push_back(gPhotocathodeResponse->ProcessPhotocathodeHit(x, y, h*c/lEkin));
        if (gHittype == "individual") {
            G4ThreeVector lDeltaPos = aTrack->GetVertexPosition() - lGlobalPosition;
            gAnalysisManager.stats_photon_direction.push_back(aTrack->GetMomentumDirection());
            gAnalysisManager.stats_photon_position.push_back(aTrack->GetPosition());
            gAnalysisManager.stats_event_id.push_back(gAnalysisManager.current_event_id);
            gAnalysisManager.stats_photon_flight_time.push_back(aTrack->GetLocalTime());
            gAnalysisManager.stats_photon_track_length.push_back(aTrack->GetTrackLength()/m);
            gAnalysisManager.stats_hit_time.push_back(aTrack->GetGlobalTime());
            gAnalysisManager.stats_photon_energy.push_back(lEkin/eV);
            gAnalysisManager.stats_event_distance.push_back(lDeltaPos.mag()/m);}
    }
    }
}

                
                
			
                
