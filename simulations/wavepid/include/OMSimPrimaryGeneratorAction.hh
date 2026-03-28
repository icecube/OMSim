/**
 * @file OMSimPrimaryGeneratorAction.hh
 * @brief Primary generator action for WavePID simulation.
 */
#pragma once

#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4AutoLock.hh>
#include <G4ThreeVector.hh>
#include <memory>

class G4ParticleGun;
class G4GeneralParticleSource;
class G4Event;

/**
 * @class OMSimPrimaryGeneratorAction
 * @brief Generates primary particles for WavePID simulation.
 *
 * Supports two modes:
 * 1. Impact parameter mode: Uses G4ParticleGun with position calculated
 *    to ensure Cherenkov cone hits DOM at origin.
 * 2. Macro mode: Uses G4GeneralParticleSource controlled via macro commands.
 */
class OMSimPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
    OMSimPrimaryGeneratorAction();
    ~OMSimPrimaryGeneratorAction();

    void GeneratePrimaries(G4Event* anEvent) override;

private:
    void setupParticleGun();

    static thread_local std::unique_ptr<G4ParticleGun> m_particleGun;
    static thread_local std::unique_ptr<G4GeneralParticleSource> m_particleSource;
    static G4Mutex m_mutex;

    bool m_useParticleGun;  ///< True if using impact parameter mode
    G4ThreeVector m_position;
    G4ThreeVector m_direction;
};
