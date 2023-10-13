#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimSNAnalysis.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <OMSimInputData.hh>

OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(),
      fParticleGun(0),
      fAction0(0),
      fAction1(0),
      fSelectedAction(0), // defaul primary generator
      fGunMessenger(0)
{
    G4int n_particle = 1;
    fParticleGun = new G4ParticleGun(n_particle);

    fAction0 = new OMSimIBD(fParticleGun);
    fAction1 = new OMSimENES(fParticleGun);
    fGunMessenger = new OMSimPrimaryGeneratorMessenger(this);
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
    delete fAction0;
    delete fAction1;
    delete fParticleGun;
    delete fGunMessenger;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event *anEvent)
{
    switch (fSelectedAction)
    {
    case 0:
        fAction0->GeneratePrimaries(anEvent);
        break;
    case 1:
        fAction1->GeneratePrimaries(anEvent);
        break;
    default:
        log_error("Invalid generator fAction");
        G4cerr << "Invalid generator fAction" << G4endl;
    }
}
