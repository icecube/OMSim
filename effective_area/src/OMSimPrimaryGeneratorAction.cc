#include "OMSimPrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4ParticleTypes.hh>



OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	lParticleSource = new G4GeneralParticleSource ();
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
	delete lParticleSource;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	lParticleSource->GeneratePrimaryVertex(anEvent);
}
