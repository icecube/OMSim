#include "OMSimPhysicsList.hh"
#include "G4ProcessManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4Cerenkov.hh"
#include "G4OpRayleigh.hh"
#include "G4OpMieHG.hh"
#include "G4eplusAnnihilation.hh"

#include "G4LivermoreIonisationModel.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4LivermoreGammaConversionModel.hh"
#include "G4LivermoreComptonModel.hh"

#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"

#include "G4OpAbsorption.hh"

#include "OMSimOpBoundaryProcess.hh"


OMSimPhysicsList::OMSimPhysicsList():  G4VUserPhysicsList()
{
	defaultCutValue = 0.1*um;
	SetVerboseLevel(0);
}

OMSimPhysicsList::~OMSimPhysicsList()
{
}

void OMSimPhysicsList::ConstructParticle()
{
	G4Gamma::GammaDefinition();
	G4OpticalPhoton::OpticalPhotonDefinition();
    G4Electron::ElectronDefinition();
    G4Positron::PositronDefinition();
}

void OMSimPhysicsList::ConstructProcess()
{
	AddTransportation();

//	The Cherenkov process
	G4Cerenkov* theCerenkovProcess = new G4Cerenkov("Cerenkov");
    theCerenkovProcess->SetTrackSecondariesFirst(true);
    theCerenkovProcess->SetMaxBetaChangePerStep(10.0);
    theCerenkovProcess->SetMaxNumPhotonsPerStep(300); 

//	The Livermore models
	G4ComptonScattering* theComptonScattering = new G4ComptonScattering();
	G4LivermoreComptonModel* theLivermoreComptonModel = new G4LivermoreComptonModel();
	theComptonScattering->SetEmModel(theLivermoreComptonModel);
    
	G4eIonisation* theIonizationModel = new G4eIonisation();
	theIonizationModel->SetEmModel(new G4LivermoreIonisationModel());

	G4PhotoElectricEffect* thePhotoElectricEffectModel = new G4PhotoElectricEffect();
	thePhotoElectricEffectModel->SetEmModel(new G4LivermorePhotoElectricModel());

	G4GammaConversion* theGammaConversionModel = new G4GammaConversion();
	theGammaConversionModel->SetEmModel(new G4LivermoreGammaConversionModel());
	
//	Now assign processes to generated particles
    auto theParticleIterator=GetParticleIterator(); //new geant4
	theParticleIterator->reset();
	while( (*theParticleIterator)() ){
		
		G4ParticleDefinition* particle = theParticleIterator->value();
		G4ProcessManager* pmanager = particle->GetProcessManager();
		G4String particleName = particle->GetParticleName();
		G4double particleMass = particle->GetPDGMass();
		G4String particleType = particle->GetParticleType();
		G4double particleCharge = particle->GetPDGCharge();
		
		if (particleName == "opticalphoton") {
			pmanager->AddDiscreteProcess(new G4OpAbsorption());
			pmanager->AddDiscreteProcess(new G4OpBoundaryProcess());
	// 		pmanager->AddDiscreteProcess(new G4OpRayleigh);
			pmanager->AddDiscreteProcess(new G4OpMieHG);
		}	
		else if (particleName == "gamma") {
			pmanager->AddDiscreteProcess(theGammaConversionModel);
			pmanager->AddDiscreteProcess(theComptonScattering);
			//pmanager->AddDiscreteProcess(new G4ComptonScattering());
			pmanager->AddDiscreteProcess(thePhotoElectricEffectModel);
		}
		else if (particleName == "e-") {
			pmanager->AddProcess(new G4eMultipleScattering(),-1,1,1);
			pmanager->AddProcess(theIonizationModel,-1,2,2);
			pmanager->AddProcess(new G4eBremsstrahlung(),-1,-1,3);
		}
		else if (particleName == "e+") {
			pmanager->AddProcess(new G4eMultipleScattering(),-1,1,1);
			pmanager->AddProcess(new G4eIonisation,-1,2,2); // The livermore ionization model is only aplicable to electrons
			pmanager->AddProcess(new G4eBremsstrahlung(),-1,-1,3);
			pmanager->AddProcess(new G4eplusAnnihilation, 0,-1, 4);
		}

		if (theCerenkovProcess->IsApplicable(*particle)) {
  			pmanager->AddProcess(theCerenkovProcess);
			pmanager->SetProcessOrdering(theCerenkovProcess, idxPostStep);
        }

	}
	
}

void OMSimPhysicsList::SetCuts()
{
	SetCutsWithDefault();
	if (verboseLevel>0) DumpCutValuesTable();
}


