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

    virtual PMTPulse processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength);
    virtual bool passQE(G4double pWavelength);

protected:
    
    double mX;
    double mY;

    G4double mWavelength;

    TGraph *mRelativeDetectionEfficiencyInterp;
    TGraph *mQEInterp;
    std::map<G4double, TH2D *> mGainG2Dmap;
    std::map<G4double, TH2D *> mGainResolutionG2Dmap;
    std::map<G4double, TH2D *> mTransitTimeG2Dmap;
    std::map<G4double, TH2D *> mTransitTimeSpreadG2Dmap;

    void configureQEinterpolator(const char* pFileName);
    TH2D *createHistogramFromData(const std::string &pFilePath, const char *pTH2DName);
    G4double getCharge(G4double pWavelengthKey);
    G4double getCharge(G4double pWavelengthKey1, G4double pWavelengthKey2);
    G4double getTransitTime(G4double pWavelengthKey);
    G4double getTransitTime(G4double pWavelengthKey1, G4double pWavelengthKey2);
    PMTPulse getPulseFromInterpolation(G4double pWavelengthKey1, G4double pWavelengthKey2);
    PMTPulse getPulseFromKey(G4double pWavelengthKey);
    G4double wavelengthInterpolatedValue(std::map<G4double, TH2D *> pMap, G4double pWavelengthKey1, G4double pWavelengthKey2);

    virtual std::vector<G4double> getScannedWavelengths() = 0;
    virtual bool scansAvailable() = 0;
    OMSimPMTResponse(){};
    virtual ~OMSimPMTResponse(){};
    // OMSimPMTResponse(const OMSimPMTResponse &) = delete;
    // OMSimPMTResponse &operator=(const OMSimPMTResponse &) = delete;
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
    bool scansAvailable(){return true;};
    mDOMPMTResponse();
    ~mDOMPMTResponse();
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
    bool scansAvailable(){return false;};
    Gen1PMTResponse();
    ~Gen1PMTResponse();
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
    bool scansAvailable(){return false;};
    DEGGPMTResponse();
    ~DEGGPMTResponse();
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
    bool scansAvailable(){return false;};
    LOMHamamatsuResponse();
    ~LOMHamamatsuResponse();
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
    bool scansAvailable(){return false;};
    LOMNNVTResponse();
    ~LOMNNVTResponse();
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
    bool scansAvailable(){return false;};

};

#endif
//
