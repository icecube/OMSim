/**
 * @file OMSimHitManager.cc
 * @brief Implementation of enhanced hit manager for WavePID.
 */
#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimLogger.hh"
#include "G4EventManager.hh"
#include "G4Event.hh"
#include <numeric>
#include <stdexcept>

G4Mutex OMSimHitManager::m_mutex = G4Mutex();
OMSimHitManager* OMSimHitManager::m_instance = nullptr;
G4ThreadLocal OMSimHitManager::ThreadLocalData* OMSimHitManager::m_threadData = nullptr;

OMSimHitManager::OMSimHitManager()
    : m_currentIndex(-1), m_rootManager(nullptr)
{
}

void OMSimHitManager::init()
{
    if (!g_hitManager) {
        g_hitManager = new OMSimHitManager();
        log_info("OMSimHitManager initialized");
    }
}

void OMSimHitManager::setROOTOutputFile(const std::string& filename)
{
    if (!g_hitManager) {
        throw std::runtime_error("OMSimHitManager::setROOTOutputFile called before init()!");
    }
    if (g_hitManager->m_rootManager) {
        delete g_hitManager->m_rootManager;
    }
    g_hitManager->m_rootManager = new ROOTHitManager(filename);
    log_info("OMSimHitManager ROOT output set to: {}", filename);
}

void OMSimHitManager::shutdown()
{
    if (g_hitManager) {
        delete g_hitManager->m_rootManager;
        delete g_hitManager;
        g_hitManager = nullptr;
    }
}

OMSimHitManager& OMSimHitManager::getInstance()
{
    if (!g_hitManager) {
        throw std::runtime_error("OMSimHitManager accessed before initialization!");
    }
    return *g_hitManager;
}

template <typename T>
void applyPermutation(std::vector<T>& p_vector, const std::vector<std::size_t>& p_permutation)
{
    std::vector<bool> done(p_vector.size());
    for (std::size_t i = 0; i < p_vector.size(); ++i) {
        if (done[i]) continue;
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p_permutation[i];
        while (i != j) {
            std::swap(p_vector[prev_j], p_vector[j]);
            done[j] = true;
            prev_j = j;
            j = p_permutation[j];
        }
    }
}

void OMSimHitManager::appendHitInfo(
    G4int p_eventid,
    G4double p_globalTime,
    G4double p_entryTime,
    G4double p_localTime,
    G4double p_trackLength,
    G4double p_energy,
    G4int pPMTHitNumber,
    G4ThreeVector pMomentumDirection,
    G4ThreeVector pGlobalPos,
    G4ThreeVector pLocalPos,
    G4double p_distance,
    OMSimPMTResponse::PMTPulse pResponse,
    G4String pPhotonOrigin,
    G4int pParentID,
    std::string pParentType,
    G4String pParentProcess,
    G4double p_wavelength,
    G4int pModuleIndex)
{
    // Option to cache hits in memory (disabled by default for memory efficiency)
    static constexpr bool kCacheHitsInMemory = false;

    if (!m_threadData) {
        log_debug("Initialized m_threadData for thread {}", G4Threading::G4GetThreadId());
        m_threadData = new ThreadLocalData();
    }

    if (m_threadData->moduleHits.find(pModuleIndex) == m_threadData->moduleHits.end()) {
        m_threadData->moduleHits[pModuleIndex] = HitStats();
    }

    auto& moduleHits = m_threadData->moduleHits[pModuleIndex];

    // Optionally cache hits in memory
    if (kCacheHitsInMemory) {
        moduleHits.eventId.push_back(p_eventid);
        moduleHits.hitTime.push_back(p_globalTime);
        moduleHits.entryTime.push_back(p_entryTime);
        moduleHits.flightTime.push_back(p_localTime);
        moduleHits.pathLenght.push_back(p_trackLength);
        moduleHits.energy.push_back(p_energy);
        moduleHits.PMTnr.push_back(pPMTHitNumber);
        moduleHits.direction.push_back(pMomentumDirection);
        moduleHits.globalPosition.push_back(pGlobalPos);
        moduleHits.localPosition.push_back(pLocalPos);
        moduleHits.generationDetectionDistance.push_back(p_distance);
        moduleHits.PMTresponse.push_back(pResponse);
        moduleHits.photonOrigin.push_back(pPhotonOrigin);
        moduleHits.parentID.push_back(pParentID);
        moduleHits.parentType.push_back(pParentType);
        moduleHits.parentProcess.push_back(pParentProcess);
    }

    log_trace("Hit on module {} sensor {} (thread {})", pModuleIndex, pPMTHitNumber, G4Threading::G4GetThreadId());

    // Create ROOT output structure
    PhotonInfoROOT info;
    info.eventID = p_eventid;
    info.hitTime = p_globalTime;
    info.entryTime = p_entryTime;
    info.flightTime = p_localTime;
    info.pathLenght = p_trackLength;
    info.generationDetectionDistance = p_distance;
    info.energy = p_energy;
    info.PMTnr = pPMTHitNumber;
    info.dir_x = pMomentumDirection.x();
    info.dir_y = pMomentumDirection.y();
    info.dir_z = pMomentumDirection.z();
    info.localPos_x = pLocalPos.x();
    info.localPos_y = pLocalPos.y();
    info.localPos_z = pLocalPos.z();
    info.globalPos_x = pGlobalPos.x();
    info.globalPos_y = pGlobalPos.y();
    info.globalPos_z = pGlobalPos.z();
    info.deltaPos_x = p_distance;
    info.deltaPos_y = 0;
    info.deltaPos_z = 0;
    info.wavelength = p_wavelength;
    info.parentID = pParentID;
    info.parentType = pParentType;
    info.photonOrigin = pPhotonOrigin;
    info.parentProcess = pParentProcess;
    info.nPhotoelectrons = 0;

    // Write to ROOT file
    if (m_rootManager) {
        m_rootManager->FillPhotonInfo(info);
    } else {
        log_error("ROOTHitManager not initialized!");
    }
}

