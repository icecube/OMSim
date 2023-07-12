#ifndef OMSimPrimaryGeneratorAction_h
#define OMSimPrimaryGeneratorAction_h 1
 
#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
 
class G4GeneralParticleSource;
class G4Event;
class OMSimPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
	OMSimPrimaryGeneratorAction();
	~OMSimPrimaryGeneratorAction();

public:
	void GeneratePrimaries(G4Event* anEvent);

private:
	G4GeneralParticleSource* particleSource;
};

#endif
