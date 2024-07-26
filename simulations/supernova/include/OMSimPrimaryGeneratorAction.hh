#ifndef OMSimPrimaryGeneratorAction_h
#define OMSimPrimaryGeneratorAction_h 1


#include "OMSimSNParticleGenerators.hh"
#include "OMSimPrimaryGeneratorMessenger.hh"
#include "OMSimSNTools.hh"

#include <globals.hh>
#include <vector>
#include <G4PhysicalConstants.hh>
#include <G4SystemOfUnits.hh>
#include <G4VUserPrimaryGeneratorAction.hh>

class G4ParticleGun;
class G4Event;


class OMSimPrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    OMSimPrimaryGeneratorAction();    
   ~OMSimPrimaryGeneratorAction();

  public:
    virtual void GeneratePrimaries(G4Event*);

  public:
    G4ParticleGun* GetParticleGun() { return fParticleGun; };
    
    void SelectAction(G4int i) { fSelectedAction = i; };    
    G4int GetSelectedAction()  { return fSelectedAction; };

    OMSimIBD*  GetAction0() { return fAction0; };
    OMSimENES*  GetAction1() { return fAction1; };
      
    
  private:
    G4ParticleGun*           fParticleGun;


    OMSimIBD* fAction0;
    OMSimENES* fAction1;
    G4int                    fSelectedAction;
    OMSimPrimaryGeneratorMessenger* fGunMessenger;     
       
};

#endif


