
#include "OMSimPhysicsList.hh"
#include "OMSimOpBoundaryProcess.hh"
#include "G4StepLimiter.hh"
#include "G4Cerenkov.hh"
#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include <G4OpRayleigh.hh>
#include <G4OpMieHG.hh>
#include <G4OpAbsorption.hh>
#include <G4ProcessManager.hh>
#include <G4SystemOfUnits.hh>
#include "G4MuMultipleScattering.hh"
#include "G4MuIonisation.hh"
#include "G4MuPairProduction.hh"
#include "G4MuBremsstrahlung.hh"
#include "G4ParticleTypes.hh"
#include "G4PhysicsListHelper.hh"
#include "G4GammaConversion.hh"
#include "OMSimG4Scintillation.hh"
#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"
#include "G4eplusAnnihilation.hh"
#include "G4ComptonScattering.hh"
#include "G4PhotoElectricEffect.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4LivermoreGammaConversionModel.hh"
#include "G4ionIonisation.hh"
#include "G4hMultipleScattering.hh"

OMSimPhysicsList::OMSimPhysicsList():  G4VUserPhysicsList()
{
	defaultCutValue = 1 * um;
	SetVerboseLevel(0);
}


void OMSimPhysicsList::ConstructParticle()
{	
	G4Gamma::GammaDefinition();
	G4OpticalPhoton::OpticalPhotonDefinition();
    G4Electron::ElectronDefinition();
    G4Positron::PositronDefinition();
    G4MuonMinus::MuonMinusDefinition();
    G4MuonPlus::MuonPlusDefinition();
	G4GenericIon::GenericIonDefinition();
	G4Positron::PositronDefinition();
	G4NeutrinoE::NeutrinoEDefinition();
	G4AntiNeutrinoE::AntiNeutrinoEDefinition();
	G4NeutrinoMu::NeutrinoMuDefinition();
	G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();

}

void OMSimPhysicsList::ConstructProcess()
{
	AddTransportation();

//	The Cherenkov process
	G4Cerenkov* theCerenkovProcess = new G4Cerenkov("Cerenkov");
    theCerenkovProcess->SetTrackSecondariesFirst(true);
    theCerenkovProcess->SetMaxBetaChangePerStep(10.0);
    theCerenkovProcess->SetMaxNumPhotonsPerStep(10000); 
	theCerenkovProcess->SetVerboseLevel(0);

	//	Scintillation Process
	OMSimG4Scintillation *theScintProcess = new OMSimG4Scintillation("Scintillation");
	theScintProcess->SetTrackSecondariesFirst(false);
	theScintProcess->SetVerboseLevel(0);
	
	G4GammaConversion* theGammaConversionModel = new G4GammaConversion();
    theGammaConversionModel->SetEmModel(new G4LivermoreGammaConversionModel());


	G4eIonisation* theIonizationModel = new G4eIonisation();
	theIonizationModel->SetEmModel(new G4LivermoreIonisationModel());

	G4PhotoElectricEffect* thePhotoElectricEffectModel = new G4PhotoElectricEffect();
    thePhotoElectricEffectModel->SetEmModel(new G4LivermorePhotoElectricModel());

    auto theParticleIterator=GetParticleIterator();
    theParticleIterator->reset();
	while( (*theParticleIterator)() ){
		
		G4ParticleDefinition* particle = theParticleIterator->value();
		G4ProcessManager* pmanager = particle->GetProcessManager();
		G4String particleName = particle->GetParticleName();
	
		if (particleName == "opticalphoton") {
			pmanager->AddDiscreteProcess(new G4OpAbsorption());
			pmanager->AddDiscreteProcess(new G4OpBoundaryProcess());
			pmanager->AddDiscreteProcess(new G4OpRayleigh);
			pmanager->AddDiscreteProcess(new G4OpMieHG);
		}	
        else if (particleName == "mu-") {
        	pmanager->AddProcess(new G4MuMultipleScattering,-1,1,-1);
            pmanager->AddProcess(new G4MuIonisation,      -1, 2,2);
            pmanager->AddProcess(new G4MuBremsstrahlung,  -1, 3,3);
            pmanager->AddProcess(new G4StepLimiter,       -1,-1,5);
            pmanager->AddProcess(new G4MuPairProduction(),-1,-1, 3);
        }
        else if (particleName == "mu+") {
            pmanager->AddProcess(new G4MuMultipleScattering(),-1,1,-1);
            pmanager->AddProcess(new G4MuIonisation(),      -1, 2,2);
            pmanager->AddProcess(new G4MuBremsstrahlung(),  -1, 3,3);
            pmanager->AddProcess(new G4StepLimiter(),       -1,-1,5);
            pmanager->AddProcess(new G4MuPairProduction(),-1,-1, 3);
        }
		else if (particleName == "e-") {
			pmanager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
			pmanager->AddProcess(theIonizationModel, -1, 2, 2);
			pmanager->AddProcess(new G4eBremsstrahlung(), -1, 3, 3);

		}
		else if (particleName == "e+") {
			pmanager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
			pmanager->AddProcess(new G4eIonisation(), -1, 2, 2);
			pmanager->AddProcess(new G4eBremsstrahlung(), -1, 3, 3);
			pmanager->AddProcess(new G4eplusAnnihilation(), 0, -1, 4);
		}
		else if (particleName == "gamma") {
			pmanager->AddDiscreteProcess(new G4ComptonScattering());
			pmanager->AddDiscreteProcess(theGammaConversionModel);
			pmanager->AddDiscreteProcess(thePhotoElectricEffectModel); 
		}
		else if (particleName == "GenericIon") {
			pmanager->AddProcess(new G4hMultipleScattering(), -1, 1, 1);
			pmanager->AddProcess(new G4ionIonisation(), -1, 2, 2);
			pmanager->AddProcess(new G4StepLimiter(), -1, -1, 3);
		}

		if (theCerenkovProcess->IsApplicable(*particle)) {
			pmanager->AddProcess(theCerenkovProcess);
			pmanager->SetProcessOrdering(theCerenkovProcess, idxPostStep);
		}
		if (theScintProcess->IsApplicable(*particle)) {
			pmanager->AddProcess(theScintProcess);
			pmanager->SetProcessOrdering(theScintProcess, idxPostStep);
		}

	}
}

void OMSimPhysicsList::SetCuts()
{
	SetCutsWithDefault();
	if (verboseLevel>0) DumpCutValuesTable();
}
