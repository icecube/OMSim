#include "OMSimSensitiveDetector.hh"
#include "G4ios.hh"
#include "G4LogicalVolume.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTypes.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VTouchable.hh"
#include "OMSimEventAction.hh"
#include "G4OpticalPhoton.hh"
#include "OMSimPMTResponse.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"

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

OMSimSensitiveDetector::OMSimSensitiveDetector(G4String name)
    : G4VSensitiveDetector(name)
{
}

void OMSimSensitiveDetector::setPMTResponse(OMSimPMTResponse *pResponse)
{
  mPMTResponse = pResponse;
}

G4bool OMSimSensitiveDetector::ProcessHits(G4Step *aStep, G4TouchableHistory *)
{
  G4Track *aTrack = aStep->GetTrack();
  if (aTrack->GetDefinition() == G4OpticalPhoton::Definition())
  {
    if (!aStep->IsLastStepInVolume() && aTrack->GetTrackStatus() == fStopAndKill)
    {

      aStep->SetLastStepFlag();

      G4double lEkin = aTrack->GetKineticEnergy();

      G4double h = 4.135667696E-15 * eV * s;
      G4double c = 2.99792458E17 * nm / s;

      std::vector<G4String> lPMTnrinfo = splitStringByDelimiter(aTrack->GetStep()->GetPreStepPoint()->GetTouchableHandle()->GetVolume(2)->GetName(), '_');

      G4ThreeVector lGlobalPosition = aTrack->GetPosition();
      G4ThreeVector lLocalPosition = aTrack->GetStep()->GetPostStepPoint()->GetTouchableHandle()->GetHistory()->GetTopTransform().TransformPoint(lGlobalPosition);

      G4double x = lLocalPosition.x() / mm;
      G4double y = lLocalPosition.y() / mm;
      G4double lR = std::sqrt(x * x + y * y);
      G4double lWavelength = h * c / lEkin;
      G4ThreeVector lDeltaPos = aTrack->GetVertexPosition() - lGlobalPosition;

      // if the QE cut is enabled and the photon doesn't pass the QE check, the function will exit early without further processing.
      if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut") && !mPMTResponse->passQE(lWavelength)) return false;
        

      OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
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
          mPMTResponse->processPhotocathodeHit(x, y, lWavelength),
          atoi(SensitiveDetectorName));

      return true;
    }
  }

  return false;
}
