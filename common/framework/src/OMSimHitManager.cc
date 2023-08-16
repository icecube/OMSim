#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"

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
 * @brief Counts the number of hits for each PMT in a module
 * @
 * @return A vector containing the counts of hits for each PMT.
 */
std::vector<double> OMSimHitManager::countHits(int moduleNumber)
{
	G4int lDOMIdx = OMSimCommandArgsTable::getInstance().get<G4int>("detector_type");

	std::vector<double> lHits(mNumPMTs + 2, 0.0);
	HitStats lHitsOfModule = mModuleHits[moduleNumber];

	for (int i = 0; i < (int)lHitsOfModule.PMT_hit.size(); i++)
	{
		lHits[lHitsOfModule.PMT_hit.at(i)] += 1; // lHitsOfModule.PMT_response.at(i).DetectionProbability;
		lHits[mNumPMTs + 1] += 1;
	}

	return lHits;
}

/**
 * @brief Empties the HitStats struct.
 */
void OMSimHitManager::setNumberOfPMTs(int pNumberOfPMTs)
{
	mNumPMTs = pNumberOfPMTs;
}

/**
 * @brief Returns struct of hit information for a module index (defaults to module 0, in case only a single module is simulated you don't have to provide a nr)
 * @param moduleIndex Module index for which we are getting the information (default 0)
 */
HitStats OMSimHitManager::getHitsOfModule(int moduleIndex)
{
	return mModuleHits[moduleIndex];
}


/**
 * @brief Empties the HitStats structs.
 */
void OMSimHitManager::reset()
{
	mModuleHits.clear();
}