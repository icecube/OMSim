#ifndef OMSimPrimaryGeneratorAction_h
#define OMSimPrimaryGeneratorAction_h 1
 
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
    static thread_local std::unique_ptr<G4GeneralParticleSource> mParticleSource;
	static G4Mutex mMutex;
};

#endif
