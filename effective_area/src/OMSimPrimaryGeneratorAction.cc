#include "OMSimPrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4ParticleTypes.hh>
#include <G4RandomTools.hh>


OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	mParticleSource = new G4GeneralParticleSource ();
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
	delete mParticleSource;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	mParticleSource->SetParticlePolarization(G4RandomDirection());
	mParticleSource->GeneratePrimaryVertex(anEvent);
}
