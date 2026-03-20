#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"




/**
 * @brief Calculates the effective area based on the number of hits and beam properties.
 * @param p_weightTotal The number of hits weighted.
 * @param p_totalCount The number of hits
 * @return Returns a structure with the effective area and its uncertainty.
 */
effectiveAreaResult OMSimEffectiveAreaAnalyisis::calculateEffectiveArea(double p_weightTotal, double p_totalCount)
{
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	G4double beamRadius = args.get<G4double>("radius")/10.; //in cm
	G4double numberPhotons = args.get<int>("numevents");
	G4double beamArea = CLHEP::pi * beamRadius * beamRadius;
	G4double effectiveArea = p_weightTotal * beamArea / numberPhotons;
	G4double effectiveAreaError = effectiveArea / sqrt(p_totalCount);
	return { effectiveArea, effectiveAreaError };
}
void OMSimEffectiveAreaAnalyisis::accumulateMultiplicity(G4double p_TimeWindow) 
{
    std::vector<int> currentMultiplicity = OMSimHitManager::getInstance().calculateMultiplicity(p_TimeWindow);
    for(size_t i = 0; i < currentMultiplicity.size(); i++) {
        m_totalMultiplicity[i] += currentMultiplicity[i];
    }
}
void OMSimEffectiveAreaAnalyisis::writeTotalMultiplicity(G4double p_TimeWindow)
{
    std::fstream dataFile;
    dataFile.open(m_outputFileNameMultiplicity.c_str(), std::ios::out | std::ios::app);
    
    // Write the accumulated multiplicity for each PMT
    for(const auto &value : m_totalMultiplicity) {
        dataFile << value << "\t";
    }
    dataFile << G4endl;
    dataFile.close();
    
    // Reset the accumulator
    std::fill(m_totalMultiplicity.begin(), m_totalMultiplicity.end(), 0);
}
//writes only if non-zero
void OMSimEffectiveAreaAnalyisis::writeMultiplicity(G4double p_TimeWindow)
{
    std::vector<int> lMultiplicity = OMSimHitManager::getInstance().calculateMultiplicity(p_TimeWindow);
    // Check if any value in the vector is non-zero
    bool hasNonZero = std::any_of(lMultiplicity.begin(), lMultiplicity.end(), [](int val) {
        return val != 0;
    });
    if (!hasNonZero) return;  // Skip writing if zero
    mDatafile.open(m_outputFileNameMultiplicity.c_str(), std::ios::out | std::ios::app);
    for (const auto &value : lMultiplicity) {
        mDatafile << value << "\t";
    }
    mDatafile << G4endl;
    mDatafile.close();
}


//writes every particle, no matter if it was a hit or not
/*void OMSimEffectiveAreaAnalyisis::writeMultiplicity(G4double p_TimeWindow)
{
    std::vector<int> lMultiplicity = OMSimHitManager::getInstance().calculateMultiplicity(p_TimeWindow);

    mDatafile.open(m_outputFileNameMultiplicity.c_str(), std::ios::out | std::ios::app);
    for (const auto &value : lMultiplicity) {
        mDatafile << value << "\t";
    }
    mDatafile << G4endl;
    mDatafile.close();
}*/

void OMSimEffectiveAreaAnalyisis::writeHitInformation()
{
    HitStats lHits = OMSimHitManager::getInstance().getMergedHitsOfModule();
    mDatafile.open(m_outputFileNameInfo.c_str(), std::ios::out | std::ios::app);
    if (!lHits.eventId.empty())
    {
        for (size_t i = 0; i < lHits.eventId.size(); i++)
        {
            mDatafile << lHits.eventId.at(i) << "\t";
            mDatafile << std::setprecision(13);
            mDatafile << lHits.hitTime.at(i) / ns << "\t"; 
            mDatafile << std::setprecision(4);
            mDatafile << lHits.PMTnr.at(i) << "\t";
            mDatafile << lHits.energy.at(i) << "\t";
            mDatafile << lHits.globalPosition.at(i).x() << "\t";
            mDatafile << lHits.globalPosition.at(i).y() << "\t";
            mDatafile << lHits.globalPosition.at(i).z() << "\t";
        //  mDatafile << lHits.PMT_response.at(i).PE << "\t";
        //   mDatafile << lHits.PMT_response.at(i).TransitTime << "\t";
        //  mDatafile << lHits.PMT_response.at(i).DetectionProbability << "\t";
            mDatafile << G4endl;
        }
    }
    mDatafile.close();
}

