#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"
#include "OMSimLogger.hh"

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
 * @brief Stores the number of PMTs in a module for correct data handling
 * @param pNumberOfPMTs Nr of PMTs in OM
 * @param pModuleIndex Module index for which we are getting the information (default 0)
 */
void OMSimHitManager::setNumberOfPMTs(int pNumberOfPMTs, int pModuleIndex)
{
	log_trace("Setting number of PMTs to {} in module with index {}", pNumberOfPMTs, pModuleIndex);
	mNumPMTs[pModuleIndex] = pNumberOfPMTs;
}

/**
 * @brief Retrieves the HitStats structure for the specified module.
 * @param moduleIndex Index of the module for which to retrieve hit statistics. Default is 0.
 * @return A HitStats structure containing hit information of specified module.
 */
HitStats OMSimHitManager::getHitsOfModule(int pModuleIndex)
{
	return mModuleHits[pModuleIndex];
}

/**
 * @brief Deletes hit information in memory for all modules.
 */
void OMSimHitManager::reset()
{
	mModuleHits.clear();
}

/**
 * @brief Counts hits for a specified module.
 * @param pModuleIndex Index of the module for which to count hits. Default is 0.
 * @return A vector containing the hit count for each PMT in the specified module.
 */
std::vector<double> OMSimHitManager::countHits(int pModuleIndex)
{
	log_debug("Counting number of detected photons in module with index {}", pModuleIndex);
	HitStats lHitsOfModule = mModuleHits[pModuleIndex];
	G4int lNumberPMTs = mNumPMTs[pModuleIndex];

	std::vector<double> lHits(lNumberPMTs + 1, 0.0);
	for (int i = 0; i < (int)lHitsOfModule.PMT_hit.size(); i++)
	{
		lHits[lHitsOfModule.PMT_hit.at(i)] += 1; // lHitsOfModule.PMT_response.at(i).DetectionProbability;
		lHits[lNumberPMTs] += 1;
	}

	return lHits;
}

/**
 * @brief Sorts the hit statistics by the hit time.
 * @param lHits The hit statistics to be sorted.
 */
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

/**
 * @brief Calculates the multiplicity of hits within a specified time window for a given module.
 *
 * This method determines the multiplicity of hits for the specified optical module within a
 * given time window. Multiplicity is defined as the number of PMTs detecting a hit within the
 * time window. The result is a vector where each element represents the number of occurrences
 * of a specific multiplicity.
 *
 * For instance, if the resulting vector is [5, 3, 2], it means:
 * - 5 occurrences of 1 PMT detecting a hit within the time window.
 * - 3 occurrences of 2 PMTs detecting hits within the same window.
 * - 2 occurrences of 3 PMTs detecting hits within the window.
 *
 * @param pTimeWindow The time window within which to calculate the multiplicity (in seconds).
 * @param pModuleIndex The index of the module for which to calculate the multiplicity. Default is 0.
 * @return A vector containing the multiplicity data.
 */
std::vector<int> OMSimHitManager::calculateMultiplicity(const G4double pTimeWindow, int pModuleIndex)
{
	log_debug("Calculating multiplicity in time window {} for module with index", pTimeWindow, pModuleIndex);
	HitStats lHitsOfModule = mModuleHits[pModuleIndex];
	G4int lNumberPMTs = mNumPMTs[pModuleIndex];

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