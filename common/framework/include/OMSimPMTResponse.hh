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
 *  @brief Singleton class to simulate PMT response.
 *
 *  Process hits on a photocathode and get resulting pulse properties based on lab. measurement data.
 */
class OMSimPMTResponse
{
protected:
    OMSimPMTResponse();
    ~OMSimPMTResponse();

public:
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

    virtual PMTPulse processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength);
    static void shutdown(){};

protected:
    void configureQEweightInterpolator(const std::string& p_FileNameAbsorbedFraction, const std::string& p_FileNameTargetQE);
    void configureCEweightInterpolator(const std::string& p_FileName);
    void configureScansInterpolators(const std::string &p_PathToFiles);

    G4double getCharge(G4double p_WavelengthKey);
    G4double getCharge(G4double p_WavelengthKey1, G4double p_WavelengthKey2);
    G4double getTransitTime(G4double p_WavelengthKey);
    G4double getTransitTime(G4double p_WavelengthKey1, G4double p_WavelengthKey2);
    PMTPulse getPulseFromInterpolation(G4double p_WavelengthKey1, G4double p_WavelengthKey2);
    PMTPulse getPulseFromKey(G4double p_WavelengthKey);
    G4double wavelengthInterpolatedValue(std::map<G4double, TH2D *> p_Map, G4double p_WavelengthKey1, G4double p_WavelengthKey2);

    virtual std::vector<G4double> getScannedWavelengths() = 0;

    bool m_scansInterpolatorsAvailable = false;
    bool m_QEWeightInterpolatorAvailable = false;
    bool m_CEWeightInterpolatorAvailable = false;
    bool m_simplePMT;
    double m_X;
    double m_Y;

    G4double m_wavelength;

    TGraph *m_relativeDetectionEfficiencyInterpolator;
    TGraph *m_QEfileInterpolator;
    TGraph *m_weightAbsorbedToQEInterpolator;
    std::map<G4double, TH2D *> m_gainG2Dmap;
    std::map<G4double, TH2D *> m_gainResolutionG2Dmap;
    std::map<G4double, TH2D *> m_transitTimeG2Dmap;
    std::map<G4double, TH2D *> m_transitTimeSpreadG2Dmap;
};

class mDOMPMTResponse : public OMSimPMTResponse
{
public:
    static void init();
    static void shutdown();
    static mDOMPMTResponse &getInstance();

    std::vector<G4double> getScannedWavelengths();
    mDOMPMTResponse();
    ~mDOMPMTResponse(){};
    mDOMPMTResponse(const mDOMPMTResponse &) = delete;
    mDOMPMTResponse &operator=(const mDOMPMTResponse &) = delete;
};
inline mDOMPMTResponse* g_mDOMPMTResponse= nullptr;


class Gen1PMTResponse : public OMSimPMTResponse
{
public:
    static void init();
    static void shutdown();
    static Gen1PMTResponse &getInstance();
    std::vector<G4double> getScannedWavelengths();
    Gen1PMTResponse();
    ~Gen1PMTResponse(){};
    Gen1PMTResponse(const Gen1PMTResponse &) = delete;
    Gen1PMTResponse &operator=(const Gen1PMTResponse &) = delete;
};
inline Gen1PMTResponse* g_Gen1PMTResponse = nullptr;


class DEGGPMTResponse : public OMSimPMTResponse
{
public:
    static void init();
    static void shutdown();
    static DEGGPMTResponse &getInstance();

    std::vector<G4double> getScannedWavelengths();
    DEGGPMTResponse();
    ~DEGGPMTResponse(){};
    DEGGPMTResponse(const DEGGPMTResponse &) = delete;
    DEGGPMTResponse &operator=(const DEGGPMTResponse &) = delete;
};
inline DEGGPMTResponse* g_DEGGPMTResponse = nullptr;


class LOMHamamatsuResponse : public OMSimPMTResponse
{
public:
    static void init();
    static void shutdown();
    static LOMHamamatsuResponse &getInstance();

    std::vector<G4double> getScannedWavelengths();
    LOMHamamatsuResponse();
    ~LOMHamamatsuResponse(){};
    LOMHamamatsuResponse(const LOMHamamatsuResponse &) = delete;
    LOMHamamatsuResponse &operator=(const LOMHamamatsuResponse &) = delete;
};
inline LOMHamamatsuResponse* g_LOMHamamatsuResponse = nullptr;



class LOMNNVTResponse : public OMSimPMTResponse
{
public:
    static void init();
    static void shutdown();
    static LOMNNVTResponse &getInstance();

    std::vector<G4double> getScannedWavelengths();
    LOMNNVTResponse();
    ~LOMNNVTResponse(){};
    LOMNNVTResponse(const LOMNNVTResponse &) = delete;
    LOMNNVTResponse &operator=(const LOMNNVTResponse &) = delete;
};
inline LOMNNVTResponse* g_LOMNNVTResponse = nullptr;

class NoResponse : public OMSimPMTResponse
{
public:
    NoResponse();
    ~NoResponse(){};
    NoResponse(const NoResponse &) = delete;
    NoResponse &operator=(const NoResponse &) = delete;
    static void init();
    static void shutdown();
    static NoResponse &getInstance();

    PMTPulse processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength) override;

private:
    std::vector<G4double> getScannedWavelengths();
};

inline NoResponse* g_NoResponse = nullptr;