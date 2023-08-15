#include "OMSimAnalysisManager.hh"
#include "OMSimCommandArgsTable.hh"

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
void OMSimHitManager::applyPermutation(std::vector<T>& vec, const std::vector<std::size_t>& p) {
    std::vector<bool> done(vec.size()); // Vector to keep track of which positions have been fixed.
    for (std::size_t i = 0; i < vec.size(); ++i) {
        if (done[i]) continue; // Skip the item if it's already in place.
        done[i] = true; // Mark the item as placed.
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j) { // Continue moving items in the cycle until the entire cycle is complete.
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}

void OMSimHitManager::sortHitStatsByTime(HitStats& lHits) {
    // Create a vector of indices
    std::vector<std::size_t> indices(lHits.hit_time.size());
    std::iota(indices.begin(), indices.end(), 0);  // Fill it with 0, 1, ... N-1

    // Sort the indices vector according to hit_time
    std::sort(indices.begin(), indices.end(),
              [&lHits](std::size_t a, std::size_t b) {
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


std::vector<int> OMSimHitManager::calculateMultiplicity(const G4double pTimeWindow, std::size_t pPMTCount)
{
	sortHitStatsByTime(mHits);
	std::vector<int> lMultiplicity(pPMTCount, 0); // Initialize with zeros and size pPMTCount

	std::size_t lSkiptUntil = 0; // Index up to which we should skip in outer loop

	for (std::size_t i = 0; i < mHits.hit_time.size() - 1; ++i)
	{
		if (i < lSkiptUntil)
		{
			continue;
		}

		int lPMTHits[pPMTCount] = {0};
		lPMTHits[mHits.PMT_hit.at(i)] = 1;
		int lcurrentSum = 0;

		// Loop through the next hits to see if they're within the time window
		for (std::size_t j = i + 1; j < mHits.hit_time.size(); ++j)
		{
			if ((mHits.hit_time.at(j) - mHits.hit_time.at(i)) > pTimeWindow)
			{
				lSkiptUntil = j;
				break;
			}
			else
			{
				lPMTHits[mHits.PMT_hit.at(j)] = 1;
			}
		}

		// Calculate the multiplicity
		for (std::size_t k = 0; k < pPMTCount; ++k)
		{
			lcurrentSum += lPMTHits[k];
		}
		lMultiplicity[lcurrentSum - 1] += 1;
	}
	return lMultiplicity;
}

void OMSimHitManager::writeMultiplicity(const std::vector<int> &pMultiplicity)
{
	for (const auto &value : pMultiplicity)
	{
		mDatafile << value << "\t";
	}
	mDatafile << G4endl;
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
 * @brief Writes a header to the output file.
 */
void OMSimHitManager::writeHeader()
{
	mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);
	mDatafile << "# Phi(deg)"
			  << "\t"
			  << "Theta(deg)"
			  << "\t"
			  << "Radius(mm)"
			  << "\t"
			  << "HitsPMTs"
			  << "\t"
			  << "Total"
			  << "\t" << G4endl;
	mDatafile.close();
}

/**
 * @brief Writes a scan result to the output file.
 * @param pPhi The phi angle used in the scan to be written to the output file.
 * @param pTheta The phi angle used in the scan to be written to the output file.
 */
void OMSimHitManager::writeScan(G4double pPhi, G4double pTheta)
{
	std::vector<double> lHits = countHits();
	mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);
	mDatafile << pPhi << "\t" << pTheta << "\t";
	for (const auto &hit : lHits)
	{
		mDatafile << hit << "\t";
	}
	mDatafile << G4endl;
	mDatafile.close();
}

/**
 * @brief Empties the HitStats struct.
 */
void OMSimHitManager::reset()
{
	mHits = HitStats();
}
