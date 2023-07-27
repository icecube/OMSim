#include "OMSimPhysicsList.hh"

#include <G4OpRayleigh.hh>
#include <G4OpMieHG.hh>
#include <G4OpAbsorption.hh>
#include <G4OpBoundaryProcess.hh>
#include <G4ProcessManager.hh>
#include <G4SystemOfUnits.hh>

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


