#include "OMSimRunAction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"

#include <ctime>
#include <sys/time.h>

#include "OMSimAnalysisManager.hh"
#include <time.h>
#include <sys/time.h>
extern G4String	ghitsfilename;
extern G4String	gHittype;
extern OMSimAnalysisManager gAnalysisManager;


OMSimRunAction::OMSimRunAction(){}
OMSimRunAction::~OMSimRunAction(){}

void OMSimRunAction::BeginOfRunAction(const G4Run*)
{  startingtime = clock() / CLOCKS_PER_SEC;
	
	
}

void OMSimRunAction::EndOfRunAction(const G4Run*)
{

gAnalysisManager.datafile.open(ghitsfilename.c_str(), std::ios::out|std::ios::app);

	if (gHittype == "individual") {
		gAnalysisManager.Write(); // for K40 analysis
	}
	if (gHittype == "collective") {
		gAnalysisManager.WriteAccept(); // mainly for acceptance 
	}
	
// 	Close output data file
gAnalysisManager.datafile.close();
gAnalysisManager.Reset();
//double finishtime=clock() / CLOCKS_PER_SEC;
//G4cout << "Computation time: " << finishtime-startingtime << " seconds." << G4endl;
}

