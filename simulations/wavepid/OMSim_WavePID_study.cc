/**
 * @file OMSim_WavePID_study.cc
 * @brief Main executable for WavePID photon origin tracking study.
 *
 * This simulation tracks photon origins (Cerenkov from Muon, Cerenkov from Electron,
 * Scintillation, etc.) for IceCube optical modules.
 */
#include "OMSim.hh"
#include "OMSimWavePIDDetector.hh"
#include "OMSimHitManager.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimUIinterface.hh"
#include "OMSimLogger.hh"

#include <boost/program_options.hpp>
#include <memory>

std::shared_ptr<spdlog::logger> g_logger;
namespace po = boost::program_options;

/**
 * @brief Add WavePID-specific command-line options.
 */
void addModuleOptions(OMSim* p_simulation)
{
    po::options_description wavepidOptions("WavePID study specific arguments");

    wavepidOptions.add_options()
        ("impact_parameter,d", po::value<G4double>()->default_value(5.0),
            "Impact parameter: perpendicular distance from muon track to DOM center in meters")
        ("primary_energy,e", po::value<G4double>()->default_value(10.0),
            "Primary particle energy in GeV")
        ("primary_particle,p", po::value<std::string>()->default_value("mu-"),
            "Primary particle type (mu-, mu+, e-, e+)")
        ("DOM_zenith,z", po::value<G4double>()->default_value(0.0),
            "Zenith angle of DOM orientation in degrees")
        ("DOM_azimuth,a", po::value<G4double>()->default_value(0.0),
            "Azimuth angle of DOM orientation in degrees")
        ("macro,m", po::value<std::string>()->default_value(""),
            "Path to macro file to execute (overrides -d, -e, -p if GPS commands used)");

    p_simulation->extendOptions(wavepidOptions);
}

/**
 * @brief Run the WavePID simulation.
 */
void runWavePIDSimulation()
{
    OMSimCommandArgsTable& args = OMSimCommandArgsTable::getInstance();

    log_info("WavePID simulation started");
    log_info("DOM orientation: zenith={} deg, azimuth={} deg",
             args.get<G4double>("DOM_zenith"),
             args.get<G4double>("DOM_azimuth"));
    // Execute macro file if provided
    std::string macroFile = args.get<std::string>("macro");
    if (!macroFile.empty()) {
        log_info("Executing macro file: {}", macroFile);
        OMSimUIinterface& ui = OMSimUIinterface::getInstance();
        ui.applyCommand("/control/execute " + macroFile);
    }

    // Run beamOn if numevents > 0
    G4int numEvents = args.get<G4int>("numevents");
    if (numEvents > 0) {
        log_info("Running {} events", numEvents);
        OMSimUIinterface& ui = OMSimUIinterface::getInstance();
        ui.applyCommand("/run/beamOn " + std::to_string(numEvents));
    }
}

int main(int argc, char* argv[])
{
    OMSim simulation;
    addModuleOptions(&simulation);

    bool successful = simulation.handleArguments(argc, argv);
    if (!successful)
        return 0;

    OMSimCommandArgsTable& args = OMSimCommandArgsTable::getInstance();

    // Construct ROOT output filename
    std::string outputFile = args.get<std::string>("output_file") + "_hits.root";

    // Initialize detector construction
    std::unique_ptr<OMSimWavePIDDetector> detector = std::make_unique<OMSimWavePIDDetector>();

    // Initialize simulation with our detector
    // Note: OMSimHitManager::init() is called in initialiseSimulation()
    simulation.initialiseSimulation(detector.get());
    detector.release();

    // Set ROOT output file for hit data
    OMSimHitManager::setROOTOutputFile(outputFile);
    log_info("ROOT output file: {}", outputFile);

    // Run the simulation
    runWavePIDSimulation();

    // Start visualization if requested
    if (args.get<bool>("visual")) {
        simulation.startVisualisation();
    }

    log_info("WavePID simulation completed");
    return 0;
}
