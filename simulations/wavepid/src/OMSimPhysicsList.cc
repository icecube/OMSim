/**
 * @file OMSimPhysicsList.cc
 * @brief Implementation of physics list for WavePID simulation.
 */
#include "OMSimPhysicsList.hh"
#include "OMSimOpBoundaryProcess.hh"

#include <G4ProcessManager.hh>
#include <G4ParticleTypes.hh>
#include <G4SystemOfUnits.hh>

// Optical processes
#include <G4Cerenkov.hh>
#include <G4OpAbsorption.hh>
#include <G4OpRayleigh.hh>
#include <G4OpMieHG.hh>

// EM processes for electrons/positrons
#include <G4eMultipleScattering.hh>
#include <G4eIonisation.hh>
#include <G4eBremsstrahlung.hh>
#include <G4eplusAnnihilation.hh>

// EM processes for muons
#include <G4MuMultipleScattering.hh>
#include <G4MuIonisation.hh>
#include <G4MuBremsstrahlung.hh>
#include <G4MuPairProduction.hh>

// Gamma processes
#include <G4ComptonScattering.hh>
#include <G4GammaConversion.hh>
#include <G4PhotoElectricEffect.hh>

// Livermore models for better accuracy
#include <G4LivermorePhotoElectricModel.hh>
#include <G4LivermoreIonisationModel.hh>
#include <G4LivermoreGammaConversionModel.hh>

#include <G4PhysicsListHelper.hh>
#include <G4LossTableManager.hh>
#include <G4UAtomicDeexcitation.hh>

OMSimPhysicsList::OMSimPhysicsList() : G4VUserPhysicsList()
{
    defaultCutValue = 0.1 * mm;
    SetVerboseLevel(0);
}

void OMSimPhysicsList::ConstructParticle()
{
    // Optical photons
    G4OpticalPhoton::OpticalPhotonDefinition();

    // Leptons
    G4Electron::ElectronDefinition();
    G4Positron::PositronDefinition();
    G4MuonPlus::MuonPlusDefinition();
    G4MuonMinus::MuonMinusDefinition();

    // Neutrinos (for completeness with muon decay)
    G4NeutrinoE::NeutrinoEDefinition();
    G4AntiNeutrinoE::AntiNeutrinoEDefinition();
    G4NeutrinoMu::NeutrinoMuDefinition();
    G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();

    // Gamma
    G4Gamma::GammaDefinition();

    // Proton (sometimes needed for interactions)
    G4Proton::ProtonDefinition();
}

void OMSimPhysicsList::ConstructProcess()
{
    AddTransportation();

    // Cerenkov process - key for WavePID
    G4Cerenkov* theCerenkovProcess = new G4Cerenkov("Cerenkov");
    theCerenkovProcess->SetTrackSecondariesFirst(true);
    theCerenkovProcess->SetMaxBetaChangePerStep(10.0);
    theCerenkovProcess->SetMaxNumPhotonsPerStep(100);
    theCerenkovProcess->SetVerboseLevel(0);

    // Ionization with Livermore model for electrons
    G4eIonisation* theIonizationModel = new G4eIonisation();
    theIonizationModel->SetEmModel(new G4LivermoreIonisationModel());
    theIonizationModel->SetVerboseLevel(0);

    // Gamma processes with Livermore models
    G4PhotoElectricEffect* thePhotoElectricEffectModel = new G4PhotoElectricEffect();
    thePhotoElectricEffectModel->SetEmModel(new G4LivermorePhotoElectricModel());
    thePhotoElectricEffectModel->SetVerboseLevel(0);

    G4GammaConversion* theGammaConversionModel = new G4GammaConversion();
    theGammaConversionModel->SetEmModel(new G4LivermoreGammaConversionModel());
    theGammaConversionModel->SetVerboseLevel(0);

    // Atomic deexcitation
    G4UAtomicDeexcitation* de = new G4UAtomicDeexcitation();
    de->SetVerboseLevel(0);
    de->SetFluo(true);
    de->SetAuger(true);
    de->SetPIXE(true);
    G4LossTableManager::Instance()->SetAtomDeexcitation(de);

    auto theParticleIterator = GetParticleIterator();
    theParticleIterator->reset();

    while ((*theParticleIterator)())
    {
        G4ParticleDefinition* particle = theParticleIterator->value();
        G4ProcessManager* pmanager = particle->GetProcessManager();
        G4String particleName = particle->GetParticleName();

        if (particleName == "opticalphoton")
        {
            // Optical photon processes
            pmanager->AddDiscreteProcess(new G4OpAbsorption());
            pmanager->AddDiscreteProcess(new G4OpBoundaryProcess());
            pmanager->AddDiscreteProcess(new G4OpRayleigh());
            pmanager->AddDiscreteProcess(new G4OpMieHG());
        }
        else if (particleName == "gamma")
        {
            // Gamma processes
            pmanager->AddDiscreteProcess(theGammaConversionModel);
            pmanager->AddDiscreteProcess(new G4ComptonScattering());
            pmanager->AddDiscreteProcess(thePhotoElectricEffectModel);
        }
        else if (particleName == "e-")
        {
            // Electron processes
            pmanager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
            pmanager->AddProcess(theIonizationModel, -1, 2, 2);
            pmanager->AddProcess(new G4eBremsstrahlung(), -1, -1, 3);
        }
        else if (particleName == "e+")
        {
            // Positron processes
            pmanager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
            pmanager->AddProcess(new G4eIonisation(), -1, 2, 2);
            pmanager->AddProcess(new G4eBremsstrahlung(), -1, -1, 3);
            pmanager->AddProcess(new G4eplusAnnihilation(), 0, -1, 4);
        }
        else if (particleName == "mu+" || particleName == "mu-")
        {
            // Muon processes - critical for WavePID
            pmanager->AddProcess(new G4MuMultipleScattering(), -1, 1, 1);
            pmanager->AddProcess(new G4MuIonisation(), -1, 2, 2);
            pmanager->AddProcess(new G4MuBremsstrahlung(), -1, -1, 3);
            pmanager->AddProcess(new G4MuPairProduction(), -1, -1, 4);
        }

        // Add Cerenkov to all charged particles
        if (theCerenkovProcess->IsApplicable(*particle))
        {
            pmanager->AddProcess(theCerenkovProcess);
            pmanager->SetProcessOrdering(theCerenkovProcess, idxPostStep);
        }
    }
}

void OMSimPhysicsList::SetCuts()
{
    SetCutsWithDefault();
    if (verboseLevel > 0)
        DumpCutValuesTable();
}
