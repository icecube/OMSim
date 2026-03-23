/**
 * @file OMSimTrackingAction.hh
 * @brief Enhanced tracking action for WavePID photon origin tracking.
 *
 * This version stores particle type and creator process for all tracks,
 * enabling photon origin classification (e.g., "Cerenkov from Muon").
 */
#pragma once

#include "G4UserTrackingAction.hh"
#include "G4Track.hh"
#include <unordered_map>
#include <string>

/**
 * @class OMSimTrackingAction
 * @brief Tracks particle information for photon origin classification.
 *
 * This singleton class stores track ID to particle type and creator process
 * mappings, which are used by OMSimSensitiveDetector to classify photon origins.
 */
class OMSimTrackingAction : public G4UserTrackingAction
{
public:
    OMSimTrackingAction();
    virtual ~OMSimTrackingAction();

    virtual void PreUserTrackingAction(const G4Track* track) override;
    virtual void PostUserTrackingAction(const G4Track* track) override;

    /**
     * @brief Get the particle type for a given track ID.
     * @param trackID The track ID to look up.
     * @return The particle name (e.g., "mu-", "e-") or "Unknown" if not found.
     */
    std::string GetParticleType(G4int trackID) const;

    /**
     * @brief Get the creator process for a given track ID.
     * @param trackID The track ID to look up.
     * @return The process name (e.g., "muIoni", "Cerenkov") or "Unknown" if not found.
     */
    std::string GetCreatorProcess(G4int trackID) const;

    /**
     * @brief Access the singleton instance.
     * @return Reference to the singleton instance.
     * @throws std::runtime_error if instance not yet created.
     */
    static OMSimTrackingAction& GetInstance();

    /**
     * @brief Check if the singleton instance exists.
     * @return true if instance exists, false otherwise.
     */
    static bool HasInstance() { return instance != nullptr; }

private:
    std::unordered_map<G4int, std::string> m_trackIDToParticleTypeMap;
    std::unordered_map<G4int, std::string> m_trackIDToCreatorProcessMap;

    static thread_local OMSimTrackingAction* instance;  ///< Thread-local instance pointer
};
