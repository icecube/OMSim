
/**
 *  @todo Add PMT data of all PMT types
 */
#include "OMSimPMTResponse.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimLogger.hh"
#include "OMSimInputData.hh"
#include "OMSimTools.hh"
/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                  Base Abstract Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

OMSimPMTResponse::OMSimPMTResponse() : m_relativeDetectionEfficiencyInterpolator(nullptr), m_QEInterpolator(nullptr)
{
}

OMSimPMTResponse::~OMSimPMTResponse()
{
    delete m_relativeDetectionEfficiencyInterpolator;
    m_relativeDetectionEfficiencyInterpolator = nullptr;
    delete m_QEInterpolator;
    m_QEInterpolator = nullptr;

    for (const auto &pair : m_gainG2Dmap)
    {
        delete pair.second;
    }
    for (const auto &pair : m_gainResolutionG2Dmap)
    {
        delete pair.second;
    }
    for (const auto &pair : m_transitTimeG2Dmap)
    {
        delete pair.second;
    }
    for (const auto &pair : m_transitTimeSpreadG2Dmap)
    {
        delete pair.second;
    }
}

/**
 * @brief Retrieve the charge in PE from measurements for a given wavelength.
 *
 * Charge sampled from a Gaussian distribution with the mean
 * and standard deviation of the single photon electron (SPE) resolution for the
 * provided wavelength key.
 *
 * @param p_WavelengthKey The key for selecting a scan from the available measurements.
 * @return G4double The charge in PE.
 */
G4double OMSimPMTResponse::getCharge(G4double p_WavelengthKey)
{

    G4double lMeanPE = m_gainG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y);
    G4double lSPEResolution = m_gainResolutionG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y);

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
 * @param p_Wavelength1 The first wavelength key for interpolation.
 * @param p_Wavelength2 The second wavelength key for interpolation.
 * @return G4double The interpolated charge in PE.
 */
G4double OMSimPMTResponse::getCharge(G4double p_Wavelength1, G4double p_Wavelength2)
{

    G4double lMeanPE = wavelengthInterpolatedValue(m_gainG2Dmap, p_Wavelength1, p_Wavelength2);
    G4double lSPEResolution = wavelengthInterpolatedValue(m_gainResolutionG2Dmap, p_Wavelength1, p_Wavelength2);

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
 * @param p_WavelengthKey The key for selecting a scan from the available measurements.
 * @return G4double The transit time in ns.
 */
G4double OMSimPMTResponse::getTransitTime(G4double p_WavelengthKey)
{

    G4double lMeanTransitTime = m_transitTimeG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y) * ns;
    G4double lTTS = m_transitTimeSpreadG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y) * ns;

    return G4RandGauss::shoot(lMeanTransitTime, lTTS);
}

/**
 * @brief Retrieve the interpolated transit time between two given wavelengths.
 *
 * This method interpolates the transit time between the two provided wavelengths
 * and then returns the time sampled from a Gaussian distribution with the mean
 * interpolated transit time and interpolated transit time spread.
 *
 * @param p_Wavelength1 The first wavelength key for interpolation.
 * @param p_Wavelength2 The second wavelength key for interpolation.
 * @return G4double The interpolated transit time in ns.
 */
G4double OMSimPMTResponse::getTransitTime(G4double p_Wavelength1, G4double p_Wavelength2)
{

    G4double lMeanTransitTime = wavelengthInterpolatedValue(m_transitTimeG2Dmap, p_Wavelength1, p_Wavelength2) * ns;
    G4double lTTS = wavelengthInterpolatedValue(m_transitTimeSpreadG2Dmap, p_Wavelength1, p_Wavelength2) * ns;

    return G4RandGauss::shoot(lMeanTransitTime, lTTS);
}

/**
 * Get interpolated value between two scans of different wavelenths
 * @param p_Map  Map with TGraph2D scans to be used
 * @param p_Wavelength1  first reference wavelength for interpolation
 * @param p_Wavelength2  second reference wavelength for interpolation
 * @return G4double interpolated value
 */
G4double OMSimPMTResponse::wavelengthInterpolatedValue(std::map<G4double, TH2D *> p_Map, G4double p_Wavelength1, G4double p_Wavelength2)
{

    G4double lValue1 = p_Map[p_Wavelength1]->Interpolate(m_X, m_Y);
    G4double lValue2 = p_Map[p_Wavelength2]->Interpolate(m_X, m_Y);
    return lValue1 + (m_wavelength - p_Wavelength1) * (lValue2 - lValue1) / (p_Wavelength2 - p_Wavelength1);
}

/**
 * Get a finished Pulse using a key for the maps directly
 * @param p_WavelengthKey  wavelength key to be used
 * @return PMTPulse Struct with transit time, charge and detection probability
 */
OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromKey(G4double p_WavelengthKey)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.PE = getCharge(p_WavelengthKey);
    lPulse.transitTime = getTransitTime(p_WavelengthKey);

    return lPulse;
}

/**
 * Get a finished Pulse interpolating scan results between measured wavelengths (simulated photons has a wavelength between these two)
 * @param p_WavelengthKey1 first wavelength key to be used
 * @param p_WavelengthKey2 second wavelength key to be used
 * @return PMTPulse Struct with transit time, charge and detection probability
 */
OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromInterpolation(G4double p_WavelengthKey1, G4double p_WavelengthKey2)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.PE = getCharge(p_WavelengthKey1, p_WavelengthKey2);
    lPulse.transitTime = getTransitTime(p_WavelengthKey1, p_WavelengthKey2);
    return lPulse;
}

/**
 * @brief Configures the QE weight interpolator for PMT response.
 *
 * @param p_FileNameAbsorbedFraction File name containing absorbed fraction data
 * @param p_FileNameTargetQE File name containing target QE data.
 *
 * @details
 * Loads QE and absorbed fraction data, calculates weights based on the ratio
 * of target QE to absorbed fraction, and creates an interpolator for these weights.
 * Handles out-of-range values and extends the wavelength range slightly.
 */
void OMSimPMTResponse::configureQEweightInterpolator(const std::string &p_FileNameAbsorbedFraction, const std::string &p_FileNameTargetQE)
{
    log_trace("Creating QE weight interpolator...");
    // Load QE data
    auto lQEData = Tools::loadtxt(p_FileNameTargetQE.c_str(), true, 0, '\t');
    auto lAbsorbedData = Tools::loadtxt(p_FileNameAbsorbedFraction.c_str(), true, 0, '\t');

    std::vector<double> lWavelengths = lQEData[0];
    std::vector<double> lQEs = lQEData[1];
    std::vector<double> lAbsorbedFraction_wavelength = lAbsorbedData[0];
    std::vector<double> lAbsorbedFraction = lAbsorbedData[1];

    auto lAbsorbedFractionInterpolator = Tools::create1dInterpolator(lAbsorbedFraction_wavelength, lAbsorbedFraction, "absorbed_fraction_interpolator");

    // Calculate weights
    std::vector<double> lWeights;
    double lMaxWavelength = *std::max_element(lAbsorbedFraction_wavelength.begin(), lAbsorbedFraction_wavelength.end());
    double lMinWavelength = *std::min_element(lAbsorbedFraction_wavelength.begin(), lAbsorbedFraction_wavelength.end());

    for (size_t i = 0; i < lWavelengths.size(); ++i)
    {
        double lWavelength = lWavelengths[i];
        if (lWavelength >= lMinWavelength && lWavelength <= lMaxWavelength)
        {
            double lInterpolatedFraction = lAbsorbedFractionInterpolator->Eval(lWavelength);
            if (lQEs[i] > lInterpolatedFraction)
            {
                log_error("Requested QE value {} is larger than possible from optical properties of PMT ({})!", lQEs[i], lInterpolatedFraction);
                throw std::invalid_argument("Requested QE value larger than possible from optical properties of PMT!");
            }
            double lToAppend = (lInterpolatedFraction==0) ? 0: lQEs[i] / lInterpolatedFraction;
            lWeights.push_back(lToAppend);
        }
        else
        {
            log_error("Requested QE value at wavelength {} outside wavelength range [{},{}]!",
                      lWavelength, lMinWavelength, lMaxWavelength);
            throw std::invalid_argument("Requested QE value outside wavelength range!");
        }
    }

    lMaxWavelength = *std::max_element(lWavelengths.begin(), lWavelengths.end());
    lMinWavelength = *std::min_element(lWavelengths.begin(), lWavelengths.end());

    // Add points above the maximum
    for (int i = 1; i <= 10; ++i)
    {
        double newWavelength = lMaxWavelength + i * (lMaxWavelength - lMinWavelength) / 10;
        lWavelengths.push_back(newWavelength);
        lWeights.push_back(0);
    }

    // Add points below the minimum
    for (int i = 1; i <= 10; ++i)
    {
        double newWavelength = lMinWavelength - i * lMinWavelength / 10;
        lWavelengths.push_back(std::max(0.0, newWavelength));
        lWeights.push_back(0);
    }

    Tools::sortVectorByReference(lWavelengths, lWeights);

    m_QEInterpolator = Tools::create1dInterpolator(lWavelengths, lWeights, p_FileNameTargetQE);
    delete lAbsorbedFractionInterpolator;

    m_QEWeightInterpolatorAvailable = true;
}

/**
 * @brief Configures the CE weight interpolator for PMT response.
 * @param p_FileName File name containing CE weights
 */
void OMSimPMTResponse::configureCEweightInterpolator(const std::string &p_FileName)
{
    log_trace("Creating CE weight interpolator...");
    m_relativeDetectionEfficiencyInterpolator = Tools::create1dInterpolator(p_FileName.c_str());
    m_CEWeightInterpolatorAvailable = true;
}

