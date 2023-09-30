#include "OMSimTrackingAction.hh"
#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4TrackingManager.hh>

std::vector<G4String> splitStringByDelimiter(G4String const &s, char delim)
{
    std::vector<G4String> result;
    std::istringstream iss(s);

    for (G4String token; std::getline(iss, token, delim);)
    {
        result.push_back(std::move(token));
    }

    return result;
}

std::vector<G4String> splitStringByDelimiter(char *cs, char d)
{
    return splitStringByDelimiter(G4String(cs), d);
}

void OMSimTrackingAction::PostUserTrackingAction(const G4Track *aTrack)
{
    if (aTrack->GetDefinition()->GetParticleName() == "opticalphoton")
    {

        if ((aTrack->GetVolume()->GetName()).substr(0, 12) == "Photocathode")
        {
            G4double lEkin = aTrack->GetKineticEnergy();

            G4double h = 4.135667696E-15 * eV * s;
            G4double c = 2.99792458E17 * nm / s;

            std::vector<G4String> lPMTnrinfo = splitStringByDelimiter(aTrack->GetStep()->GetPreStepPoint()->GetTouchableHandle()->GetVolume(2)->GetName(), '_');
            std::vector<G4String> lModulenrinfo = splitStringByDelimiter(aTrack->GetStep()->GetPreStepPoint()->GetTouchableHandle()->GetVolume(4)->GetName(), '_');
            
            OMSimPMTResponse &lPhotocathodeResponse = OMSimPMTResponse::getInstance();
            OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

            G4ThreeVector lGlobalPosition = aTrack->GetPosition();
            G4ThreeVector lLocalPosition = aTrack->GetStep()->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(lGlobalPosition);

            G4double x = lLocalPosition.x() / mm;
            G4double y = lLocalPosition.y() / mm;
            G4double lR = std::sqrt(x * x + y * y);
            G4double lWavelength = h * c / lEkin;
            G4ThreeVector lDeltaPos = aTrack->GetVertexPosition() - lGlobalPosition;

            //if the QE cut is enabled and the photon doesn't pass the QE check, the function will exit early without further processing.
            if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut") && !lPhotocathodeResponse.passQE(lWavelength)) return;

            lHitManager.appendHitInfo(
                aTrack->GetGlobalTime(),
                aTrack->GetLocalTime(),
                aTrack->GetTrackLength() / m,
                lEkin / eV,
                atoi(lPMTnrinfo.at(1)),
                aTrack->GetMomentumDirection(),
                aTrack->GetPosition(),
                lLocalPosition,
                lDeltaPos.mag() / m,
                lPhotocathodeResponse.processPhotocathodeHit(x, y, lWavelength),
                atoi(lModulenrinfo.at(1)));
        }
    }
}
