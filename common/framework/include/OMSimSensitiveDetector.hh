
#ifndef LXePMTSD_h
#define LXePMTSD_h 1

#include "G4VSensitiveDetector.hh"
#include "OMSimPMTResponse.hh"

#include <vector>

class G4DataVector;
class G4HCofThisEvent;
class G4Step;

class OMSimSensitiveDetector : public G4VSensitiveDetector
{
public:
    OMSimSensitiveDetector(G4String pName);
    ~OMSimSensitiveDetector(){};

    G4bool ProcessHits(G4Step *pStep, G4TouchableHistory *) override;
    void setPMTResponse(OMSimPMTResponse* pResponse);

private:
    OMSimPMTResponse *mPMTResponse;
};

#endif