void OMSimPMTResponse::configureScansInterpolators(const std::string &p_PathToFiles)
{
    log_trace("Creating TH2D interpolators from scan data...");

    for (const auto &lKey : getScannedWavelengths())
    {
        std::string lWv = std::to_string((int)(lKey / nm));
        m_gainG2Dmap[lKey] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "Gain_PE_" + lWv + ".dat");
        m_gainResolutionG2Dmap[lKey] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "SPEresolution_" + lWv + ".dat");
        m_transitTimeSpreadG2Dmap[lKey] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "TransitTimeSpread_" + lWv + ".dat");
        m_transitTimeG2Dmap[lKey] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "TransitTime_" + lWv + ".dat");
    }
    m_scansInterpolatorsAvailable = true;
    log_trace("Finished opening photocathode scans data...");
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
    double lQE = m_QEInterpolator->Eval(pWavelength / nm) / 100.;
    // Check against random value
    G4double lRand = G4UniformRand();
    return lRand < lQE;
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
    m_X = pX / mm;
    m_Y = pY / mm;
    G4double lR = std::sqrt(m_X * m_X + m_Y * m_Y);
    m_wavelength = pWavelength;
    double lQEweight = (m_QEWeightInterpolatorAvailable) ? m_QEInterpolator->Eval(pWavelength / nm) : 1;
    double lCEweight = (m_CEWeightInterpolatorAvailable) ? m_relativeDetectionEfficiencyInterpolator->Eval(lR) : 1;

    lPulse.detectionProbability = lQEweight * lCEweight;
    lPulse.PE = -1;
    lPulse.transitTime = -1;

    if (!m_scansInterpolatorsAvailable)
        return lPulse;

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
    lPulse.detectionProbability = lQEweight;

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

    std::string lPath = "../common/data/PMTs/measurement_matching_data/";

    configureCEweightInterpolator(lPath + "CE_weight/240813_mDOM_Hamamatsu_CAT.dat");

    // Get QE file from arguments or use default file
    std::string lQEFileName = OMSimCommandArgsTable::getInstance().get<std::string>("QE_file");
    if (lQEFileName == "default")
        lQEFileName = lPath + "QE/mDOM_Hamamatsu_R15458_mean_QE.dat";

    configureQEweightInterpolator(lPath + "QE/mDOM_Hamamatsu_R15458_CAT_intrinsic_QE.dat",
                                  lQEFileName);

    configureScansInterpolators("../common/data/PMTs/measurement_matching_data/scans/R15458/");
}

std::vector<G4double> mDOMPMTResponse::getScannedWavelengths()
{
    return {460 * nm, 480 * nm, 500 * nm, 520 * nm, 540 * nm, 560 * nm, 580 * nm, 600 * nm, 620 * nm, 640 * nm};
}

/*
 * %%%%%%%%%%%%%%%% Gen1 DOM %%%%%%%%%%%%%%%%
 */
Gen1PMTResponse::Gen1PMTResponse()
{
    log_info("Using Gen1 DOM PMT response");
}

std::vector<G4double> Gen1PMTResponse::getScannedWavelengths()
{
    return {};
}

/*
 * %%%%%%%%%%%%%%%% DEGG %%%%%%%%%%%%%%%%
 */
DEGGPMTResponse::DEGGPMTResponse()
{
    log_info("Using DEGG PMT response");
}

std::vector<G4double> DEGGPMTResponse::getScannedWavelengths()
{
    return {};
}

/*
 * %%%%%%%%%%%%%%%% LOM Hamamatsu %%%%%%%%%%%%%%%%
 */
LOMHamamatsuResponse::LOMHamamatsuResponse()
{
    log_info("Using Hamamatsu LOM PMT response");
}

std::vector<G4double> LOMHamamatsuResponse::getScannedWavelengths()
{
    return {};
}

/*
 * %%%%%%%%%%%%%%%% LOM NNVT %%%%%%%%%%%%%%%%
 */
LOMNNVTResponse::LOMNNVTResponse()
{
    log_info("Using NNVT LOM PMT response");
}

std::vector<G4double> LOMNNVTResponse::getScannedWavelengths()
{
    return {};
}

/*
 * %%%%%%%%%%%%%%%% No response %%%%%%%%%%%%%%%%
 */
NoResponse::NoResponse()
{
    log_trace("OMSimResponse NoResponse initiated");
}

std::vector<G4double> NoResponse::getScannedWavelengths()
{
    return {};
}

OMSimPMTResponse::PMTPulse NoResponse::processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength)
{

    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.detectionProbability = -1;
    lPulse.PE = -1;
    lPulse.transitTime = -1;
    return lPulse;
}

bool NoResponse::passQE(G4double pWavelength)
{
    return true;
}