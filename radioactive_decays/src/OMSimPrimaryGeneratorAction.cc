#include "OMSimPrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4ParticleTypes.hh>



OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	mParticleSource = new G4GeneralParticleSource ();
	mParticleSource->SetParticleDefinition(G4GenericIon::GenericIonDefinition());
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
	delete mParticleSource;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	mParticleSource->GeneratePrimaryVertex(anEvent);
}





