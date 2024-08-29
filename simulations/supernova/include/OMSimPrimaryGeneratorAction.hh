#pragma once

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
    G4ParticleGun* GetParticleGun() { return m_particleGun; };
    
    void SelectAction(G4int i) { m_selectedAction = i; };    
    G4int GetSelectedAction()  { return m_selectedAction; };

    OMSimIBD*  GetAction0() { return m_action0; };
    OMSimENES*  GetAction1() { return m_action1; };
      
    
  private:
    G4ParticleGun*           m_particleGun;


    OMSimIBD* m_action0;
    OMSimENES* m_action1;
    G4int                    m_selectedAction;
    OMSimPrimaryGeneratorMessenger* m_gunMessenger;     
       
};