void OMSimHitManager::setNumberOfPMTs(int pNumberOfPMTs, int pModuleIndex)
{
    log_trace("Setting {} PMTs for module {}", pNumberOfPMTs, pModuleIndex);
    m_numberOfPMTs[pModuleIndex] = pNumberOfPMTs;
}

HitStats OMSimHitManager::getMergedHitsOfModule(int pModuleIndex)
{
    return m_moduleHits[pModuleIndex];
}

HitStats OMSimHitManager::getSingleThreadHitsOfModule(int pModuleIndex)
{
    log_debug("Getting thread data for module {} (thread {})", pModuleIndex, G4Threading::G4GetThreadId());
    return m_threadData->moduleHits[pModuleIndex];
}

bool OMSimHitManager::areThereHitsInModuleSingleThread(int pModuleIndex)
{
    if (!m_threadData) return false;
    return m_threadData->moduleHits.find(pModuleIndex) != m_threadData->moduleHits.end();
}

void OMSimHitManager::reset()
{
    log_trace("Resetting hit manager");
    if (m_threadData) {
        m_threadData->moduleHits.clear();
        delete m_threadData;
        m_threadData = nullptr;
    }
    m_moduleHits.clear();
}

std::vector<double> OMSimHitManager::countMergedHits(int pModuleIndex, bool p_getWeightedDE)
{
    HitStats hitsOfModule = m_moduleHits[pModuleIndex];
    G4int numberOfPMTs = m_numberOfPMTs[pModuleIndex];

    std::vector<double> hits(numberOfPMTs + 1, 0.0);
    for (int i = 0; i < (int)hitsOfModule.PMTnr.size(); i++) {
        double newcount = p_getWeightedDE ? hitsOfModule.PMTresponse.at(i).detectionProbability : 1;
        hits[hitsOfModule.PMTnr.at(i)] += newcount;
        hits[numberOfPMTs] += newcount;
    }
    return hits;
}

void OMSimHitManager::sortHitStatsByTime(HitStats& p_hits)
{
    std::vector<std::size_t> indices(p_hits.hitTime.size());
    std::iota(indices.begin(), indices.end(), 0);

    std::sort(indices.begin(), indices.end(),
        [&p_hits](std::size_t a, std::size_t b) {
            return p_hits.hitTime[a] < p_hits.hitTime[b];
        });

    applyPermutation(p_hits.eventId, indices);
    applyPermutation(p_hits.hitTime, indices);
    applyPermutation(p_hits.entryTime, indices);
    applyPermutation(p_hits.flightTime, indices);
    applyPermutation(p_hits.pathLenght, indices);
    applyPermutation(p_hits.energy, indices);
    applyPermutation(p_hits.PMTnr, indices);
    applyPermutation(p_hits.direction, indices);
    applyPermutation(p_hits.localPosition, indices);
    applyPermutation(p_hits.globalPosition, indices);
    applyPermutation(p_hits.generationDetectionDistance, indices);
    applyPermutation(p_hits.PMTresponse, indices);
    applyPermutation(p_hits.photonOrigin, indices);
    applyPermutation(p_hits.parentID, indices);
    applyPermutation(p_hits.parentType, indices);
    applyPermutation(p_hits.parentProcess, indices);
}

