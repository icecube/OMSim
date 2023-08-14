#ifndef OMSimPMTResponse_h
#define OMSimPMTResponse_h 1

#include <G4SystemOfUnits.hh>
#include <G4Types.hh>
#include <map>
#include <TGraph.h>
#include <TH2D.h>

class OMSimPMTResponse
{
public:

    static OMSimPMTResponse& getInstance() { // Meyers singleton
        static OMSimPMTResponse instance;
        return instance;
    }


    struct PMTPulse{
    G4double PE;
    G4double TransitTime;
    G4double DetectionProbability;
  };

    PMTPulse ProcessPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength);


private:
    std::vector<G4double> mScannedWavelengths{460*nm, 480*nm, 500*nm, 520*nm, 540*nm, 560*nm,  580*nm, 600*nm, 620*nm, 640*nm};

    double mX;
    double mY;
    G4double mWavelength;

    TGraph* mRelativeDetectionEfficiencyInterp;
    std::map<G4double, TH2D*> mGainG2Dmap;
    std::map<G4double, TH2D*> mGainResolutionG2Dmap;
    std::map<G4double, TH2D*> mTransitTimeG2Dmap;
    std::map<G4double, TH2D*> mTransitTimeSpreadG2Dmap;

    TH2D* createHistogramFromData(const std::string& pFilePath, const char* pTH2DName);

    G4double GetCharge(G4double pWavelengthKey);
    G4double GetCharge(G4double pWavelengthKey1, G4double pWavelengthKey2);

    G4double GetTransitTime(G4double pWavelengthKey);
    G4double GetTransitTime(G4double pWavelengthKey1, G4double pWavelengthKey2);

    PMTPulse GetPulseFromInterpolation(G4double pWavelengthKey1, G4double pWavelengthKey2);
    PMTPulse GetPulseFromKey(G4double pWavelengthKey);

    G4double WavelengthInterpolatedValue(std::map<G4double, TH2D*> pMap, G4double pWavelengthKey1, G4double pWavelengthKey2);

    OMSimPMTResponse();
    ~OMSimPMTResponse() = default;
    OMSimPMTResponse(const OMSimPMTResponse&) = delete;
    OMSimPMTResponse& operator=(const OMSimPMTResponse&) = delete;

};

#endif
//
