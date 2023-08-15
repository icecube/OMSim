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
    OMSimPMTResponse::PMTPulse response
)
{
	G4int lEventID = EventInfoManager::getInstance().getCurrentEventID();
    mHits.event_id.push_back(lEventID);
    mHits.hit_time.push_back(globalTime);
    mHits.photon_flight_time.push_back(localTime);
    mHits.photon_track_length.push_back(trackLength);
    mHits.photon_energy.push_back(energy);
    mHits.PMT_hit.push_back(PMTHitNumber);
    mHits.photon_direction.push_back(momentumDirection);
    mHits.photon_global_position.push_back(globalPos);
    mHits.photon_local_position.push_back(localPos);
    mHits.event_distance.push_back(distance);
    mHits.PMT_response.push_back(response);
}


/**
 * @brief Counts the number of hits for each PMT
 * @return A vector containing the counts of hits for each PMT.
 */
std::vector<double> OMSimHitManager::countHits()
{
	G4int lDOMIdx = OMSimCommandArgsTable::getInstance().get<G4int>("detector_type");

	int lNumPMTs;
	if (lDOMIdx == 1)
	{
		lNumPMTs = 1;
	} // single PMT
	else if (lDOMIdx == 2)
	{
		lNumPMTs = 24;
	} // mDOM
	else if (lDOMIdx == 3)
	{
		lNumPMTs = 1;
	} // PDOM
	else if (lDOMIdx == 4)
	{
		lNumPMTs = 16;
	} // LOM16
	else if (lDOMIdx == 5)
	{
		lNumPMTs = 18;
	} // LOM18
	else if (lDOMIdx == 6)
	{
		lNumPMTs = 2;
	} // DEGG
	else
	{
		lNumPMTs = 99;
	} // custom

	std::vector<double> lHits(lNumPMTs + 2, 0.0);

	for (int i = 0; i < (int)mHits.PMT_hit.size(); i++)
	{
		lHits[mHits.PMT_hit.at(i)] += 1; // mHits.PMT_response.at(i).DetectionProbability;
		lHits[lNumPMTs + 1] += 1;
	}

	return lHits;
}

/**
 * @brief Empties the HitStats struct.
 */
void OMSimHitManager::reset()
{
	mHits = HitStats();
}
