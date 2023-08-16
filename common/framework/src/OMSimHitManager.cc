#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"



/**
 * @brief Appends hit information for a detected photon to the corresponding module's hit data.
 * 
 * This method appends hit information to the corresponding module's `HitStats` structure in the manager.
 * If the specified module number is not yet in the manager, a new `HitStats` structure is created for it.
 *
 * @param globalTime Time of detection.
 * @param localTime Photon flight time.
 * @param trackLength Length of the photon's path before hitting.
 * @param energy Energy of the detected photon.
 * @param PMTHitNumber ID of the PMT that detected the photon.
 * @param momentumDirection Momentum direction of the photon at the time of detection.
 * @param globalPos Global position of the detected photon.
 * @param localPos Local position of the detected photon within the PMT.
 * @param distance Distance between generation and detection of photon.
 * @param response PMT's response to the detected photon, encapsulated as a `PMTPulse`.
 * @param moduleNumber ID of the module in which the photon was detected.
 *
 * @note This method should be called every time a photon is detected in a PMT.
 */
void OMSimHitManager::appendHitInfo(
	G4double globalTime,
	G4double localTime,
	G4double trackLength,
	G4double energy,
	G4int PMTHitNumber,
	G4ThreeVector momentumDirection,
	G4ThreeVector globalPos,
	G4ThreeVector localPos,
	G4double distance,
	OMSimPMTResponse::PMTPulse response,
	G4int moduleNumber)
{
	// Check if the module exists in the map
	if (mModuleHits.find(moduleNumber) == mModuleHits.end())
	{
		// Create a new HitStats for this module
		mModuleHits[moduleNumber] = HitStats();
	}
	G4int lEventID = EventInfoManager::getInstance().getCurrentEventID();
	mModuleHits[moduleNumber].event_id.push_back(lEventID);
	mModuleHits[moduleNumber].hit_time.push_back(globalTime);
	mModuleHits[moduleNumber].photon_flight_time.push_back(localTime);
	mModuleHits[moduleNumber].photon_track_length.push_back(trackLength);
	mModuleHits[moduleNumber].photon_energy.push_back(energy);
	mModuleHits[moduleNumber].PMT_hit.push_back(PMTHitNumber);
	mModuleHits[moduleNumber].photon_direction.push_back(momentumDirection);
	mModuleHits[moduleNumber].photon_global_position.push_back(globalPos);
	mModuleHits[moduleNumber].photon_local_position.push_back(localPos);
	mModuleHits[moduleNumber].event_distance.push_back(distance);
	mModuleHits[moduleNumber].PMT_response.push_back(response);
}

/**
 * @brief Counts hits for a specified module.
 * @param moduleIndex Index of the module for which to count hits. Default is 0.
 * @return A vector containing the hit count for each PMT in the specified module.
 */
std::vector<double> OMSimHitManager::countHits(int moduleNumber)
{
	HitStats lHitsOfModule = mModuleHits[moduleNumber];
	G4int lNumberPMTs = mNumPMTs[moduleNumber];

	std::vector<double> lHits(lNumberPMTs + 2, 0.0);
	for (int i = 0; i < (int)lHitsOfModule.PMT_hit.size(); i++)
	{
		lHits[lHitsOfModule.PMT_hit.at(i)] += 1; // lHitsOfModule.PMT_response.at(i).DetectionProbability;
		lHits[lNumberPMTs + 1] += 1;
	}

	return lHits;
}

/**
 * @brief Saves the number of PMTs in a module
 * @param pNumberOfPMTs Nr of PMTs in OM
 * @param pModuleIndex Module index for which we are getting the information (default 0)
 */
void OMSimHitManager::setNumberOfPMTs(int pNumberOfPMTs, int pModuleIndex)
{
	mNumPMTs[pModuleIndex] = pNumberOfPMTs;

}

/**
 * @brief Retrieves the hit statistics for a specified module.
 * @param moduleIndex Index of the module for which to retrieve hit statistics. Default is 0.
 * @return A HitStats structure containing hit information for the specified module.
 */
HitStats OMSimHitManager::getHitsOfModule(int pModuleIndex)
{
	return mModuleHits[pModuleIndex];
}


/**
 * @brief Resets hit information for all modules.
 */
void OMSimHitManager::reset()
{
	mModuleHits.clear();
}