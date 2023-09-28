/**
 *  @file OMSimPMTResponse.hh
 *  @brief Provides simulation for PMT response.
 *  This class is responsible for transforming hits on photocathode into pulses, with transit time, charge, and detection probability.
 *  @ingroup common
 */
#ifndef OMSimPMTResponse_h
#define OMSimPMTResponse_h 1

#include <G4SystemOfUnits.hh>
#include <G4Types.hh>
#include <map>
#include <TGraph.h>
#include <TH2D.h>

/**
 *  @class OMSimPMTResponse
 *  @brief Singleton class to simulate PMT response.
 *
 *  Provides methods to process hits on a photocathode and get resulting pulses
 *  based on various measurements and scans.
 */
class OMSimPMTResponse
{
public:
    static OMSimPMTResponse &getInstance()
    { // Meyers singleton
        static OMSimPMTResponse instance;
        return instance;
    }

    /**
     *  @struct PMTPulse
     *  @brief Represents the output pulse for a detected photon.
     */
    struct PMTPulse
    {
        G4double PE;                   ///< Charge in photoelectrons.
        G4double TransitTime;          ///< Detection time relative to average response of PMT.
        G4double DetectionProbability; ///< Probability of photon being detected.
    };

    PMTPulse processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength);

    bool passQE(G4double pWavelength);

private:
    std::vector<G4double> mScannedWavelengths{460 * nm, 480 * nm, 500 * nm, 520 * nm, 540 * nm, 560 * nm, 580 * nm, 600 * nm, 620 * nm, 640 * nm};

    double mX;
    double mY;
    G4double mWavelength;

    TGraph *mRelativeDetectionEfficiencyInterp;
    TGraph *mQEInterp;
    std::map<G4double, TH2D *> mGainG2Dmap;
    std::map<G4double, TH2D *> mGainResolutionG2Dmap;
    std::map<G4double, TH2D *> mTransitTimeG2Dmap;
    std::map<G4double, TH2D *> mTransitTimeSpreadG2Dmap;

    void configureQEinterpolator();
    TH2D *createHistogramFromData(const std::string &pFilePath, const char *pTH2DName);
    G4double getCharge(G4double pWavelengthKey);
    G4double getCharge(G4double pWavelengthKey1, G4double pWavelengthKey2);
    G4double getTransitTime(G4double pWavelengthKey);
    G4double getTransitTime(G4double pWavelengthKey1, G4double pWavelengthKey2);
    PMTPulse getPulseFromInterpolation(G4double pWavelengthKey1, G4double pWavelengthKey2);
    PMTPulse getPulseFromKey(G4double pWavelengthKey);
    G4double wavelengthInterpolatedValue(std::map<G4double, TH2D *> pMap, G4double pWavelengthKey1, G4double pWavelengthKey2);

    OMSimPMTResponse();
    ~OMSimPMTResponse();
    OMSimPMTResponse(const OMSimPMTResponse &) = delete;
    OMSimPMTResponse &operator=(const OMSimPMTResponse &) = delete;
};

#endif
//
