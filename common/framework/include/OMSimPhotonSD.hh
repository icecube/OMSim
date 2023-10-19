
#ifndef OMSimPhotonSD_h
#define OMSimPhotonSD_h 1

#include "G4VSensitiveDetector.hh"
#include "OMSimPMTResponse.hh"

#include <vector>

class G4DataVector;
class G4HCofThisEvent;
class G4Step;

class OMSimPhotonSD : public G4VSensitiveDetector
{
public:
    OMSimPhotonSD(G4String pName);
    ~OMSimPhotonSD(){};

    G4bool ProcessHits(G4Step *pStep, G4TouchableHistory *) override;
    void setPMTResponse(OMSimPMTResponse* pResponse);

private:
    OMSimPMTResponse *mPMTResponse;
};

#endif
