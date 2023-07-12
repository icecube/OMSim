#include "OMSimPrimaryGeneratorAction.hh"


#include "G4Event.hh"
#include "G4GeneralParticleSource.hh"
#include "G4ParticleTypes.hh"



OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	particleSource = new G4GeneralParticleSource ();
	particleSource->SetParticleDefinition(G4GenericIon::GenericIonDefinition());
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
	delete particleSource;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	particleSource->GeneratePrimaryVertex(anEvent);
}
