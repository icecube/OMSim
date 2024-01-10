
/**
 *  @todo Add PMT data of all PMT types
 */
#include "OMSimPMTResponse.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimLogger.hh"
#include "OMSimInputData.hh"

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                  Base Abstract Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 * @brief Create a histogram from provided data.
 *
 * Loads the data from a given path and constructs a histogram based on the data.
 *
 * @param pFilePath Path to the data file.
 * @param pTH2DName Name of the histogram.
 * @return Pointer to the created histogram.
 */
TH2D *OMSimPMTResponse::createHistogramFromData(const std::string &pFilePath, const char *pTH2DName)
{
    // Load the data
    std::vector<std::vector<double>> lData = InputDataManager::loadtxt(pFilePath, true, 0, '\t');

    // Deduce the number of bins and the bin widths
    double binWidthX, binWidthY;
    for (size_t i = 1; i < lData[0].size(); i++)
    {
        if (lData[0][i] - lData[0][i - 1] > 0)
        {
            binWidthX = lData[0][i] - lData[0][i - 1];
            break;
        }
    }
    for (size_t i = 1; i < lData[1].size(); i++)
    {
        if (lData[1][i] - lData[1][i - 1] > 0)
        {
            binWidthY = lData[1][i] - lData[1][i - 1];
            break;
        }
    }
    double minX = *std::min_element(lData[0].begin(), lData[0].end()) - binWidthX / 2.0;
    double maxX = *std::max_element(lData[0].begin(), lData[0].end()) + binWidthX / 2.0;
    double minY = *std::min_element(lData[1].begin(), lData[1].end()) - binWidthY / 2.0;
    double maxY = *std::max_element(lData[1].begin(), lData[1].end()) + binWidthY / 2.0;
    int nBinsX = (int)((maxX - minX) / binWidthX);
    int nBinsY = (int)((maxY - minY) / binWidthY);

    // Create histogram
    TH2D *h = new TH2D(pTH2DName, "title", nBinsX, minX, maxX, nBinsY, minY, maxY);

    // Fill the histogram
    for (size_t i = 0; i < lData[0].size(); i++)
    {
        h->Fill(lData[0][i], lData[1][i], lData[2][i]);
    }

    return h;
}

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
G4double OMSimPMTResponse::getCharge(G4double pWavelengthKey)
{

    G4double lMeanPE = mGainG2Dmap[pWavelengthKey]->Interpolate(mX, mY);
    G4double lSPEResolution = mGainResolutionG2Dmap[pWavelengthKey]->Interpolate(mX, mY);

    double lToReturn = -1;
    double lCounter = 0;
    while (lToReturn < 0)
    {
        lToReturn = G4RandGauss::shoot(lMeanPE, lSPEResolution);
        lCounter++;
        if (lCounter > 10)
            return 0;
    }

    return lToReturn;
}

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
G4double OMSimPMTResponse::getCharge(G4double pWavelength1, G4double pWavelength2)
{

    G4double lMeanPE = wavelengthInterpolatedValue(mGainG2Dmap, pWavelength1, pWavelength2);
    G4double lSPEResolution = wavelengthInterpolatedValue(mGainResolutionG2Dmap, pWavelength1, pWavelength2);

    double lToReturn = -1;
    double lCounter = 0;
    while (lToReturn < 0)
    {
        lToReturn = G4RandGauss::shoot(lMeanPE, lSPEResolution);
        lCounter++;
        if (lCounter > 10)
            return 0;
    }

    return lToReturn;
}

/**
 * @brief Retrieve the transit time from measurements for a given wavelength.
 *
 * This method returns the transit time sampled from a Gaussian distribution
 * with the mean transit time and transit time spread for the provided wavelength key.
 *
 * @param pWavelengthKey The key for selecting a scan from the available measurements.
 * @return G4double The transit time in ns.
 */
G4double OMSimPMTResponse::getTransitTime(G4double pWavelengthKey)
{

    G4double lMeanTransitTime = mTransitTimeG2Dmap[pWavelengthKey]->Interpolate(mX, mY) * ns;
    G4double lTTS = mTransitTimeSpreadG2Dmap[pWavelengthKey]->Interpolate(mX, mY) * ns;

    return G4RandGauss::shoot(lMeanTransitTime, lTTS);
}

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
G4double OMSimPMTResponse::getTransitTime(G4double pWavelength1, G4double pWavelength2)
{

    G4double lMeanTransitTime = wavelengthInterpolatedValue(mTransitTimeG2Dmap, pWavelength1, pWavelength2) * ns;
    G4double lTTS = wavelengthInterpolatedValue(mTransitTimeSpreadG2Dmap, pWavelength1, pWavelength2) * ns;

    return G4RandGauss::shoot(lMeanTransitTime, lTTS);
}

