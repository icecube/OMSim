#include "OMSimPrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4ParticleTypes.hh>



OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	m_particleSource = new G4GeneralParticleSource ();
	m_particleSource->SetParticleDefinition(G4GenericIon::GenericIonDefinition());
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
	delete m_particleSource;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
	m_particleSource->GeneratePrimaryVertex(anEvent);
}





