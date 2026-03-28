/**
 * @file OMSim.cc
 * @brief Implementation of the OMSim class for WavePID simulation.
 */

#include "OMSim.hh"
#include "OMSimTools.hh"
#include "OMSimLogger.hh"
#include "OMSimActionInitialization.hh"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
extern std::shared_ptr<spdlog::logger> g_logger;

OMSim::OMSim() :
m_startingTime(std::chrono::high_resolution_clock::now()),
m_generalOptions("General options"),
m_runManager(nullptr),
m_visManager(nullptr),
m_navigator(nullptr)
{
    OMSimCommandArgsTable::init();
    setGeneralOptions();
    initialLoggerConfiguration();
}

void OMSim::setGeneralOptions()
{
    m_generalOptions.add_options()("help", "produce help message")
    ("log_level", po::value<std::string>()->default_value("info"), "Granularity of logger, defaults to info [trace, debug, info, warn, error, critical, off]")
    ("output_file,o", po::value<std::string>()->default_value("output"), "filename for output")
    ("numevents,n", po::value<G4int>()->default_value(0), "number of events")
    ("visual,v", po::bool_switch()->default_value(false), "shows visualization of module after run")
    ("save_args", po::bool_switch()->default_value(true), "if true a json file with the args and seed is saved")
    ("seed", po::value<long>(), "seed for random engine. If none is given a seed from CPU time is used")
    ("environment", po::value<G4int>()->default_value(0), "medium in which the setup is emmersed [AIR = 0, ice = 1, spice = 2]")
    ("depth_pos", po::value<int>()->default_value(75), "index for choosing the depth for ice properties. [DustLayer=65, MeanICUProperties(approx)=75, CleanestIce=88]")
    ("simple_PMT", po::bool_switch(), "if given, simulate simple PMT")
    ("QE_file", po::value<std::string>()->default_value("default"), "file path for custom QE file (file should contain two columns separated by tab, QE should not be in %!)")
    ("efficiency_cut", po::bool_switch(), "if given, the photons will be deleted if they don't pass QE")
    ("pmt_response", po::bool_switch(), "if given, simulates PMT response using scan data (currently only for mDOM PMT)")
    ("place_harness",po::bool_switch(),"place OM harness (if implemented)")
    ("detector_type", po::value<G4int>()->default_value(2), "module type [custom = 0, Single PMT = 1, mDOM = 2, DOM = 3, LOM16 = 4, LOM18 = 5, DEGG = 6, pDOM (HQE deepcore) = 7]")
    ("check_overlaps", po::bool_switch()->default_value(false), "check overlaps between volumes during construction")
    ("glass", po::value<G4int>()->default_value(0), "DEPRECATED. Index to select glass type [VITROVEX = 0, Chiba = 1, Kopp = 2, myVitroVex = 3, myChiba = 4, WOMQuartz = 5, fusedSilica = 6]")
    ("gel", po::value<G4int>()->default_value(1), "DEPRECATED. Index to select gel type [Wacker = 0, Chiba = 1, IceCube = 2, Wacker_company = 3]")
    ("reflective_surface", po::value<G4int>()->default_value(0), "DEPRECATED. Index to select reflective surface type [Surf_V95Gel = 0, Surf_V98Gel = 1, Surf_Aluminium = 2, Surf_Total98 = 3]")
    ("pmt_model", po::value<G4int>()->default_value(0), "DEPRECATED. R15458 (mDOM) = 0,  R7081 (DOM) = 1, 4inch (LOM) = 2, R5912_20_100 (D-Egg)= 3, R7081_HQE (pDOM) = 4")
    ("threads", po::value<int>()->default_value(1), "number of threads to use (only active with --multithreading).")
    ("multithreading", po::bool_switch()->default_value(false), "enable multithreaded mode (G4MTRunManager); disabled by default because the Geant4 Qt GUI crashes in MT mode due to a SetupShadowProcess failure in G4WorkerRunManagerKernel");
}

void OMSim::initialLoggerConfiguration()
{
    g_logger = spdlog::stdout_color_mt("console");
    g_logger->set_level(spdlog::level::info);
    g_logger->set_pattern("%^[%H:%M:%S.%e][t %t][%l][%s:%#]%$ %v");
    spdlog::set_default_logger(g_logger);
}

spdlog::level::level_enum getLogLevelFromString(const std::string &p_levelString)
{
    static const std::unordered_map<std::string, spdlog::level::level_enum> levelMap = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}};
    auto it = levelMap.find(p_levelString);
    if (it != levelMap.end())
    {
        return it->second;
    }
    return spdlog::level::info;
}

void OMSim::configureLogger()
{
    std::string logLevel = OMSimCommandArgsTable::getInstance().get<std::string>("log_level");
    g_logger->set_level(getLogLevelFromString(logLevel));
    spdlog::set_default_logger(g_logger);
    log_trace("Logger configured to level {}", logLevel);
}

