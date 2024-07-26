#include "OMSimPhysicsList.hh"
#include "OMSimG4Scintillation.hh"
#include "OMSimG4RadioactiveDecay.hh"
#include "OMSimOpBoundaryProcess.hh"

#include "G4ProcessManager.hh"
#include "G4ParticleTypes.hh"
#include "G4SystemOfUnits.hh"
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4Cerenkov.hh"
#include "G4OpRayleigh.hh"
#include "G4OpMieHG.hh"
#include "G4eplusAnnihilation.hh"

#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4LivermoreGammaConversionModel.hh"

#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"


#include "G4OpAbsorption.hh"


#include "G4PhysicsListHelper.hh"
#include "G4LossTableManager.hh"
#include "G4IonConstructor.hh"
#include "G4UAtomicDeexcitation.hh"

#include "G4ionIonisation.hh"
#include "G4hMultipleScattering.hh"

OMSimPhysicsList::OMSimPhysicsList() : G4VUserPhysicsList()
{
	defaultCutValue = 0.1 * um;
	SetVerboseLevel(0);
}

void OMSimPhysicsList::ConstructParticle()
{
	G4Gamma::GammaDefinition();
	G4OpticalPhoton::OpticalPhotonDefinition();
	G4GenericIon::GenericIonDefinition(); // !
	G4IonConstructor iConstructor;		  //
	iConstructor.ConstructParticle();	  //
	G4Electron::ElectronDefinition();
	G4Positron::PositronDefinition();
	G4NeutrinoE::NeutrinoEDefinition();
	G4AntiNeutrinoE::AntiNeutrinoEDefinition();
	G4NeutrinoMu::NeutrinoMuDefinition();
	G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();
	G4Proton::ProtonDefinition();
}

void OMSimPhysicsList::ConstructProcess()
{
	AddTransportation();
	G4Cerenkov *theCerenkovProcess = new G4Cerenkov("Cerenkov");
	theCerenkovProcess->SetTrackSecondariesFirst(false);
	theCerenkovProcess->SetMaxBetaChangePerStep(10.0);
	theCerenkovProcess->SetMaxNumPhotonsPerStep(10000);
	theCerenkovProcess->SetVerboseLevel(0);

	G4eIonisation *theIonizationModel = new G4eIonisation();
	theIonizationModel->SetEmModel(new G4LivermoreIonisationModel());
	theIonizationModel->SetVerboseLevel(0);

	G4PhotoElectricEffect *thePhotoElectricEffectModel = new G4PhotoElectricEffect();
	thePhotoElectricEffectModel->SetEmModel(new G4LivermorePhotoElectricModel());

	G4GammaConversion *theGammaConversionModel = new G4GammaConversion();
	theGammaConversionModel->SetEmModel(new G4LivermoreGammaConversionModel());
	theGammaConversionModel->SetVerboseLevel(0);
	//	Scintillation Process
	OMSimG4Scintillation *theScintProcess = new OMSimG4Scintillation("Scintillation");
	theScintProcess->SetTrackSecondariesFirst(false);
	theScintProcess->SetVerboseLevel(0);

	G4RadioactiveDecay *theRadioactiveDecay = new G4RadioactiveDecay();
	theRadioactiveDecay->SetVerboseLevel(0);
	// theRadioactiveDecay->SetICM(true); // Internal Conversion
	theRadioactiveDecay->SetARM(true); // Atomic Rearangement

	G4ionIonisation *theBragg = new G4ionIonisation();
	theBragg->SetVerboseLevel(0);

	G4hMultipleScattering *multiple = new G4hMultipleScattering();
	multiple->SetVerboseLevel(0);
	G4PhysicsListHelper *ph = G4PhysicsListHelper::GetPhysicsListHelper();
	ph->SetVerboseLevel(0);

	ph->RegisterProcess(theRadioactiveDecay, G4GenericIon::GenericIon());

	ph->RegisterProcess(theBragg, G4GenericIon::GenericIon());
	ph->RegisterProcess(theBragg, G4Alpha::AlphaDefinition());

	ph->RegisterProcess(multiple, G4Alpha::AlphaDefinition());
	ph->RegisterProcess(multiple, G4GenericIon::GenericIon());

	G4UAtomicDeexcitation *de = new G4UAtomicDeexcitation();
	de->SetFluo(true);
	de->SetAuger(true);
	de->SetPIXE(true);
	G4LossTableManager::Instance()->SetAtomDeexcitation(de);

	auto theParticleIterator = GetParticleIterator();

	theParticleIterator->reset();
	while ((*theParticleIterator)())
	{
		G4ParticleDefinition *particle = theParticleIterator->value();
		G4ProcessManager *pmanager = particle->GetProcessManager();
		G4String particleName = particle->GetParticleName();
		G4double particleMass = particle->GetPDGMass();
		G4String particleType = particle->GetParticleType();
		G4double particleCharge = particle->GetPDGCharge();

		if (particleName == "opticalphoton")
		{
			pmanager->AddDiscreteProcess(new G4OpAbsorption());
			pmanager->AddDiscreteProcess(new G4OpBoundaryProcess());
			pmanager->AddDiscreteProcess(new G4OpRayleigh);
			pmanager->AddDiscreteProcess(new G4OpMieHG);
		}
		else if (particleName == "gamma")
		{
			pmanager->AddDiscreteProcess(theGammaConversionModel);
			pmanager->AddDiscreteProcess(new G4ComptonScattering());
			pmanager->AddDiscreteProcess(thePhotoElectricEffectModel);
		}

		else if (particleName == "e-")
		{
			pmanager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
			pmanager->AddProcess(theIonizationModel, -1, 2, 2);
			pmanager->AddProcess(new G4eBremsstrahlung(), -1, -1, 3);
		}

		else if (particleName == "e+")
		{
			pmanager->AddProcess(new G4eMultipleScattering(), -1, 1, 1);
			pmanager->AddProcess(new G4eIonisation, -1, 2, 2); // The livermore ionization model is only aplicable to electrons
			pmanager->AddProcess(new G4eBremsstrahlung(), -1, -1, 3);
			pmanager->AddProcess(new G4eplusAnnihilation, 0, -1, 4);
		}

		if (theCerenkovProcess->IsApplicable(*particle))
		{
			pmanager->AddProcess(theCerenkovProcess);
			pmanager->SetProcessOrdering(theCerenkovProcess, idxPostStep);
		}

		if (theScintProcess->IsApplicable(*particle))
		{
			pmanager->AddProcess(theScintProcess);
			pmanager->SetProcessOrdering(theScintProcess, idxPostStep);
		}
	}
}

/**
 * @brief Sets the production cuts with default values.
 */
void OMSimPhysicsList::SetCuts()
{
	SetCutsWithDefault();
	if (verboseLevel > 0)
		DumpCutValuesTable();
}
