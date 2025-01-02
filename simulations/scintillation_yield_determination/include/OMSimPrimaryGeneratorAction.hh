/**
 * @file
 * @brief Defines a simple OMSimPrimaryGeneratorAction class for the radioactive decays simulation using GPS.
 * @ingroup radioactive
 */
#ifndef OMSimPrimaryGeneratorAction_h
#define OMSimPrimaryGeneratorAction_h 1
 
#include <G4VUserPrimaryGeneratorAction.hh>
 
class G4GeneralParticleSource;
class G4Event;

/**
 * @class
 * @brief OMSimPrimaryGeneratorAction class for the radioactive decays simulation using only GPS.
 * @ingroup radioactive
 */
class OMSimPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
public:
	OMSimPrimaryGeneratorAction();
	~OMSimPrimaryGeneratorAction();

public:
	void GeneratePrimaries(G4Event* anEvent);

private:
	G4GeneralParticleSource* lParticleSource;
};

#endif
