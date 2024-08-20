/**
 *  @file OMSimPMTResponse.hh
 *  @brief Simulation of PMT response.
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
    virtual bool passQE(G4double pWavelength);

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

    bool m_ScansInterpolatorsAvailable = false;
    bool m_QEWeightInterpolatorAvailable = false;
    bool m_CEWeightInterpolatorAvailable = false;
    double m_X;
    double m_Y;

    G4double mWavelength;

    TGraph *m_RelativeDetectionEfficiencyInterpolator;
    TGraph *m_QEInterpolator;
    std::map<G4double, TH2D *> m_GainG2Dmap;
    std::map<G4double, TH2D *> m_GainResolutionG2Dmap;
    std::map<G4double, TH2D *> m_TransitTimeG2Dmap;
    std::map<G4double, TH2D *> m_TransitTimeSpreadG2Dmap;
};

class mDOMPMTResponse : public OMSimPMTResponse
{
public:
    static mDOMPMTResponse &getInstance()
    { // Meyers singleton
        static mDOMPMTResponse instance;
        return instance;
    }
    std::vector<G4double> getScannedWavelengths();
    mDOMPMTResponse();
    ~mDOMPMTResponse(){};
    mDOMPMTResponse(const mDOMPMTResponse &) = delete;
    mDOMPMTResponse &operator=(const mDOMPMTResponse &) = delete;
};

class Gen1PMTResponse : public OMSimPMTResponse
{
public:
    static Gen1PMTResponse &getInstance()
    { // Meyers singleton
        static Gen1PMTResponse instance;
        return instance;
    }
    std::vector<G4double> getScannedWavelengths();
    Gen1PMTResponse();
    ~Gen1PMTResponse(){};
    Gen1PMTResponse(const Gen1PMTResponse &) = delete;
    Gen1PMTResponse &operator=(const Gen1PMTResponse &) = delete;
};

class DEGGPMTResponse : public OMSimPMTResponse
{
public:
    static DEGGPMTResponse &getInstance()
    { // Meyers singleton
        static DEGGPMTResponse instance;
        return instance;
    }
    std::vector<G4double> getScannedWavelengths();
    DEGGPMTResponse();
    ~DEGGPMTResponse(){};
    DEGGPMTResponse(const DEGGPMTResponse &) = delete;
    DEGGPMTResponse &operator=(const DEGGPMTResponse &) = delete;
};

class LOMHamamatsuResponse : public OMSimPMTResponse
{
public:
    static LOMHamamatsuResponse &getInstance()
    { // Meyers singleton
        static LOMHamamatsuResponse instance;
        return instance;
    }
    std::vector<G4double> getScannedWavelengths();
    LOMHamamatsuResponse();
    ~LOMHamamatsuResponse(){};
    LOMHamamatsuResponse(const LOMHamamatsuResponse &) = delete;
    LOMHamamatsuResponse &operator=(const LOMHamamatsuResponse &) = delete;
};

class LOMNNVTResponse : public OMSimPMTResponse
{
public:
    static LOMNNVTResponse &getInstance()
    { // Meyers singleton
        static LOMNNVTResponse instance;
        return instance;
    }
    std::vector<G4double> getScannedWavelengths();
    LOMNNVTResponse();
    ~LOMNNVTResponse(){};
    LOMNNVTResponse(const LOMNNVTResponse &) = delete;
    LOMNNVTResponse &operator=(const LOMNNVTResponse &) = delete;
};

class NoResponse : public OMSimPMTResponse
{
public:
    NoResponse();
    ~NoResponse(){};
    NoResponse(const NoResponse &) = delete;
    NoResponse &operator=(const NoResponse &) = delete;

    static NoResponse &getInstance()
    { // Meyers singleton
        static NoResponse instance;
        return instance;
    }
    PMTPulse processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength) override;
    bool passQE(G4double pWavelength) override;

private:
    std::vector<G4double> getScannedWavelengths();
};

#endif
//
