#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimEventAction.hh"
#include "OMSimLogger.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include <numeric>

G4Mutex OMSimHitManager::mMutex = G4Mutex();
OMSimHitManager *OMSimHitManager::mInstance = nullptr;
G4ThreadLocal OMSimHitManager::ThreadLocalData *OMSimHitManager::mThreadData = nullptr;


OMSimHitManager::OMSimHitManager(): mCurrentIndex(-1)
{
};

/**
 * @brief Initializes the global instance of OMSimHitManager.
 * 
 * This is normally done in OMSim::initialiseSimulation.
 */
void OMSimHitManager::init()
{
	if (!gHitManager) gHitManager = new OMSimHitManager();
}

/**
 * @brief Shuts down and deletes the global instance of OMSimHitManager.
 */
void OMSimHitManager::shutdown()
{
	delete gHitManager;
	gHitManager = nullptr;
}


/**
 * @return A reference to the global OMSimHitManager instance.
 * @throws assert if instance exists (i.e. if it was called before init() or after shutdown()).
 */
OMSimHitManager &OMSimHitManager::getInstance()
{
	assert(gHitManager);
	return *gHitManager;
}

/**
 * @brief Applies a given permutation to a container.
 *
 * This function rearranges the elements of a vector according to a given permutation.
 * It achieves this using cycles to minimize the number of moves. The function assumes that
 * `pPermutation` holds a permutation of indices into `pVec`.
 * @param pVec The vector to which the permutation will be applied.
 * @param pPermutation A permutation represented as a vector of indices.
 */
template <typename T>
void applyPermutation(std::vector<T> &pVec, const std::vector<std::size_t> &pPermutation)
{
	std::vector<bool> lDone(pVec.size()); // Vector to keep track of which positions have been fixed.
	for (std::size_t i = 0; i < pVec.size(); ++i)
	{
		if (lDone[i])
			continue;	 // Skip the item if it's already in place.
		lDone[i] = true; // Mark the item as placed.
		std::size_t prev_j = i;
		std::size_t j = pPermutation[i];
		while (i != j)
		{ // Continue moving items in the cycle until the entire cycle is complete.
			std::swap(pVec[prev_j], pVec[j]);
			lDone[j] = true;
			prev_j = j;
			j = pPermutation[j];
		}
	}
}

/**
 * @brief Appends hit information for a detected photon to the corresponding module's hit data.
 *
 * This method appends hit information to the corresponding module's `HitStats` structure in the manager.
 * If the specified module number is not yet in the manager, a new `HitStats` structure is created for it.
 *
 * @param pGlobalTime Time of detection.
 * @param pLocalTime Photon flight time.
 * @param pTrackLength Length of the photon's path before hitting.
 * @param pEnergy Energy of the detected photon.
 * @param pPMTHitNumber ID of the PMT that detected the photon.
 * @param pMomentumDirection Momentum direction of the photon at the time of detection.
 * @param pGlobalPos Global position of the detected photon.
 * @param pLocalPos Local position of the detected photon within the PMT.
 * @param pDistance Distance between generation and detection of photon.
 * @param pResponse PMT's pResponse to the detected photon, encapsulated as a `PMTPulse`.
 * @param pModuleNumber ID of the module in which the photon was detected.
 */
