/**
 *  @file OMSimPMTResponse.hh
 *  @brief Simulation of PMT response.
 *  @ingroup common
 */
#pragma once

#include <G4SystemOfUnits.hh>
#include <G4Types.hh>
#include <map>
#include <TGraph.h>
#include <TH2D.h>
/**
 *  @class OMSimPMTResponse
 *  @brief Class to simulate PMT response.
 *
 *  Process hits on a photocathode and get resulting pulse properties based on lab. measurement data.
 */
class OMSimPMTResponse
{
public:
    OMSimPMTResponse();
    ~OMSimPMTResponse();
    /**
     *  @struct PMTPulse
     *  @brief Represents the output pulse for a detected photon.
     */
    struct PMTPulse
    {
        G4double PE;                   ///< Charge in photoelectrons.
        G4double transitTime;          ///< Detection time relative to average response of PMT.
        G4double detectionProbability; ///< Probability of photon being detected.
    };

    PMTPulse processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength);
    void makeQEweightInterpolator(const std::string& p_FileNameAbsorbedFraction);
    void makeQEInterpolator(const std::string& p_FileName);
    void makeCEweightInterpolator(const std::string& p_FileName);
    void makeScansInterpolators(const std::string &p_PathToFiles);
    void setScannedWavelengths(std::vector<double> p_wavelengths);

private:

    G4double getCharge(G4double p_WavelengthKey);
    G4double getCharge(G4double p_WavelengthKey1, G4double p_WavelengthKey2);
    G4double getTransitTime(G4double p_WavelengthKey);
    G4double getTransitTime(G4double p_WavelengthKey1, G4double p_WavelengthKey2);
    PMTPulse getPulseFromInterpolation(G4double p_WavelengthKey1, G4double p_WavelengthKey2);
    PMTPulse getPulseFromKey(G4double p_WavelengthKey);
    G4double wavelengthInterpolatedValue(std::map<G4double, TH2D *> p_Map, G4double p_WavelengthKey1, G4double p_WavelengthKey2);
    std::vector<G4double> getScannedWavelengths();
    

    bool m_scansInterpolatorsAvailable = false;
    bool m_QEWeightInterpolatorAvailable = false;
    bool m_QEInterpolatorAvailable = false;
    bool m_CEWeightInterpolatorAvailable = false;
    bool m_simplePMT;
    double m_X;
    double m_Y;

    G4double m_wavelength;
    std::vector<G4double> m_scannedWavelengths;

    TGraph *m_relativeDetectionEfficiencyInterpolator;
    TGraph *m_QEfileInterpolator;
    TGraph *m_weightAbsorbedToQEInterpolator;
    std::map<G4double, TH2D *> m_gainG2Dmap;
    std::map<G4double, TH2D *> m_gainResolutionG2Dmap;
    std::map<G4double, TH2D *> m_transitTimeG2Dmap;
    std::map<G4double, TH2D *> m_transitTimeSpreadG2Dmap;

};