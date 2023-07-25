// OMSim.h

#ifndef OMSIM_H
#define OMSIM_H

#include "OMSimDetectorConstruction.hh"
#include "OMSimPhysicsList.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimEventAction.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimSteppingAction.hh"
#include "OMSimAnalysisManager.hh"
#include "OMSimPMTResponse.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimUIinterface.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4ThreeVector.hh"
#include "G4Navigator.hh"
#include "G4UItcsh.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"

#include <ctime>
#include <sys/time.h>
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <filesystem>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <TGraph.h>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

class OMSim
{
public:
    OMSim();
    ~OMSim();

    void ensure_output_directory_exists(const std::string &filepath);
    void initialiseSimulation();
    po::options_description mGeneralArgs;

private:
    G4RunManager *mRunManager;
    G4VisExecutive *mVisManager;
    G4Navigator *mNavigator;

    OMSimDetectorConstruction *mDetector = nullptr;
    G4VUserPhysicsList *mPhysics = nullptr;
    G4VUserPrimaryGeneratorAction *mGenAction = nullptr;
    G4UserRunAction *mRunAction = nullptr;
    G4UserEventAction *mEventAction = nullptr;
    G4UserTrackingAction *mTracking = nullptr;
    G4UserSteppingAction *mStepping = nullptr;
    G4TouchableHistory *mHistory = nullptr;
};

#endif // OMSIM_H