/**
 * Get interpolated value between two scans of different wavelenths
 * @param pMap  Map with TGraph2D scans to be used
 * @param pWavelength1  first reference wavelength for interpolation
 * @param pWavelength2  second reference wavelength for interpolation
 * @return G4double interpolated value
 */
G4double OMSimPMTResponse::wavelengthInterpolatedValue(std::map<G4double, TH2D *> pMap, G4double pWavelength1, G4double pWavelength2)
{

    G4double lValue1 = pMap[pWavelength1]->Interpolate(mX, mY);
    G4double lValue2 = pMap[pWavelength2]->Interpolate(mX, mY);
    return lValue1 + (mWavelength - pWavelength1) * (lValue2 - lValue1) / (pWavelength2 - pWavelength1);
}

/**
 * Get a finished Pulse using a key for the maps directly
 * @param pWavelengthKey  wavelength key to be used
 * @return PMTPulse Struct with transit time, charge and detection probability
 */
OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromKey(G4double pWavelengthKey)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.PE = getCharge(pWavelengthKey);
    lPulse.TransitTime = getTransitTime(pWavelengthKey);

    return lPulse;
}

/**
 * Get a finished Pulse interpolating scan results between measured wavelengths (simulated photons has a wavelength between these two)
 * @param pWavelengthKey1 first wavelength key to be used
 * @param pWavelengthKey2 second wavelength key to be used
 * @return PMTPulse Struct with transit time, charge and detection probability
 */
OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromInterpolation(G4double pWavelengthKey1, G4double pWavelengthKey2)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.PE = getCharge(pWavelengthKey1, pWavelengthKey2);
    lPulse.TransitTime = getTransitTime(pWavelengthKey1, pWavelengthKey2);
    return lPulse;
}

/**
 * @brief Initializes the QE interpolator from a data file.
 *
 * Sets up the quantum efficiency (QE) interpolator using data from a predefined path.
 *
 * @warning Ensure the data file is present at the specified path.
 *
 * @sa passQE
 */
void OMSimPMTResponse::configureQEinterpolator(const char *pFileName)
{
    mQEInterp = new TGraph(pFileName);
    mQEInterp->SetName(pFileName);
}

/**
 * @brief Checks if photon with given wavelength passes QE check.
 * @param pWavelength Photon wavelength to be checked in nm.
 * @return True if the photon passes the QE check, false otherwise.
 *
 * @sa configureQEinterpolator
 */
bool OMSimPMTResponse::passQE(G4double pWavelength)
{
    double lQE = mQEInterp->Eval(pWavelength / nm) / 100.;
    // Check against random value
    G4double rand = G4UniformRand();
    return rand < lQE;
}

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
OMSimPMTResponse::PMTPulse OMSimPMTResponse::processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.DetectionProbability = -1;
    lPulse.PE = -1;
    lPulse.TransitTime = -1;

    if (!scansAvailable())
        return lPulse;

    mX = pX / mm;
    mY = pY / mm;
    G4double lR = std::sqrt(mX * mX + mY * mY);
    mWavelength = pWavelength;

    lPulse.DetectionProbability = mRelativeDetectionEfficiencyInterp->Eval(lR);

    std::vector<G4double> lScannedWavelengths = getScannedWavelengths();

    // If wavelength matches with one of the measured ones
    if (std::find(lScannedWavelengths.begin(), lScannedWavelengths.end(), pWavelength) != lScannedWavelengths.end())
    {
        lPulse = getPulseFromKey(pWavelength);
    }
    // If wavelength smaller than scanned, return for first measured wavelength
    else if (pWavelength < lScannedWavelengths.at(0))
    {
        lPulse = getPulseFromKey(lScannedWavelengths.at(0));
    }
    // If wavelength larger than scanned, return for last measured wavelength
    else if (pWavelength > lScannedWavelengths.back())
    {
        lPulse = getPulseFromKey(lScannedWavelengths.back());
    }
    // If wavelength in range of scanned, return interpolated values
    else
    {
        // get first index of first element larger than seeked wavelength
        auto const lLowerBoundIndex = std::lower_bound(lScannedWavelengths.begin(), lScannedWavelengths.end(), pWavelength) - lScannedWavelengths.begin();

        G4double lW1 = lScannedWavelengths.at(lLowerBoundIndex - 1);
        G4double lW2 = lScannedWavelengths.at(lLowerBoundIndex);

        lPulse = getPulseFromInterpolation(lW1, lW2);
    }

    return lPulse;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                 Derived Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/*
 * %%%%%%%%%%%%%%%% mDOM %%%%%%%%%%%%%%%%
 */