void OMSimHitManager::appendHitInfo(
	G4double pGlobalTime,
	G4double pLocalTime,
	G4double pTrackLength,
	G4double pEnergy,
	G4int pPMTHitNumber,
	G4ThreeVector pMomentumDirection,
	G4ThreeVector pGlobalPos,
	G4ThreeVector pLocalPos,
	G4double pDistance,
	OMSimPMTResponse::PMTPulse pResponse,
	G4int pModuleNumber)
{

	if (!mThreadData)
	{
		log_debug("Initialized mThreadData for thread {} seed {}", G4Threading::G4GetThreadId(),  G4Random::getTheSeed());
		mThreadData = new ThreadLocalData();
	}

	// Check if the module exists in the thread-local map
	if (mThreadData->moduleHits.find(pModuleNumber) == mThreadData->moduleHits.end())
	{
		// Create a new HitStats for this module
		mThreadData->moduleHits[pModuleNumber] = HitStats();
	}
	
	auto &moduleHits = mThreadData->moduleHits[pModuleNumber];
	G4int eventID = G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID();
	//log_debug("Thread {} Seed {} event {} size {}", G4Threading::G4GetThreadId(),  G4Random::getTheSeed(), eventID, moduleHits.eventId.size());
	moduleHits.eventId.push_back(eventID);
	moduleHits.hitTime.push_back(pGlobalTime);
	moduleHits.flightTime.push_back(pLocalTime);
	moduleHits.pathLenght.push_back(pTrackLength);
	moduleHits.energy.push_back(pEnergy);
	moduleHits.PMTnr.push_back(pPMTHitNumber);
	moduleHits.direction.push_back(pMomentumDirection);
	moduleHits.globalPosition.push_back(pGlobalPos);
	moduleHits.localPosition.push_back(pLocalPos);
	moduleHits.generationDetectionDistance.push_back(pDistance);
	moduleHits.PMTresponse.push_back(pResponse);
	log_trace("Saved hit nr {} on module {} sensor {} (thread {})", moduleHits.eventId.size(), pModuleNumber, pPMTHitNumber, G4Threading::G4GetThreadId());
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
 * @brief Retrieves the HitStats structure for the specified module, should be called after data between threads was merged. @see mergeThreadData
 * @param moduleIndex Index of the module for which to retrieve hit statistics. Default is 0.
 * @return A HitStats structure containing hit information of specified module.
 */
HitStats OMSimHitManager::getMergedHitsOfModule(int pModuleIndex)
{
	return mModuleHits[pModuleIndex];
}




/**
 * @brief Retrieves the HitStats structure for the specified module of single thread.
 * @param moduleIndex Index of the module for which to retrieve hit statistics. Default is 0.
 * @return A HitStats structure containing hit information of specified module.
 */
HitStats OMSimHitManager::getSingleThreadHitsOfModule(int pModuleIndex)
{
	log_debug("Getting mThreadData of module {} (thread {})", pModuleIndex, G4Threading::G4GetThreadId());
	return mThreadData->moduleHits[pModuleIndex];
}


bool OMSimHitManager::areThereHitsInModuleSingleThread(int pModuleIndex)
{
	if (!mThreadData) {return false;};
	return mThreadData->moduleHits.find(pModuleIndex) != mThreadData->moduleHits.end();
}

/**
 * @brief Deletes hit information in memory for all modules.
 */
void OMSimHitManager::reset()
{
	log_trace("Reseting hit manager");
	//mModuleHits.clear();
	if (mThreadData)
	{
		log_trace("Deleting mThreadData of Thread ID {}", G4Threading::G4GetThreadId());
		mThreadData->moduleHits.clear();
		delete mThreadData;
		mThreadData = nullptr;
	}
	mModuleHits.clear();
	log_trace("Finished reseting hit manager");
}

/**
 * @brief Counts hits for a specified module.
 * @param pModuleIndex Index of the module for which to count hits. Default is 0.
 * @return A vector containing the hit count for each PMT in the specified module.
 */
std::vector<double> OMSimHitManager::countMergedHits(int pModuleIndex)
{
	log_trace("Counting number of detected photons in module with index {}", pModuleIndex);
	HitStats lHitsOfModule = mModuleHits[pModuleIndex];
	G4int lNumberPMTs = mNumPMTs[pModuleIndex];

	std::vector<double> lHits(lNumberPMTs + 1, 0.0);
	for (int i = 0; i < (int)lHitsOfModule.PMTnr.size(); i++)
	{
		lHits[lHitsOfModule.PMTnr.at(i)] += 1; // lHitsOfModule.PMTresponse.at(i).detectionProbability;
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
	std::vector<std::size_t> indices(lHits.hitTime.size());
	std::iota(indices.begin(), indices.end(), 0); // Fill it with 0, 1, ... N-1

	// Sort the indices vector according to hitTime
	std::sort(indices.begin(), indices.end(),
			  [&lHits](std::size_t a, std::size_t b)
			  {
				  return lHits.hitTime[a] < lHits.hitTime[b];
			  });

	// Now, apply the permutation function to every vector in the struct
	applyPermutation(lHits.eventId, indices);
	applyPermutation(lHits.hitTime, indices);
	applyPermutation(lHits.flightTime, indices);
	applyPermutation(lHits.pathLenght, indices);
	applyPermutation(lHits.energy, indices);
	applyPermutation(lHits.PMTnr, indices);
	applyPermutation(lHits.direction, indices);
	applyPermutation(lHits.localPosition, indices);
	applyPermutation(lHits.globalPosition, indices);
	applyPermutation(lHits.generationDetectionDistance, indices);
	applyPermutation(lHits.PMTresponse, indices);
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
	log_trace("Calculating multiplicity in time window {} for module with index", pTimeWindow, pModuleIndex);

	HitStats lHitsOfModule = mModuleHits[pModuleIndex];
	G4int lNumberPMTs = mNumPMTs[pModuleIndex];

	sortHitStatsByTime(lHitsOfModule);

	std::vector<int> lMultiplicity(lNumberPMTs, 0); // Initialize with zeros and size pPMTCount

	std::size_t lSkiptUntil = 0; // Index up to which we should skip in outer loop
	int lVectorSize = lHitsOfModule.hitTime.size();

	for (std::size_t i = 0; i < lVectorSize - 1; ++i)
	{
		if (i < lSkiptUntil)
		{
			continue;
		}

		int lPMTHits[lNumberPMTs] = {0};
		lPMTHits[lHitsOfModule.PMTnr.at(i)] = 1;
		int lCurrentSum = 0;

		// Loop through the next hits to see if they're within the time window
		for (std::size_t j = i + 1; j < lVectorSize; ++j)
		{
			if ((lHitsOfModule.hitTime.at(j) - lHitsOfModule.hitTime.at(i)) > pTimeWindow)
			{
				lSkiptUntil = j;
				break;
			}
			else
			{
				lPMTHits[lHitsOfModule.PMTnr.at(j)] = 1;
			}
		}

		// Calculate the multiplicity
		for (std::size_t k = 0; k < lNumberPMTs; ++k)
		{
			lCurrentSum += lPMTHits[k];
		}
		lMultiplicity[lCurrentSum - 1] += 1;
	}
	return lMultiplicity;
}

void OMSimHitManager::mergeThreadData()
{
	log_trace("Merge thread data was called");
	G4AutoLock lock(&mMutex);
	if (mThreadData)
	{
		log_debug("Merging data for thread {}", G4Threading::G4GetThreadId());
		for (const auto &[moduleIndex, hits] : mThreadData->moduleHits)
		{
			auto &globalHits = mModuleHits[moduleIndex];

			log_debug("Thread ID: {} - Module Index: {} - Hit vector sizes: ={} - Merged size prior {}",
					  G4Threading::G4GetThreadId(), moduleIndex, hits.eventId.size(), globalHits.eventId.size());

			globalHits.eventId.insert(globalHits.eventId.end(), hits.eventId.begin(), hits.eventId.end());
			globalHits.hitTime.insert(globalHits.hitTime.end(), hits.hitTime.begin(), hits.hitTime.end());
			globalHits.flightTime.insert(globalHits.flightTime.end(), hits.flightTime.begin(), hits.flightTime.end());
			globalHits.pathLenght.insert(globalHits.pathLenght.end(), hits.pathLenght.begin(), hits.pathLenght.end());
			globalHits.energy.insert(globalHits.energy.end(), hits.energy.begin(), hits.energy.end());
			globalHits.PMTnr.insert(globalHits.PMTnr.end(), hits.PMTnr.begin(), hits.PMTnr.end());
			globalHits.direction.insert(globalHits.direction.end(), hits.direction.begin(), hits.direction.end());
			globalHits.localPosition.insert(globalHits.localPosition.end(), hits.localPosition.begin(), hits.localPosition.end());
			globalHits.globalPosition.insert(globalHits.globalPosition.end(), hits.globalPosition.begin(), hits.globalPosition.end());
			globalHits.generationDetectionDistance.insert(globalHits.generationDetectionDistance.end(), hits.generationDetectionDistance.begin(), hits.generationDetectionDistance.end());
			globalHits.PMTresponse.insert(globalHits.PMTresponse.end(), hits.PMTresponse.begin(), hits.PMTresponse.end());
		}
		delete mThreadData;
		mThreadData = nullptr;
	}
}