std::vector<int> OMSimHitManager::calculateMultiplicity(const G4double pTimeWindow, int pModuleNumber)
{
    HitStats hitsOfModule = m_moduleHits[pModuleNumber];
    G4int numberOfPMTs = m_numberOfPMTs[pModuleNumber];

    sortHitStatsByTime(hitsOfModule);

    std::vector<int> multiplicity(numberOfPMTs, 0);
    std::size_t skipUntil = 0;
    int vectorSize = hitsOfModule.hitTime.size();

    for (std::size_t i = 0; i < vectorSize - 1; ++i) {
        if (i < skipUntil) continue;

        std::vector<int> hitPMT(numberOfPMTs, 0);
        hitPMT[hitsOfModule.PMTnr.at(i)] = 1;
        int currentSum = 0;

        for (std::size_t j = i + 1; j < vectorSize; ++j) {
            if ((hitsOfModule.hitTime.at(j) - hitsOfModule.hitTime.at(i)) > pTimeWindow) {
                skipUntil = j;
                break;
            } else {
                hitPMT[hitsOfModule.PMTnr.at(j)] = 1;
            }
        }

        for (std::size_t k = 0; k < numberOfPMTs; ++k) {
            currentSum += hitPMT[k];
        }
        multiplicity[currentSum - 1] += 1;
    }
    return multiplicity;
}

void OMSimHitManager::mergeThreadData()
{
    log_trace("Merging thread data");
    G4AutoLock lock(&m_mutex);

    if (m_threadData) {
        log_debug("Merging data for thread {}", G4Threading::G4GetThreadId());

        for (const auto& [moduleIndex, hits] : m_threadData->moduleHits) {
            auto& globalHits = m_moduleHits[moduleIndex];

            globalHits.eventId.insert(globalHits.eventId.end(), hits.eventId.begin(), hits.eventId.end());
            globalHits.hitTime.insert(globalHits.hitTime.end(), hits.hitTime.begin(), hits.hitTime.end());
            globalHits.entryTime.insert(globalHits.entryTime.end(), hits.entryTime.begin(), hits.entryTime.end());
            globalHits.flightTime.insert(globalHits.flightTime.end(), hits.flightTime.begin(), hits.flightTime.end());
            globalHits.pathLenght.insert(globalHits.pathLenght.end(), hits.pathLenght.begin(), hits.pathLenght.end());
            globalHits.energy.insert(globalHits.energy.end(), hits.energy.begin(), hits.energy.end());
            globalHits.PMTnr.insert(globalHits.PMTnr.end(), hits.PMTnr.begin(), hits.PMTnr.end());
            globalHits.direction.insert(globalHits.direction.end(), hits.direction.begin(), hits.direction.end());
            globalHits.localPosition.insert(globalHits.localPosition.end(), hits.localPosition.begin(), hits.localPosition.end());
            globalHits.globalPosition.insert(globalHits.globalPosition.end(), hits.globalPosition.begin(), hits.globalPosition.end());
            globalHits.generationDetectionDistance.insert(globalHits.generationDetectionDistance.end(), hits.generationDetectionDistance.begin(), hits.generationDetectionDistance.end());
            globalHits.PMTresponse.insert(globalHits.PMTresponse.end(), hits.PMTresponse.begin(), hits.PMTresponse.end());
            globalHits.photonOrigin.insert(globalHits.photonOrigin.end(), hits.photonOrigin.begin(), hits.photonOrigin.end());
            globalHits.parentID.insert(globalHits.parentID.end(), hits.parentID.begin(), hits.parentID.end());
            globalHits.parentType.insert(globalHits.parentType.end(), hits.parentType.begin(), hits.parentType.end());
            globalHits.parentProcess.insert(globalHits.parentProcess.end(), hits.parentProcess.begin(), hits.parentProcess.end());
        }

        delete m_threadData;
        m_threadData = nullptr;
    }
}
