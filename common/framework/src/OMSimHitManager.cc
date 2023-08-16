#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"

#include <numeric>

/**
 * @brief Applies a given permutation to a container.
 *
 * This function rearranges the elements of a vector according to a given permutation.
 * It achieves this using cycles to minimize the number of moves. The function assumes that
 * `p` holds a permutation of indices into `vec`.
 * @param vec The vector to which the permutation will be applied.
 * @param p A permutation represented as a vector of indices.
 */
template <typename T>
void applyPermutation(std::vector<T> &vec, const std::vector<std::size_t> &p)
{
	std::vector<bool> done(vec.size()); // Vector to keep track of which positions have been fixed.
	for (std::size_t i = 0; i < vec.size(); ++i)
	{
		if (done[i])
			continue;	// Skip the item if it's already in place.
		done[i] = true; // Mark the item as placed.
		std::size_t prev_j = i;
		std::size_t j = p[i];
		while (i != j)
		{ // Continue moving items in the cycle until the entire cycle is complete.
			std::swap(vec[prev_j], vec[j]);
			done[j] = true;
			prev_j = j;
			j = p[j];
		}
	}
}


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


void OMSimHitManager::setNumberOfPMTs(int pNumberOfPMTs, int pModuleIndex)
{
	mNumPMTs[pModuleIndex] = pNumberOfPMTs;
}


HitStats OMSimHitManager::getHitsOfModule(int pModuleIndex)
{
	return mModuleHits[pModuleIndex];
}


void OMSimHitManager::reset()
{
	mModuleHits.clear();
}


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


void OMSimHitManager::sortHitStatsByTime(HitStats &lHits)
{
	// Create a vector of indices
	std::vector<std::size_t> indices(lHits.hit_time.size());
	std::iota(indices.begin(), indices.end(), 0); // Fill it with 0, 1, ... N-1

	// Sort the indices vector according to hit_time
	std::sort(indices.begin(), indices.end(),
			  [&lHits](std::size_t a, std::size_t b)
			  {
				  return lHits.hit_time[a] < lHits.hit_time[b];
			  });

	// Now, apply the permutation function to every vector in the struct
	applyPermutation(lHits.event_id, indices);
	applyPermutation(lHits.hit_time, indices);
	applyPermutation(lHits.photon_flight_time, indices);
	applyPermutation(lHits.photon_track_length, indices);
	applyPermutation(lHits.photon_energy, indices);
	applyPermutation(lHits.PMT_hit, indices);
	applyPermutation(lHits.photon_direction, indices);
	applyPermutation(lHits.photon_local_position, indices);
	applyPermutation(lHits.photon_global_position, indices);
	applyPermutation(lHits.event_distance, indices);
	applyPermutation(lHits.PMT_response, indices);
}


std::vector<int> OMSimHitManager::calculateMultiplicity(const G4double pTimeWindow, int moduleNumber)
{
	HitStats lHitsOfModule = mModuleHits[moduleNumber];
	G4int lNumberPMTs = mNumPMTs[moduleNumber];

	sortHitStatsByTime(lHitsOfModule);

	std::vector<int> lMultiplicity(lNumberPMTs, 0); // Initialize with zeros and size pPMTCount

	std::size_t lSkiptUntil = 0; // Index up to which we should skip in outer loop
	int lVectorSize = lHitsOfModule.hit_time.size();

	for (std::size_t i = 0; i < lVectorSize - 1; ++i)
	{
		if (i < lSkiptUntil)
		{
			continue;
		}

		int lPMTHits[lNumberPMTs] = {0};
		lPMTHits[lHitsOfModule.PMT_hit.at(i)] = 1;
		int lcurrentSum = 0;

		// Loop through the next hits to see if they're within the time window
		for (std::size_t j = i + 1; j < lVectorSize; ++j)
		{
			if ((lHitsOfModule.hit_time.at(j) - lHitsOfModule.hit_time.at(i)) > pTimeWindow)
			{
				lSkiptUntil = j;
				break;
			}
			else
			{
				lPMTHits[lHitsOfModule.PMT_hit.at(j)] = 1;
			}
		}

		// Calculate the multiplicity
		for (std::size_t k = 0; k < lNumberPMTs; ++k)
		{
			lcurrentSum += lPMTHits[k];
		}
		lMultiplicity[lcurrentSum - 1] += 1;
	}
	return lMultiplicity;
}