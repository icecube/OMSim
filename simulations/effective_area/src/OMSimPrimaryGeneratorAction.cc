#include "OMSimPrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4ParticleTypes.hh>
#include <G4RandomTools.hh>

thread_local std::unique_ptr<G4GeneralParticleSource> OMSimPrimaryGeneratorAction::m_particleSource;
G4Mutex OMSimPrimaryGeneratorAction::m_mutex;
OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	if (!m_particleSource)
	{
	m_particleSource = std::make_unique<G4GeneralParticleSource>();
	}
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{

}
 
void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event *p_event)
{
	if (m_particleSource)
	{
		std::lock_guard<G4Mutex> lock(m_mutex);
		m_particleSource->SetParticlePolarization(G4RandomDirection());
		m_particleSource->GeneratePrimaryVertex(p_event);
	}
}
