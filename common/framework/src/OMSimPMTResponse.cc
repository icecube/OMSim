
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

OMSimPMTResponse::OMSimPMTResponse() : m_relativeDetectionEfficiencyInterpolator(nullptr), m_QEfileInterpolator(nullptr), m_weightAbsorbedToQEInterpolator(nullptr), m_scannedWavelengths(std::vector<G4double>()), m_X(0), m_Y(0)
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
 * @brief Creates the QE interpolator from the target QE file.
 *
 * @param p_FileNameTargetQE File name containing target QE data.
 */
void OMSimPMTResponse::makeQEInterpolator(const std::string &p_FileNameTargetQE)
{
    log_trace("Creating QE interpolator...");
    auto dataQE = Tools::loadtxt(p_FileNameTargetQE.c_str(), true, 0, '\t');

    std::vector<double> wavelengths = dataQE[0];
    std::vector<double> theQEs = dataQE[1];

    double maxWavelength = *std::max_element(wavelengths.begin(), wavelengths.end());
    double minWavelength = *std::min_element(wavelengths.begin(), wavelengths.end());

    // Append points above the maximum
    for (int i = 1; i <= 3; ++i)
    {
        double newWavelength = maxWavelength + i * (maxWavelength - minWavelength) / 3;
        wavelengths.push_back(newWavelength);
        theQEs.push_back(0);
    }

    // Append points below the minimum
    for (int i = 1; i <= 3; ++i)
    {
        double newWavelength = minWavelength - i * minWavelength / 3;
        wavelengths.push_back(std::max(0.0, newWavelength));
        theQEs.push_back(0);
    }

    Tools::sortVectorByReference(wavelengths, theQEs);
    m_QEfileInterpolator = Tools::create1dInterpolator(wavelengths, theQEs, p_FileNameTargetQE);
    m_QEInterpolatorAvailable = true;
}

/**
 * @brief Configures the QE weight interpolator for PMT response.
 * @param p_FileNameAbsorbedFraction File name containing absorbed fraction data
 */
void OMSimPMTResponse::makeQEweightInterpolator(const std::string &p_FileNameAbsorbedFraction)
{
    log_trace("Creating QE weight interpolator...");
    // Load absorbed fraction data
    auto absorbedData = Tools::loadtxt(p_FileNameAbsorbedFraction.c_str(), true, 0, '\t');

    std::vector<double> wavelengths = absorbedData[0];
    std::vector<double> absorbedFraction = absorbedData[1];

    // Calculate weights
    std::vector<double> weights;

    for (size_t i = 0; i < wavelengths.size(); ++i)
    {
        double wavelength = wavelengths[i];
        double fraction = absorbedFraction[i];
        double targetQE = m_QEfileInterpolator->Eval(wavelength);
        if (targetQE > fraction)
        {
            log_error("Requested QE value {} is larger than possible from optical properties of PMT ({})!", targetQE, fraction);
            throw std::invalid_argument("Requested QE value larger than possible from optical properties of PMT!");
        }
        double toAppend = (fraction == 0) ? 0 : targetQE / fraction;
        weights.push_back(toAppend);
    }
    double maxWavelength = *std::max_element(wavelengths.begin(), wavelengths.end());
    double minWavelength = *std::min_element(wavelengths.begin(), wavelengths.end());

    // Append points above the maximum
    for (int i = 1; i <= 3; ++i)
    {
        double newWavelength = maxWavelength + i * (maxWavelength - minWavelength) / 3;
        wavelengths.push_back(newWavelength);
        weights.push_back(0);
    }

    // Append points below the minimum
    for (int i = 1; i <= 3; ++i)
    {
        double newWavelength = minWavelength - i * minWavelength / 3;
        wavelengths.push_back(std::max(0.0, newWavelength));
        weights.push_back(0);
    }

    Tools::sortVectorByReference(const_cast<std::vector<double> &>(wavelengths), weights);
    m_weightAbsorbedToQEInterpolator = Tools::create1dInterpolator(wavelengths, weights, "weights QE interpolator");
    m_QEWeightInterpolatorAvailable = true;
}

/**
 * @brief Configures the CE weight interpolator for PMT response.
 * @param p_FileName File name containing CE weights
 */
void OMSimPMTResponse::makeCEweightInterpolator(const std::string &p_FileName)
{
    log_trace("Creating CE weight interpolator...");
    m_relativeDetectionEfficiencyInterpolator = Tools::create1dInterpolator(p_FileName.c_str());
    m_CEWeightInterpolatorAvailable = true;
}

void OMSimPMTResponse::makeScansInterpolators(const std::string &p_PathToFiles)
{
    log_trace("Creating TH2D interpolators from scan data...");

    for (const auto &key : getScannedWavelengths())
    {
        std::string wavelength = std::to_string((int)(key/nm));
        m_gainG2Dmap[key] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "Gain_PE_" + wavelength + ".dat");
        m_gainResolutionG2Dmap[key] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "SPEresolution_" + wavelength + ".dat");
        m_transitTimeSpreadG2Dmap[key] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "TransitTimeSpread_" + wavelength + ".dat");
        m_transitTimeG2Dmap[key] = Tools::create2DHistogramFromDataFile(p_PathToFiles + "TransitTime_" + wavelength + ".dat");
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
        weightQE = (m_QEInterpolatorAvailable) ? m_QEfileInterpolator->Eval(p_wavelength / nm) : 1.;
        weightCE = 1.;
    }
    else
    {
        weightQE = (m_QEWeightInterpolatorAvailable) ? m_weightAbsorbedToQEInterpolator->Eval(p_wavelength / nm) : 1.;
        weightCE = (m_CEWeightInterpolatorAvailable) ? m_relativeDetectionEfficiencyInterpolator->Eval(r) : 1.;
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
    pulse.detectionProbability = weightQE * weightCE;
    return pulse;
}

/**
 * @brief Set the scanned wavelengths for the PMT response.
 *
 * @param p_wavelengths The scanned wavelengths.
 */ 
void OMSimPMTResponse::setScannedWavelengths(std::vector<double> p_wavelengths)
{
    m_scannedWavelengths = p_wavelengths;
}

/**
 * @brief Get the scanned wavelengths for the PMT response.
 *
 * @return std::vector<double> The scanned wavelengths.
 */ 
std::vector<G4double> OMSimPMTResponse::getScannedWavelengths() 
{
    return m_scannedWavelengths;
}