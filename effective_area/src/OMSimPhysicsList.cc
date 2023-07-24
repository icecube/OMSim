#include "OMSimPhysicsList.hh"
#include "G4ProcessManager.hh"
#include "G4ParticleTypes.hh"
#include "G4StepLimiter.hh"
#include "G4SystemOfUnits.hh"
#include "G4ComptonScattering.hh"
#include "G4GammaConversion.hh"
#include "G4PhotoElectricEffect.hh"

#include "G4RadioactiveDecay.hh"
#include "G4DecayPhysics.hh"
#include "G4Cerenkov.hh"
#include "G4OpRayleigh.hh"
#include "G4OpMieHG.hh"
#include "G4eplusAnnihilation.hh"


#include "G4LivermoreIonisationModel.hh"
#include "G4LivermorePhotoElectricModel.hh"
#include "G4LivermoreIonisationModel.hh"
#include "G4LivermoreGammaConversionModel.hh"

#include "G4eMultipleScattering.hh"
#include "G4eIonisation.hh"
#include "G4eBremsstrahlung.hh"

#include "G4OpAbsorption.hh"
#include "G4OpBoundaryProcess.hh"


OMSimPhysicsList::OMSimPhysicsList():  G4VUserPhysicsList()
{
	defaultCutValue = 0.1*mm;
	SetVerboseLevel(0);
}

OMSimPhysicsList::~OMSimPhysicsList()
{
}

void OMSimPhysicsList::ConstructParticle()
{
	G4OpticalPhoton::OpticalPhotonDefinition();
}

void OMSimPhysicsList::ConstructProcess()
{
	AddTransportation();
	
//	Now assign processes to generated particles
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

	}
	
}

void OMSimPhysicsList::SetCuts()
{
	SetCutsWithDefault();
	if (verboseLevel>0) DumpCutValuesTable();
}


