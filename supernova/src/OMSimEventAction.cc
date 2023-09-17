#include "OMSimEventAction.hh"
#include "OMSimSNAnalysis.hh"
#include <G4RunManager.hh>

OMSimEventAction::OMSimEventAction()
{}

OMSimEventAction::~OMSimEventAction()
{}

void OMSimEventAction::BeginOfEventAction(const G4Event* evt)
{
	EventInfoManager::getInstance().setCurrentEventID(evt->GetEventID());
}

void OMSimEventAction::EndOfEventAction(const G4Event* evt)
{
//	Analyze event
	G4double Xev = evt->GetPrimaryVertex()->GetX0();
	G4double Yev = evt->GetPrimaryVertex()->GetY0();
	G4double Zev = evt->GetPrimaryVertex()->GetZ0();
        //G4cout << Xev << " " << Yev << " " <<Zev << G4endl;
	G4double Rev = pow(pow(Xev,2)+pow(Yev,2)+pow(Zev,2),1./2.); //Spherical R
	
	G4double Pxev = evt->GetPrimaryVertex()->GetPrimary()->GetPx();
	G4double Pyev = evt->GetPrimaryVertex()->GetPrimary()->GetPy();
	G4double Pzev = evt->GetPrimaryVertex()->GetPrimary()->GetPz();
	G4double Pmod = pow(pow(Pxev,2)+pow(Pyev,2)+pow(Pzev,2),1./2.);
	
	G4double Pxdir = Pxev/Pmod;
	G4double Pydir = Pyev/Pmod;
	G4double Pzdir = Pzev/Pmod;
    OMSimSNAnalysis &lAnalysisManager = OMSimSNAnalysis::getInstance();
	lAnalysisManager.primaryX = Xev;
	lAnalysisManager.primaryY = Yev;
	lAnalysisManager.primaryZ = Zev;
	lAnalysisManager.primaryR= Rev;
	lAnalysisManager.primaryDirX = Pxdir;
	lAnalysisManager.primaryDirY = Pydir;
	lAnalysisManager.primaryDirZ = Pzdir;
	
	//lAnalysisManager.AnalyzeEvent();
	lAnalysisManager.Writer_InfoFile();
}
