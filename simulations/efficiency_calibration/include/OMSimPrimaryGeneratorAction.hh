#pragma once
 
#include <G4VUserPrimaryGeneratorAction.hh>
#include <G4AutoLock.hh>
class G4GeneralParticleSource;
class G4Event;
class OMSimPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
	OMSimPrimaryGeneratorAction();
	~OMSimPrimaryGeneratorAction();

public:
	void GeneratePrimaries(G4Event* anEvent) override;

private:
    static thread_local std::unique_ptr<G4GeneralParticleSource> m_particleSource;
	static G4Mutex m_mutex;
};


