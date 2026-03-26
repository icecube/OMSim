/**
 * @file OMSim.hh
 * @brief Main simulation class for the Optical Module (OM) simulation.
 *
 * WavePID version - includes our enhanced headers with photon origin tracking.
 */

#pragma once

#include "OMSimDetectorConstruction.hh"
#include "OMSimPhysicsList.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimEventAction.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimSteppingAction.hh"
#include "OMSimUIinterface.hh"

#include <G4RunManager.hh>
#include <G4MTRunManager.hh>
#include <G4VisExecutive.hh>
#include <G4UIExecutive.hh>

#include <boost/program_options.hpp>

namespace po = boost::program_options;

/**
 * @class OMSim
 * @brief Controls the main simulation process.
 *
 * The OMSim class is the top-level controller for the simulation. It sets up and
 * manages the individual components of the Geant4 simulation, such as the run manager,
 * the visualization manager, and the navigator.
 */
class OMSim
{
public:
    OMSim();
    ~OMSim();

    void initialiseSimulation(OMSimDetectorConstruction *pDetectorConstruction);
    void configureLogger();
    bool handleArguments(int pArgumentCount, char *pArgumentVector[]);
    void startVisualisation();

    G4Navigator *getNavigator() { return m_navigator.get(); };
    void extendOptions(po::options_description pNewOptions);
    po::options_description m_generalOptions;

private:
    void initialLoggerConfiguration();
    int determineNumberOfThreads();
    po::variables_map parseArguments(int pArgumentCount, char *pArgumentVector[]);
    void setUserArgumentsToArgTable(po::variables_map pVariablesMap);
    void setGeneralOptions();

    std::unique_ptr<G4RunManager> m_runManager;
    std::unique_ptr<G4VisExecutive> m_visManager;
    std::unique_ptr<G4VUserPhysicsList> m_physics;
    std::unique_ptr<G4TouchableHistory> m_history;
    std::unique_ptr<G4Navigator> m_navigator;

    std::chrono::high_resolution_clock::time_point m_startingTime;
};
