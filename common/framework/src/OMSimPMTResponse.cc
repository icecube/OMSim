
/** 
 *  @todo Add PMT data of all PMT types
 */
#include "OMSimPMTResponse.hh"
#include "OMSimLogger.hh"
#include "OMSimInputData.hh"

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

OMSimPMTResponse::OMSimPMTResponse()
{
    log_debug("Opening photocathode scans data...");
    std::string path = "../common/data/PMT_scans/";

    mRelativeDetectionEfficiencyInterp = new TGraph((path + "weightsVsR_vFit_220nm.txt").c_str());
    mRelativeDetectionEfficiencyInterp->SetName("RelativeDetectionEfficiencyWeight");

    for (const auto &lKey : mScannedWavelengths)
    {
        std::string lWv = std::to_string((int)(lKey / nm));
        mGainG2Dmap[lKey] = createHistogramFromData(path + "Gain_PE_" + lWv + ".dat", ("Gain_PE_" + lWv).c_str());
        mGainResolutionG2Dmap[lKey] = createHistogramFromData(path + "SPEresolution_" + lWv + ".dat", ("SPEresolution_" + lWv).c_str());
        mTransitTimeSpreadG2Dmap[lKey] = createHistogramFromData(path + "TransitTimeSpread_" + lWv + ".dat", ("TransitTimeSpread_" + lWv).c_str());
        mTransitTimeG2Dmap[lKey] = createHistogramFromData(path + "TransitTime_" + lWv + ".dat", ("TransitTime_" + lWv).c_str());
    }

    log_debug("Finished opening photocathode scans data...");
}

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

G4double OMSimPMTResponse::getTransitTime(G4double pWavelengthKey)
{

    G4double lMeanTransitTime = mTransitTimeG2Dmap[pWavelengthKey]->Interpolate(mX, mY) * ns;
    G4double lTTS = mTransitTimeSpreadG2Dmap[pWavelengthKey]->Interpolate(mX, mY) * ns;

    return G4RandGauss::shoot(lMeanTransitTime, lTTS);
}

G4double OMSimPMTResponse::getTransitTime(G4double pWavelength1, G4double pWavelength2)
{

    G4double lMeanTransitTime = wavelengthInterpolatedValue(mTransitTimeG2Dmap, pWavelength1, pWavelength2) * ns;
    G4double lTTS = wavelengthInterpolatedValue(mTransitTimeSpreadG2Dmap, pWavelength1, pWavelength2) * ns;

    return G4RandGauss::shoot(lMeanTransitTime, lTTS);
}

G4double OMSimPMTResponse::wavelengthInterpolatedValue(std::map<G4double, TH2D *> pMap, G4double pWavelength1, G4double pWavelength2)
{

    G4double lValue1 = pMap[pWavelength1]->Interpolate(mX, mY);
    G4double lValue2 = pMap[pWavelength2]->Interpolate(mX, mY);
    return lValue1 + (mWavelength - pWavelength1) * (lValue2 - lValue1) / (pWavelength2 - pWavelength1);
}

OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromKey(G4double pWavelengthKey)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.PE = getCharge(pWavelengthKey);
    lPulse.TransitTime = getTransitTime(pWavelengthKey);

    return lPulse;
}

OMSimPMTResponse::PMTPulse OMSimPMTResponse::getPulseFromInterpolation(G4double pWavelengthKey1, G4double pWavelengthKey2)
{
    OMSimPMTResponse::PMTPulse lPulse;
    lPulse.PE = getCharge(pWavelengthKey1, pWavelengthKey2);
    lPulse.TransitTime = getTransitTime(pWavelengthKey1, pWavelengthKey2);
    return lPulse;
}

OMSimPMTResponse::PMTPulse OMSimPMTResponse::processPhotocathodeHit(G4double pX, G4double pY, G4double pWavelength)
{

    mX = pX / mm;
    mY = pY / mm;
    G4double lR = std::sqrt(mX * mX + mY * mY);
    mWavelength = pWavelength;

    OMSimPMTResponse::PMTPulse lPulse;

    lPulse.DetectionProbability = mRelativeDetectionEfficiencyInterp->Eval(lR);

    // If wavelength matches with one of the measured ones
    if (std::find(mScannedWavelengths.begin(), mScannedWavelengths.end(), pWavelength) != mScannedWavelengths.end())
    {
        lPulse = getPulseFromKey(pWavelength);
    }
    // If wavelength smaller than scanned, return for first measured wavelength
    else if (pWavelength < mScannedWavelengths.at(0))
    {
        lPulse = getPulseFromKey(mScannedWavelengths.at(0));
    }
    // If wavelength larger than scanned, return for last measured wavelength
    else if (pWavelength > mScannedWavelengths.back())
    {
        lPulse = getPulseFromKey(mScannedWavelengths.back());
    }
    // If wavelength in range of scanned, return interpolated values
    else
    {
        // get first index of first element larger than seeked wavelength
        auto const lLowerBoundIndex = std::lower_bound(mScannedWavelengths.begin(), mScannedWavelengths.end(), pWavelength) - mScannedWavelengths.begin();

        G4double lW1 = mScannedWavelengths.at(lLowerBoundIndex - 1);
        G4double lW2 = mScannedWavelengths.at(lLowerBoundIndex);

        lPulse = getPulseFromInterpolation(lW1, lW2);
    }
    return lPulse;
}
