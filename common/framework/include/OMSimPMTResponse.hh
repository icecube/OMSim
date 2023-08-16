/** @file OMSimPMTResponse.hh
 *  @brief PMT response simulator
 */

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
    static OMSimPMTResponse &getInstance()
    { // Meyers singleton
        static OMSimPMTResponse instance;
        return instance;
    }

    struct PMTPulse
    {
        G4double PE;
        G4double TransitTime;
        G4double DetectionProbability;
    };
    /**
     * Transform a hit on the photocathode to a Pulse, with the transit time, charge and detection probability.
     * @param pX  x position on photocathode
     * @param pY  x position on photocathode
     * @param pWavelength  Wavelength of photon
     * @return PMTPulse Struct with transit time, charge and detection probability
     */
    PMTPulse processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength);

private:
    std::vector<G4double> mScannedWavelengths{460 * nm, 480 * nm, 500 * nm, 520 * nm, 540 * nm, 560 * nm, 580 * nm, 600 * nm, 620 * nm, 640 * nm};

    double mX;
    double mY;
    G4double mWavelength;

    TGraph *mRelativeDetectionEfficiencyInterp;
    std::map<G4double, TH2D *> mGainG2Dmap;
    std::map<G4double, TH2D *> mGainResolutionG2Dmap;
    std::map<G4double, TH2D *> mTransitTimeG2Dmap;
    std::map<G4double, TH2D *> mTransitTimeSpreadG2Dmap;

    TH2D *createHistogramFromData(const std::string &pFilePath, const char *pTH2DName);

    /**
     * Get mean charge and SPE resolution from measurements for a given wavelength and sample from gaussian with mean PE and std SPE_res.
     * There are only a limited number of scans, which one can select from the scan map. We use TGraph2D from ROOT to interpolate selected scan.
     * @param pWavelengthKey  key for selecting a scan
     * @return G4double charge in PE
     */
    G4double getCharge(G4double pWavelengthKey);
    G4double getCharge(G4double pWavelengthKey1, G4double pWavelengthKey2);

    /**
     * Get transit time and transit time spread from measurements for a given wavelength and sample from gaussian with mean TT and std TTS.
     * There are only a limited number of scans, which one can select from the scan map. We use TGraph2D from ROOT to interpolate selected scan.
     * @param pWavelengthKey  key for selecting a scan
     * @return G4double transit time
     */
    G4double getTransitTime(G4double pWavelengthKey);
    G4double getTransitTime(G4double pWavelengthKey1, G4double pWavelengthKey2);

    /**
     * Get a finished Pulse interpolating scan results between measured wavelengths (simulated photons has a wavelength between these two)
     * @param pWavelengthKey1 first wavelength key to be used
     * @param pWavelengthKey2 second wavelength key to be used
     * @return PMTPulse Struct with transit time, charge and detection probability
     */
    PMTPulse getPulseFromInterpolation(G4double pWavelengthKey1, G4double pWavelengthKey2);

    /**
     * Get a finished Pulse using a key for the maps directly
     * @param pWavelengthKey  wavelength key to be used
     * @return PMTPulse Struct with transit time, charge and detection probability
     */
    PMTPulse getPulseFromKey(G4double pWavelengthKey);

    /**
     * Get interpolated value between two scans of different wavelenths
     * @param pMap  Map with TGraph2D scans to be used
     * @param pWavelength1  first reference wavelength for interpolation
     * @param pWavelength2  second reference wavelength for interpolation
     * @return G4double interpolated value
     */
    G4double wavelengthInterpolatedValue(std::map<G4double, TH2D *> pMap, G4double pWavelengthKey1, G4double pWavelengthKey2);

    OMSimPMTResponse();
    ~OMSimPMTResponse() = default;
    OMSimPMTResponse(const OMSimPMTResponse &) = delete;
    OMSimPMTResponse &operator=(const OMSimPMTResponse &) = delete;
};

#endif
//
