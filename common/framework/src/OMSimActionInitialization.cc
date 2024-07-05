#include "OMSimActionInitialization.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimEventAction.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimSteppingAction.hh"
#include <G4RandomTools.hh>

OMSimActionInitialization::OMSimActionInitialization(long pSeed)
    : G4VUserActionInitialization(), mMasterSeed(pSeed)
{
}

OMSimActionInitialization::~OMSimActionInitialization()
{
}

void OMSimActionInitialization::BuildForMaster() const
{
    SetUserAction(new OMSimRunAction);
}

void OMSimActionInitialization::Build() const
{
    SetUserAction(new OMSimPrimaryGeneratorAction);
    SetUserAction(new OMSimRunAction);
    SetUserAction(new OMSimEventAction);
    SetUserAction(new OMSimTrackingAction);
    SetUserAction(new OMSimSteppingAction);
    const long lPrime = 2147483647; 
    long lSeed = (mMasterSeed + (G4Threading::G4GetThreadId()+1) * lPrime);
    lSeed = lSeed % std::numeric_limits<long>::max();
    log_debug("Random engine of thread {} was assigned seed {}", G4Threading::G4GetThreadId(), lSeed);
    G4Random::setTheSeed(lSeed);
    G4Random::setTheEngine(new CLHEP::RanluxEngine);
}