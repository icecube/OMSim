/**
 * @file OMSimTrackingAction.cc
 * @brief Implementation of enhanced tracking action for WavePID.
 */
#include "OMSimTrackingAction.hh"
#include "G4ParticleDefinition.hh"
#include "G4VProcess.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include <stdexcept>

// Thread-local instance pointer — each Geant4 worker thread gets its own instance
thread_local OMSimTrackingAction* OMSimTrackingAction::instance = nullptr;

OMSimTrackingAction& OMSimTrackingAction::GetInstance()
{
    if (!instance) {
        throw std::runtime_error("OMSimTrackingAction instance not created!");
    }
    return *instance;
}

OMSimTrackingAction::OMSimTrackingAction()
    : G4UserTrackingAction(),
      m_trackIDToParticleTypeMap(),
      m_trackIDToCreatorProcessMap()
{
    if (instance != nullptr) {
        throw std::runtime_error("OMSimTrackingAction is a singleton - only one instance allowed!");
    }
    instance = this;
}

OMSimTrackingAction::~OMSimTrackingAction()
{
    instance = nullptr;
}

void OMSimTrackingAction::PreUserTrackingAction(const G4Track* track)
{
    G4int trackID = track->GetTrackID();
    std::string particleName = track->GetDefinition()->GetParticleName();

    // Store particle type for this track
    m_trackIDToParticleTypeMap[trackID] = particleName;

    // Optional: Energy cut for non-optical particles to improve performance
    // Kills low-energy particles that won't produce detectable Cerenkov light
    if (track->GetDefinition() != G4OpticalPhoton::Definition()) {
        if (track->GetKineticEnergy() < 990 * keV) {
            const_cast<G4Track*>(track)->SetTrackStatus(fStopAndKill);
            return;
        }
    }

    // Store creator process
    const G4VProcess* creatorProcess = track->GetCreatorProcess();
    if (creatorProcess) {
        m_trackIDToCreatorProcessMap[trackID] = creatorProcess->GetProcessName();
    } else {
        m_trackIDToCreatorProcessMap[trackID] = "Primary";
    }
}

void OMSimTrackingAction::PostUserTrackingAction(const G4Track* /*track*/)
{
    // Optional: Could clean up maps here for completed tracks
    // Currently keeping data for potential later analysis
}

std::string OMSimTrackingAction::GetParticleType(G4int trackID) const
{
    auto it = m_trackIDToParticleTypeMap.find(trackID);
    if (it != m_trackIDToParticleTypeMap.end()) {
        return it->second;
    }
    return "Unknown";
}

std::string OMSimTrackingAction::GetCreatorProcess(G4int trackID) const
{
    auto it = m_trackIDToCreatorProcessMap.find(trackID);
    if (it != m_trackIDToCreatorProcessMap.end()) {
        return it->second;
    }
    return "Unknown";
}