mDOMPMTResponse::mDOMPMTResponse()
{
    log_info("Using mDOM PMT response");
    log_debug("Opening mDOM photocathode scans data...");
    std::string path = "../common/data/PMT_scans/";

    mRelativeDetectionEfficiencyInterp = new TGraph((path + "weightsVsR_vFit_220nm.txt").c_str());
    mRelativeDetectionEfficiencyInterp->SetName("RelativeDetectionEfficiencyWeight");

    if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut"))
        configureQEinterpolator("../common/data/PMT_scans/QuantumEfficiency.dat");

    for (const auto &lKey : getScannedWavelengths())
    {
        std::string lWv = std::to_string((int)(lKey / nm));
        mGainG2Dmap[lKey] = createHistogramFromData(path + "Gain_PE_" + lWv + ".dat", ("Gain_PE_" + lWv).c_str());
        mGainResolutionG2Dmap[lKey] = createHistogramFromData(path + "SPEresolution_" + lWv + ".dat", ("SPEresolution_" + lWv).c_str());
        mTransitTimeSpreadG2Dmap[lKey] = createHistogramFromData(path + "TransitTimeSpread_" + lWv + ".dat", ("TransitTimeSpread_" + lWv).c_str());
        mTransitTimeG2Dmap[lKey] = createHistogramFromData(path + "TransitTime_" + lWv + ".dat", ("TransitTime_" + lWv).c_str());
    }

    log_debug("Finished opening photocathode scans data...");
}

std::vector<G4double> mDOMPMTResponse::getScannedWavelengths()
{
    return {460 * nm, 480 * nm, 500 * nm, 520 * nm, 540 * nm, 560 * nm, 580 * nm, 600 * nm, 620 * nm, 640 * nm};
}

mDOMPMTResponse::~mDOMPMTResponse()
{
    delete mRelativeDetectionEfficiencyInterp;
    delete mQEInterp;
    for (const auto &lKey : getScannedWavelengths())
    {
        std::string lWv = std::to_string((int)(lKey / nm));
        delete mGainG2Dmap[lKey];
        delete mGainResolutionG2Dmap[lKey];
        delete mTransitTimeSpreadG2Dmap[lKey];
        delete mTransitTimeG2Dmap[lKey];
    }
}

/*
 * %%%%%%%%%%%%%%%% Gen1 DOM %%%%%%%%%%%%%%%%
 */
Gen1PMTResponse::Gen1PMTResponse()
{
    log_info("Using Gen1 DOM PMT response");
    if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut"))
        configureQEinterpolator("../common/data/PMT_scans/QuantumEfficiency.dat");
}

std::vector<G4double> Gen1PMTResponse::getScannedWavelengths()
{
    return {};
}

Gen1PMTResponse::~Gen1PMTResponse()
{
    delete mQEInterp;
}

/*
 * %%%%%%%%%%%%%%%% DEGG %%%%%%%%%%%%%%%%
 */
DEGGPMTResponse::DEGGPMTResponse()
{
    log_info("Using DEGG PMT response");
    if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut"))
        configureQEinterpolator("../common/data/PMT_scans/QuantumEfficiency.dat");
}

std::vector<G4double> DEGGPMTResponse::getScannedWavelengths()
{
    return {};
}

DEGGPMTResponse::~DEGGPMTResponse()
{
    delete mQEInterp;
}

/*
 * %%%%%%%%%%%%%%%% LOM Hamamatsu %%%%%%%%%%%%%%%%
 */
LOMHamamatsuResponse::LOMHamamatsuResponse()
{
    log_info("Using Hamamatsu LOM PMT response");
    if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut"))
        configureQEinterpolator("../common/data/PMT_scans/QuantumEfficiency.dat");
}

std::vector<G4double> LOMHamamatsuResponse::getScannedWavelengths()
{
    return {};
}

LOMHamamatsuResponse::~LOMHamamatsuResponse()
{
    delete mQEInterp;
}

/*
 * %%%%%%%%%%%%%%%% LOM NNVT %%%%%%%%%%%%%%%%
 */
LOMNNVTResponse::LOMNNVTResponse()
{
    log_info("Using NNVT LOM PMT response");
    if (OMSimCommandArgsTable::getInstance().get<bool>("QE_cut"))
        configureQEinterpolator("../common/data/PMT_scans/QuantumEfficiency.dat");
}

std::vector<G4double> LOMNNVTResponse::getScannedWavelengths()
{
    return {};
}

LOMNNVTResponse::~LOMNNVTResponse()
{
    delete mQEInterp;
}

/*
 * %%%%%%%%%%%%%%%% No response %%%%%%%%%%%%%%%%
 */
NoResponse::NoResponse()
{
    log_debug("OMSimResponse NoResponse initiated");
}

std::vector<G4double> NoResponse::getScannedWavelengths()
{
    return {};
}


OMSimPMTResponse::PMTPulse NoResponse::processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.DetectionProbability = -1;
    lPulse.PE = -1;
    lPulse.TransitTime = -1;
    return lPulse;
}

bool NoResponse::passQE(G4double pWavelength)
{
    return true;
}