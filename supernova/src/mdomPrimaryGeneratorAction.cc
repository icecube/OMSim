/** @file mdomPrimaryGeneratorAction.cc
 *  @brief Main primary generator. Chooses between generators 0 (gps), 1 (SN enes), 2 (SN ibd), 3 (solar neutrinos)
 * 
 *  @author Cristian Jesus Lozano Mariscal (c.lozano@wwu.de)
 * 
 *  @version Geant4 10.7
 */

#include "mdomPrimaryGeneratorAction.hh"
#include "mdomPrimaryGeneratorAction0.hh"
#include "mdomPrimaryGeneratorAction1.hh"
#include "mdomPrimaryGeneratorAction2.hh"
#include "mdomPrimaryGeneratorAction3.hh"
#include "mdomPrimaryGeneratorMessenger.hh"

#include "G4Event.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "Randomize.hh"


mdomPrimaryGeneratorAction::mdomPrimaryGeneratorAction()
 : G4VUserPrimaryGeneratorAction(),
   fParticleGun(0),
   fAction1(0),
   fAction2(0),
   fAction3(0),
   fSelectedAction(0), //defaul primary generator
   fGunMessenger(0)
{
  // default particle kinematic
  //
  G4int n_particle = 1;
  fParticleGun  = new G4ParticleGun(n_particle);

  
  fAction0 = new mdomPrimaryGeneratorAction0();
  fAction1 = new mdomPrimaryGeneratorAction1(fParticleGun);
  fAction2 = new mdomPrimaryGeneratorAction2(fParticleGun);
  fAction3 = new mdomPrimaryGeneratorAction3(fParticleGun);
  fGunMessenger = new mdomPrimaryGeneratorMessenger(this);    
  
}



mdomPrimaryGeneratorAction::~mdomPrimaryGeneratorAction()
{
  delete fAction0;
  delete fAction1;
  delete fAction2;
  delete fAction3;
  delete fParticleGun;    
  delete fGunMessenger;      
}



void mdomPrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  switch(fSelectedAction)
  {
   case 0:
    fAction0->GeneratePrimaries(anEvent);
    break;
   case 1:
    fAction1->GeneratePrimaries(anEvent);
    break; 
   case 2:
    fAction2->GeneratePrimaries(anEvent);
    break;
   case 3:
    fAction3->GeneratePrimaries(anEvent);
    break;
   default:
    G4cerr << "Invalid generator fAction" << G4endl;
  }
}

