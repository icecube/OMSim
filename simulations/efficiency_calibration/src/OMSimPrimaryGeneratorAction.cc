#include "OMSimPrimaryGeneratorAction.hh"

#include <G4GeneralParticleSource.hh>
#include <G4ParticleTypes.hh>
#include <G4RandomTools.hh>

thread_local std::unique_ptr<G4GeneralParticleSource> OMSimPrimaryGeneratorAction::mParticleSource;
G4Mutex OMSimPrimaryGeneratorAction::mMutex;
OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
{
	if (!mParticleSource)
	{
	mParticleSource = std::make_unique<G4GeneralParticleSource>();
	}
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{

}
 
void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event *anEvent)
{
	if (mParticleSource)
	{
		std::lock_guard<G4Mutex> lock(mMutex);
		mParticleSource->SetParticlePolarization(G4RandomDirection());
		mParticleSource->GeneratePrimaryVertex(anEvent);
	}
}
