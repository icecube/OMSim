#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimSNAnalysis.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <OMSimInputData.hh>

OMSimPrimaryGeneratorAction::OMSimPrimaryGeneratorAction()
    : G4VUserPrimaryGeneratorAction(),
      m_particleGun(0),
      m_action0(0),
      m_action1(0),
      m_selectedAction(0), // defaul primary generator
      m_gunMessenger(0)
{
    G4int n_particle = 1;
    m_particleGun = new G4ParticleGun(n_particle);

    m_action0 = new OMSimIBD(m_particleGun);
    m_action1 = new OMSimENES(m_particleGun);
    m_gunMessenger = new OMSimPrimaryGeneratorMessenger(this);
}

OMSimPrimaryGeneratorAction::~OMSimPrimaryGeneratorAction()
{
    delete m_action0;
    delete m_action1;
    delete m_particleGun;
    delete m_gunMessenger;
}

void OMSimPrimaryGeneratorAction::GeneratePrimaries(G4Event *p_event)
{
    switch (m_selectedAction)
    {
    case 0:
        m_action0->GeneratePrimaries(p_event);
        break;
    case 1:
        m_action1->GeneratePrimaries(p_event);
        break;
    default:
        log_error("Invalid generator fAction");
        G4cerr << "Invalid generator fAction" << G4endl;
    }
}