void OMSim::startVisualisation()
{
    OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
    char arg0[] = "all";
    char *argv[] = {arg0, NULL};
    G4UIExecutive *uiEx = new G4UIExecutive(1, argv);
    // Use WavePID-specific visualization macro that filters optical photons
    uiInterface.applyCommand("/control/execute ../simulations/wavepid/vis_nophotons.mac");
    uiEx->SessionStart();
    delete uiEx;
}

int OMSim::determineNumberOfThreads()
{
    int requestedThreads = OMSimCommandArgsTable::getInstance().get<int>("threads");
    int availableThreads = G4Threading::G4GetNumberOfCores();

    if (requestedThreads <= 0) {
        log_info("Auto-detected {} available cores", availableThreads);
        return availableThreads;
    } else {
        int threadsToUse = std::min(requestedThreads, availableThreads);
        log_info("Using {} out of {} available cores", threadsToUse, availableThreads);
        return requestedThreads;
    }
}

void OMSim::initialiseSimulation(OMSimDetectorConstruction* p_detectorConstruction)
{
    OMSimHitManager::init();

    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
    Tools::ensureDirectoryExists(args.get<std::string>("output_file"));

    std::string fileName = args.get<std::string>("output_file") + "_args.json";
    if (args.get<bool>("save_args"))
        args.writeToJson(fileName);

    long seed = args.get<long>("seed");
    G4Random::setTheEngine(new CLHEP::MixMaxRng(seed));
    G4Random::setTheSeed(seed);

    // Default: single-threaded G4RunManager. The Geant4 Qt GUI crashes in
    // G4MTRunManager mode because G4WorkerRunManagerKernel::SetupShadowProcess()
    // fails for particles (e.g. alpha) that have no process manager in worker
    // threads during interactive vis sessions. Pass --multithreading to opt in
    // to G4MTRunManager for large batch runs where the GUI is not used.
    bool useMultithreading = args.get<bool>("multithreading");
    if (useMultithreading) {
        auto mtManager = std::make_unique<G4MTRunManager>();
        int nThreads = determineNumberOfThreads();
        mtManager->SetNumberOfThreads(nThreads);
        m_runManager = std::move(mtManager);
    } else {
        m_runManager = std::make_unique<G4RunManager>();
    }
    m_visManager = std::make_unique<G4VisExecutive>();
    m_navigator = std::make_unique<G4Navigator>();

    m_runManager->SetUserInitialization(p_detectorConstruction);

    m_physics = std::make_unique<OMSimPhysicsList>();
    m_runManager->SetUserInitialization(m_physics.get());
    m_physics.release();

    m_visManager->Initialize();

    OMSimActionInitialization* actionInitialization = new OMSimActionInitialization();
    m_runManager->SetUserInitialization(actionInitialization);
    m_runManager->Initialize();

    OMSimUIinterface::init();
    OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
    uiInterface.setUI(G4UImanager::GetUIpointer());

    m_navigator.get()->SetWorldVolume(p_detectorConstruction->m_worldPhysical);
    m_navigator.get()->LocateGlobalPointAndSetup(G4ThreeVector(0., 0., 0.));

    m_history = std::unique_ptr<G4TouchableHistory>(m_navigator->CreateTouchableHistory());
}

void OMSim::extendOptions(po::options_description p_newOptions)
{
    m_generalOptions.add(p_newOptions);
}

po::variables_map OMSim::parseArguments(int p_argumentCount, char *p_argumentVector[])
{
    po::variables_map variableMap;
    po::store(po::parse_command_line(p_argumentCount, p_argumentVector, m_generalOptions), variableMap);
    po::notify(variableMap);
    return variableMap;
}

void OMSim::setUserArgumentsToArgTable(po::variables_map p_variablesMap)
{
    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
    for (const auto &option : p_variablesMap)
    {
        args.setParameter(option.first, option.second.value());
    }
    args.finalize();
}

bool OMSim::handleArguments(int p_argumentCount, char *p_argumentVector[])
{
    po::variables_map variableMap = parseArguments(p_argumentCount, p_argumentVector);

    if (variableMap.count("help"))
    {
        std::cout << m_generalOptions << "\n";
        return false;
    }

    setUserArgumentsToArgTable(variableMap);
    configureLogger();
    return true;
}

OMSim::~OMSim()
{
    log_trace("Resetting m_history");
    m_history.reset();

    log_trace("Resetting m_navigator");
    m_navigator.reset();

    log_trace("Resetting m_physics");
    m_physics.reset();

    log_trace("Resetting m_visManager");
    m_visManager.reset();

    log_trace("Resetting m_runManager");
    m_runManager.reset();

    log_trace("Deleting OMSimHitManager");
    OMSimHitManager::shutdown();

    log_trace("Deleting OMSimCommandArgsTable");
    OMSimCommandArgsTable::shutdown();

    log_trace("Deleting OMSimUIinterface");
    OMSimUIinterface::shutdown();

    log_trace("OMSim destructor finished");
    std::chrono::high_resolution_clock::time_point finishTime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> deltaT = finishTime - m_startingTime;
    log_info("Computation time: {} {}", deltaT.count(), " seconds.");
}
