#ifndef OMSimRunAction_h
#define OMSimRunAction_h 1

#include "G4UserRunAction.hh"
#include "globals.hh"

class G4Run;

class OMSimRunAction : public G4UserRunAction
{
public:
  OMSimRunAction();
  ~OMSimRunAction();

public:
	void BeginOfRunAction(const G4Run*);
	void EndOfRunAction(const G4Run*);

private:
  double startingtime;
};

#endif
