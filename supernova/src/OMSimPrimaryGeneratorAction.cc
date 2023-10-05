/** @file OMSimPrimaryGeneratorAction.cc
 *  @brief Main primary generator. Chooses between generators 0 (IBD), 1 (ENES), 2 (SN ibd)
 * 
 *  @author Cristian Jesus Lozano Mariscal
 */

#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimIBD.hh"
#include "OMSimENES.hh"
#include "OMSimPrimaryGeneratorMessenger.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"


OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(),
   fParticleGun(0),
   fAction0(0),
   fAction1(0),
   fSelectedAction(0), //defaul primary generator
   fGunMessenger(0)
{
  G4int n_particle = 1;
  fParticleGun  = new G4ParticleGun(n_particle);

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



void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  switch(fSelectedAction)
  {
    case 0:
      fAction0->GeneratePrimaries(anEvent);
      break;
    case 1:
      fAction1->GeneratePrimaries(anEvent);
      break; 
    default:
      G4cerr << "Invalid generator fAction" << G4endl;
  }
}

