#include "OMSimTrackingAction.hh"
#include "OMSimEffectiveAreaAnalyisis.hh"
#include <G4TrackingManager.hh>

std::vector<G4String> split_string_by_delimiter(G4String const &s, char delim) {
    std::vector<G4String> result;
    std::istringstream iss(s);

    for (G4String token; std::getline(iss, token, delim); ) {
        result.push_back(std::move(token));
    }

    return result;
}

std::vector<G4String> split_string_by_delimiter(char* cs, char d) {
    return split_string_by_delimiter(G4String(cs), d);
}


void OMSimTrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{
    if ( aTrack->GetDefinition()->GetParticleName() == "opticalphoton" ) {

        if ((aTrack->GetVolume()->GetName()).substr(0,12) == "Photocathode"){
        G4double lEkin = aTrack->GetKineticEnergy();

        G4double h = 4.135667696E-15*eV*s;
        G4double c = 2.99792458E17*nm/s;

        std::vector<G4String> n = split_string_by_delimiter(aTrack->GetStep()->GetPreStepPoint()->GetTouchableHandle()->GetVolume(2)->GetName(),'_');

        OMSimPMTResponse& lPhotocathodeResponse = OMSimPMTResponse::getInstance();
	    OMSimEffectiveAreaAnalyisis& lAnalysisManager = OMSimEffectiveAreaAnalyisis::getInstance();

        G4ThreeVector lGlobalPosition = aTrack->GetPosition();
        G4ThreeVector lLocalPosition = aTrack->GetStep()->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(lGlobalPosition);

        G4double x = lLocalPosition.x()/mm;
        G4double y = lLocalPosition.y()/mm;
        G4double lR = std::sqrt(x*x+y*y);

        G4ThreeVector lDeltaPos = aTrack->GetVertexPosition() - lGlobalPosition;

        lAnalysisManager.appendHitInfo(
            aTrack->GetGlobalTime(),
            aTrack->GetLocalTime(),
            aTrack->GetTrackLength()/m,
            lEkin/eV,
            atoi(n.at(1)),
            aTrack->GetMomentumDirection(),
            aTrack->GetPosition(),
            lLocalPosition,
            lDeltaPos.mag()/m,
            lPhotocathodeResponse.processPhotocathodeHit(x, y, h*c/lEkin)
        );
    }
    }
}

                
                
			
                
