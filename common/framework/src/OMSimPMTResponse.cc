
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

OMSimPMTResponse::OMSimPMTResponse() : m_relativeDetectionEfficiencyInterpolator(nullptr), m_QEfileInterpolator(nullptr), m_weightAbsorbedToQEInterpolator(nullptr)
{
    m_simplePMT = OMSimCommandArgsTable::getInstance().get<bool>("simple_PMT");
}

OMSimPMTResponse::~OMSimPMTResponse()
{
    log_trace("Destructing OMSimPMTResponse");
    delete m_relativeDetectionEfficiencyInterpolator;
    m_relativeDetectionEfficiencyInterpolator = nullptr;

    delete m_weightAbsorbedToQEInterpolator;
    m_weightAbsorbedToQEInterpolator = nullptr;

    delete m_QEfileInterpolator;
    m_QEfileInterpolator = nullptr;

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

    G4double meanPE = m_gainG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y);
    G4double SPEresolution = m_gainResolutionG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y);

    double toReturn = -1;
    double counter = 0;
    while (toReturn < 0)
    {
        toReturn = G4RandGauss::shoot(meanPE, SPEresolution);
        counter++;
        if (counter > 10)
            return 0;
    }

    return toReturn;
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

    G4double meanPE = wavelengthInterpolatedValue(m_gainG2Dmap, p_Wavelength1, p_Wavelength2);
    G4double SPEresolution = wavelengthInterpolatedValue(m_gainResolutionG2Dmap, p_Wavelength1, p_Wavelength2);

    double toReturn = -1;
    double counter = 0;
    while (toReturn < 0)
    {
        toReturn = G4RandGauss::shoot(meanPE, SPEresolution);
        counter++;
        if (counter > 10)
            return 0;
    }

    return toReturn;
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

    G4double meanTransitTime = m_transitTimeG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y) * ns;
    G4double TTS = m_transitTimeSpreadG2Dmap[p_WavelengthKey]->Interpolate(m_X, m_Y) * ns;

    return G4RandGauss::shoot(meanTransitTime, TTS);
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

    G4double meanTransitTime = wavelengthInterpolatedValue(m_transitTimeG2Dmap, p_Wavelength1, p_Wavelength2) * ns;
    G4double TTS = wavelengthInterpolatedValue(m_transitTimeSpreadG2Dmap, p_Wavelength1, p_Wavelength2) * ns;

    return G4RandGauss::shoot(meanTransitTime, TTS);
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

    G4double value1 = p_Map[p_Wavelength1]->Interpolate(m_X, m_Y);
    G4double value2 = p_Map[p_Wavelength2]->Interpolate(m_X, m_Y);
    return value1 + (m_wavelength - p_Wavelength1) * (value2 - value1) / (p_Wavelength2 - p_Wavelength1);
}

/**
 * Get a finished Pulse using a key for the maps directly
 * @param p_WavelengthKey  wavelength key to be used
 * @return PMTPulse Struct with transit time, charge and detection probability
 */
OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromKey(G4double p_WavelengthKey)
{
    OMSimPMTResponse::PMTPulse pulse;
    pulse.PE = getCharge(p_WavelengthKey);
    pulse.transitTime = getTransitTime(p_WavelengthKey);

    return pulse;
}

/**
 * Get a finished Pulse interpolating scan results between measured wavelengths (simulated photons has a wavelength between these two)
 * @param p_WavelengthKey1 first wavelength key to be used
 * @param p_WavelengthKey2 second wavelength key to be used
 * @return PMTPulse Struct with transit time, charge and detection probability
 */
OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromInterpolation(G4double p_WavelengthKey1, G4double p_WavelengthKey2)
{
    OMSimPMTResponse::PMTPulse pulse;
    pulse.PE = getCharge(p_WavelengthKey1, p_WavelengthKey2);
    pulse.transitTime = getTransitTime(p_WavelengthKey1, p_WavelengthKey2);
    return pulse;
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
    auto dataQE = Tools::loadtxt(p_FileNameTargetQE.c_str(), true, 0, '\t');
    auto absorbedData = Tools::loadtxt(p_FileNameAbsorbedFraction.c_str(), true, 0, '\t');

    std::vector<double> wavelengths = dataQE[0];
    std::vector<double> theQEs = dataQE[1];
    std::vector<double> wavelengthAbsorbedFraction = absorbedData[0];
    std::vector<double> absorbedFraction = absorbedData[1];

    auto absorbedFractionInterpolator = Tools::create1dInterpolator(wavelengthAbsorbedFraction, absorbedFraction, "absorbed_fraction_interpolator");

    // Calculate weights
    std::vector<double> weights;
    double maxWavelength = *std::max_element(wavelengthAbsorbedFraction.begin(), wavelengthAbsorbedFraction.end());
    double minWavelength = *std::min_element(wavelengthAbsorbedFraction.begin(), wavelengthAbsorbedFraction.end());

    for (size_t i = 0; i < wavelengths.size(); ++i)
    {
        double wavelength = wavelengths[i];
        if (wavelength >= minWavelength && wavelength <= maxWavelength)
        {
            double lInterpolatedFraction = absorbedFractionInterpolator->Eval(wavelength);
            if (theQEs[i] > lInterpolatedFraction)
            {
                log_error("Requested QE value {} is larger than possible from optical properties of PMT ({})!", theQEs[i], lInterpolatedFraction);
                throw std::invalid_argument("Requested QE value larger than possible from optical properties of PMT!");
            }
            double lToAppend = (lInterpolatedFraction == 0) ? 0 : theQEs[i] / lInterpolatedFraction;
            weights.push_back(lToAppend);
        }
        else
        {
            log_error("Requested QE value at wavelength {} outside wavelength range [{},{}]!",
                      wavelength, minWavelength, maxWavelength);
            throw std::invalid_argument("Requested QE value outside wavelength range!");
        }
    }

    maxWavelength = *std::max_element(wavelengths.begin(), wavelengths.end());
    minWavelength = *std::min_element(wavelengths.begin(), wavelengths.end());

    // Add points above the maximum
    for (int i = 1; i <= 10; ++i)
    {
        double newWavelength = maxWavelength + i * (maxWavelength - minWavelength) / 10;
        wavelengths.push_back(newWavelength);
        weights.push_back(0);
        theQEs.push_back(0);
    }

    // Add points below the minimum
    for (int i = 1; i <= 10; ++i)
    {
        double newWavelength = minWavelength - i * minWavelength / 10;
        wavelengths.push_back(std::max(0.0, newWavelength));
        weights.push_back(0);
        theQEs.push_back(0);
    }

    Tools::sortVectorByReference(wavelengths, weights);

    m_weightAbsorbedToQEInterpolator = Tools::create1dInterpolator(wavelengths, weights, "weights QE interpolator");
    m_QEfileInterpolator = Tools::create1dInterpolator(wavelengths, theQEs, p_FileNameTargetQE);

    delete absorbedFractionInterpolator;

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
 * @brief Process a hit on the photocathode into a PMT pulse.
 *
 * Transform a hit based on its location and photon wavelength to a PMT pulse,
 * considering the transit time, charge, and detection probability.
 *
 * @param p_x  x position on photocathode
 * @param p_y  y position on photocathode
 * @param p_wavelength  Wavelength of the hitting photon
 * @return PMTPulse representing the processed hit.
 */
OMSimPMTResponse::PMTPulse OMSimPMTResponse::processPhotocathodeHit(G4double p_x, G4double p_y, G4double p_wavelength)
{
    OMSimPMTResponse::PMTPulse pulse;
    m_X = p_x / mm;
    m_Y = p_y / mm;
    G4double r = std::sqrt(m_X * m_X + m_Y * m_Y);
    m_wavelength = p_wavelength;
    double weightQE, weightCE;

    if (m_simplePMT)
    {
        weightQE = (m_QEWeightInterpolatorAvailable) ? m_QEfileInterpolator->Eval(p_wavelength / nm) : 1;
        weightCE = 1;
    }
    else
    {
        weightQE = (m_QEWeightInterpolatorAvailable) ? m_weightAbsorbedToQEInterpolator->Eval(p_wavelength / nm) : 1;
        weightCE = (m_CEWeightInterpolatorAvailable) ? m_relativeDetectionEfficiencyInterpolator->Eval(r) : 1;
    }

    pulse.detectionProbability = weightQE * weightCE;
    pulse.PE = -1;
    pulse.transitTime = -1;

    if (!m_scansInterpolatorsAvailable)
        return pulse;

    std::vector<G4double> scannedWavelengths = getScannedWavelengths();

    // If wavelength matches with one of the measured ones
    if (std::find(scannedWavelengths.begin(), scannedWavelengths.end(), p_wavelength) != scannedWavelengths.end())
    {
        pulse = getPulseFromKey(p_wavelength);
    }
    // If wavelength smaller than scanned, return for first measured wavelength
    else if (p_wavelength < scannedWavelengths.at(0))
    {
        pulse = getPulseFromKey(scannedWavelengths.at(0));
    }
    // If wavelength larger than scanned, return for last measured wavelength
    else if (p_wavelength > scannedWavelengths.back())
    {
        pulse = getPulseFromKey(scannedWavelengths.back());
    }
    // If wavelength in range of scanned, return interpolated values
    else
    {
        // get first index of first element larger than seeked wavelength
        auto const lowerBoundIndex = std::lower_bound(scannedWavelengths.begin(), scannedWavelengths.end(), p_wavelength) - scannedWavelengths.begin();

        G4double lW1 = scannedWavelengths.at(lowerBoundIndex - 1);
        G4double lW2 = scannedWavelengths.at(lowerBoundIndex);

        pulse = getPulseFromInterpolation(lW1, lW2);
    }
    pulse.detectionProbability = weightQE;

    return pulse;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                 Derived Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/*
 * %%%%%%%%%%%%%%%% mDOM %%%%%%%%%%%%%%%%
 */
void mDOMPMTResponse::init()
{
    if (!g_mDOMPMTResponse)
        g_mDOMPMTResponse = new mDOMPMTResponse();
}

void mDOMPMTResponse::shutdown()
{
    delete g_mDOMPMTResponse;
    g_mDOMPMTResponse = nullptr;
}
mDOMPMTResponse &mDOMPMTResponse::getInstance()
{
    if (!g_mDOMPMTResponse)
        Tools::throwError("mDOMPMTResponse accessed before initialization or after shutdown!");
    return *g_mDOMPMTResponse;
}

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
void Gen1PMTResponse::init()
{
    if (!g_Gen1PMTResponse)
        g_Gen1PMTResponse = new Gen1PMTResponse();
}

void Gen1PMTResponse::shutdown()
{
    delete g_Gen1PMTResponse;
    g_Gen1PMTResponse = nullptr;
}
Gen1PMTResponse &Gen1PMTResponse::getInstance()
{
    if (!g_Gen1PMTResponse)
        Tools::throwError("Gen1PMTResponse accessed before initialization or after shutdown!");
    return *g_Gen1PMTResponse;
}
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
void DEGGPMTResponse::init()
{
    if (!g_DEGGPMTResponse)
        g_DEGGPMTResponse = new DEGGPMTResponse();
}

void DEGGPMTResponse::shutdown()
{
    delete g_DEGGPMTResponse;
    g_DEGGPMTResponse = nullptr;
}
DEGGPMTResponse &DEGGPMTResponse::getInstance()
{
    if (!g_DEGGPMTResponse)
        Tools::throwError("DEGGPMTResponse accessed before initialization or after shutdown!");
    return *g_DEGGPMTResponse;
}
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

void LOMHamamatsuResponse::init()
{
    if (!g_LOMHamamatsuResponse)
        g_LOMHamamatsuResponse = new LOMHamamatsuResponse();
}

void LOMHamamatsuResponse::shutdown()
{
    delete g_LOMHamamatsuResponse;
    g_LOMHamamatsuResponse = nullptr;
}
LOMHamamatsuResponse &LOMHamamatsuResponse::getInstance()
{
    if (!g_LOMHamamatsuResponse)
        Tools::throwError("LOMNNVTResponse accessed before initialization or after shutdown!");
    return *g_LOMHamamatsuResponse;
}
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

void LOMNNVTResponse::init()
{
    if (!g_LOMNNVTResponse)
        g_LOMNNVTResponse = new LOMNNVTResponse();
}

void LOMNNVTResponse::shutdown()
{
    delete g_LOMNNVTResponse;
    g_LOMNNVTResponse = nullptr;
}
LOMNNVTResponse &LOMNNVTResponse::getInstance()
{
    if (!g_LOMNNVTResponse)
        Tools::throwError("LOMNNVTResponse accessed before initialization or after shutdown!");
    return *g_LOMNNVTResponse;
}

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
void NoResponse::init()
{
    if (!g_NoResponse)
        g_NoResponse = new NoResponse();
}

void NoResponse::shutdown()
{
    delete g_NoResponse;
    g_NoResponse = nullptr;
}
NoResponse &NoResponse::getInstance()
{
    if (!g_NoResponse)
        Tools::throwError("NoResponse accessed before initialization or after shutdown!");
    return *g_NoResponse;
}

NoResponse::NoResponse()
{
    log_trace("OMSimResponse NoResponse initiated");
}

std::vector<G4double> NoResponse::getScannedWavelengths()
{
    return {};
}

OMSimPMTResponse::PMTPulse NoResponse::processPhotocathodeHit(G4double p_x, G4double p_y, G4double p_wavelength)
{

    OMSimPMTResponse::PMTPulse pulse;
    pulse.detectionProbability = 1;
    pulse.PE = 0;
    pulse.transitTime = 0;
    return pulse;
}
