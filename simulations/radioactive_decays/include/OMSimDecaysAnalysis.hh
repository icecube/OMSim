/**
 * @file
 * @brief Defines the OMSimDecaysAnalysis class  and DecayStats struct for the radioactive decays simulation.
 * @ingroup radioactive
 */

#pragma once
#include <G4ThreeVector.hh>
#include <fstream>
#include "G4AutoLock.hh"

/**
 * @brief A structure to store information about decays.
 * @ingroup radioactive
 */
struct DecayStats
{
    std::vector<G4long> eventId;               ///< ID of the event
    std::vector<G4String> isotopeName;        ///< Isotope name and energy level
    std::vector<G4double> decayTime;          ///< Time of the decay after (possibly) randomising inside simulation time window
    std::vector<G4ThreeVector> decayPosition; ///< Global position of the decay.
};

/**
 * @ingroup radioactive
 * @brief Singleton class responsible for managing, analysing, and saving decay-related data.
 */
class OMSimDecaysAnalysis
{
public:
    /**
     * @brief Returns the instance of OMSimDecaysAnalysis (Singleton pattern).
     */
    static OMSimDecaysAnalysis &getInstance();
    void appendDecay(G4String pParticleName, G4double pDecayTime, G4ThreeVector pDecayPosition);
    void mergeDecayData();
    void writeMultiplicity(G4double pTimeWindow);
    void writeThreadDecayInformation();
    void writeThreadHitInformation();
    void reset();

private:
    G4ThreadLocal static DecayStats *m_threadDecayStats;

    static G4Mutex m_mutex;

    static OMSimDecaysAnalysis* m_instance;
    OMSimDecaysAnalysis() = default;
    ~OMSimDecaysAnalysis() = default;
    OMSimDecaysAnalysis(const OMSimDecaysAnalysis &) = delete;
    OMSimDecaysAnalysis &operator=(const OMSimDecaysAnalysis &) = delete;
};
