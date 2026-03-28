/**
 * @file OMSimPhysicsList.hh
 * @brief Physics list for WavePID simulation with muon and Cerenkov physics.
 *
 * Includes muons, electrons, gammas, and optical photons with full Cerenkov
 * production for photon origin tracking studies.
 */
#pragma once

#include <G4VUserPhysicsList.hh>

class G4VPhysicsConstructor;
class G4ProductionCuts;

/**
 * @class OMSimPhysicsList
 * @brief Custom physics list for WavePID photon origin tracking.
 *
 * This physics list initializes particles and processes needed for
 * tracking Cerenkov radiation from muons and electrons in ice.
 */
class OMSimPhysicsList: public G4VUserPhysicsList
{
public:
    OMSimPhysicsList();
    ~OMSimPhysicsList() = default;

protected:
    void ConstructParticle() override;
    void ConstructProcess() override;
    void SetCuts() override;
};
