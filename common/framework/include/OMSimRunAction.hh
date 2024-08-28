#pragma once

#include <G4UserRunAction.hh>
#include "OMSimHitManager.hh"
#include "OMSimLogger.hh"
#include "Randomize.hh"
#include "G4Run.hh" 

class OMSimRunAction : public G4UserRunAction
{
public:
  OMSimRunAction(){};
  ~OMSimRunAction(){};

public:
  void BeginOfRunAction(const G4Run *run) {
  };
  void EndOfRunAction(const G4Run *run) override
  {
    log_debug("EndOfRunAction called, nr of events {}", run->GetNumberOfEvent() );
    OMSimHitManager::getInstance().mergeThreadData();
  }
};
