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

    /**
     * @brief Process a hit on the photocathode into a PMT pulse.
     *
     * Transform a hit based on its location and photon wavelength to a PMT pulse,
     * considering the transit time, charge, and detection probability.
     *
     * @param pX  x position on photocathode
     * @param pY  y position on photocathode
     * @param pWavelength  Wavelength of the hitting photon
     * @return PMTPulse representing the processed hit.
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

    /**
     * @brief Create a histogram from provided data.
     *
     * Loads the data from a given path and constructs a histogram based on the data.
     *
     * @param pFilePath Path to the data file.
     * @param pTH2DName Name of the histogram.
     * @return Pointer to the created histogram.
     */
    TH2D *createHistogramFromData(const std::string &pFilePath, const char *pTH2DName);

    /**
     * @brief Retrieve the charge in PE from measurements for a given wavelength.
     *
     * This method returns the charge sampled from a Gaussian distribution with the mean
     * and standard deviation of the single photon electron (SPE) resolution for the
     * provided wavelength key.
     *
     * @param pWavelengthKey The key for selecting a scan from the available measurements.
     * @return G4double The charge in PE.
     */
    G4double getCharge(G4double pWavelengthKey);

    /**
     * @brief Retrieve the interpolated charge between two given wavelengths.
     *
     * This method interpolates the charge between the two provided wavelengths
     * and then returns the charge sampled from a Gaussian distribution with the mean
     * interpolated charge and interpolated single photon electron (SPE) resolution.
     *
     * @param pWavelength1 The first wavelength key for interpolation.
     * @param pWavelength2 The second wavelength key for interpolation.
     * @return G4double The interpolated charge in PE.
     */
    G4double getCharge(G4double pWavelengthKey1, G4double pWavelengthKey2);

    /**
     * @brief Retrieve the transit time from measurements for a given wavelength.
     *
     * This method returns the transit time sampled from a Gaussian distribution
     * with the mean transit time and transit time spread for the provided wavelength key.
     *
     * @param pWavelengthKey The key for selecting a scan from the available measurements.
     * @return G4double The transit time in ns.
     */
    G4double getTransitTime(G4double pWavelengthKey);

    /**
     * @brief Retrieve the interpolated transit time between two given wavelengths.
     *
     * This method interpolates the transit time between the two provided wavelengths
     * and then returns the time sampled from a Gaussian distribution with the mean
     * interpolated transit time and interpolated transit time spread.
     *
     * @param pWavelength1 The first wavelength key for interpolation.
     * @param pWavelength2 The second wavelength key for interpolation.
     * @return G4double The interpolated transit time in ns.
     */
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
