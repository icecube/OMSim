#include "OMSimPrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4ParticleTypes.hh>



OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	lParticleSource = new G4GeneralParticleSource ();
	lParticleSource->SetParticleDefinition(G4GenericIon::GenericIonDefinition());
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
	delete lParticleSource;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	lParticleSource->GeneratePrimaryVertex(anEvent);
}
