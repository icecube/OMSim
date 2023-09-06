#ifndef OMSimPrimaryGeneratorAction_h
#define OMSimPrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"


class OMSimIBD;
class G4ParticleGun;
class G4Event;
class mdomPrimaryGeneratorMessenger;


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
    /*
    OMSimPrimaryGeneratorAction1*  GetAction1() { return fAction1; };
    OMSimPrimaryGeneratorAction2*  GetAction2() { return fAction2; };
    OMSimPrimaryGeneratorAction3*  GetAction3() { return fAction3; };*/
      
    
  private:
    G4ParticleGun*           fParticleGun;


    OMSimIBD* fAction0;
    /*OMSimPrimaryGeneratorAction1* fAction1;
    OMSimPrimaryGeneratorAction2* fAction2;
    OMSimPrimaryGeneratorAction3* fAction3;*/
    G4int                    fSelectedAction;
    mdomPrimaryGeneratorMessenger* fGunMessenger;     
       
};

#endif
