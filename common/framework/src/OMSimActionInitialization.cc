#include "OMSimActionInitialization.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimEventAction.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimSteppingAction.hh"
#include <G4RandomTools.hh>

OMSimActionInitialization::OMSimActionInitialization()
    : G4VUserActionInitialization()
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
}