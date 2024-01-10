/**
 * @file OMSim.cc
 * @brief Implementation of the OMSim class.
 * 
 *  @warning
 * There are a few material related arguments that are depracated as for example the glass and gel arguments. This were used to easily change materials during the OM development phase. Check @link InputDataManager::getMaterial @endlink and modify the respective OM class if you want to use these args.
 * 
 * @ingroup common
 */

#include "OMSim.hh"
#include "OMSimLogger.hh"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
extern std::shared_ptr<spdlog::logger> global_logger;

/**
 * This constructor initializes the Geant4 run manager, visualization manager,
 * and navigator. It also sets up the command line arguments for the simulation.
 */
OMSim::OMSim() : mGeneralArgs("General options")
{
    mStartingTime = clock() / CLOCKS_PER_SEC;
    mRunManager = new G4RunManager;
    mVisManager = new G4VisExecutive;
    mNavigator = new G4Navigator();

    mGeneralArgs.add_options()("help", "produce help message")
    ("log_level", po::value<std::string>()->default_value("info"), "Granularity of logger, defaults to info [trace, debug, info, warn, error, critical, off]")
    ("output_file,o", po::value<std::string>()->default_value("output"), "filename for output")
    ("numevents,n", po::value<G4int>()->default_value(0), "number of events")
    ("visual,v", po::bool_switch()->default_value(false), "shows visualization of module after run")
    ("save_args", po::bool_switch()->default_value(true), "if true a json file with the args and seed is saved")
    ("seed", po::value<long>(), "seed for random engine. If none is given a seed from CPU time is used")
    ("environment", po::value<G4int>()->default_value(0), "medium in which the setup is emmersed [AIR = 0, ice = 1, spice = 2]")
    ("depth_pos", po::value<int>()->default_value(75), "index for choosing the depth for ice properties. [DustLayer=65, MeanICUProperties(approx)=75, CleanestIce=88]")
    ("detail_pmt", po::bool_switch(), "if given, simulate PMT with internal reflections and thin photocathode")
    ("QE_cut", po::bool_switch(), "if given, the photons will be deleted if they don't pass QE. DO NOT USE WITH detail_pmt.")
    ("pmt_response", po::bool_switch(), "if given, simulates PMT response using scan data (currently only for mDOM PMT)")
    ("place_harness",po::bool_switch(),"place OM harness (if implemented)")
	("detector_type", po::value<G4int>()->default_value(2), "module type [custom = 0, Single PMT = 1, mDOM = 2, pDDOM = 3, LOM16 = 4]")
    ("check_overlaps", po::bool_switch()->default_value(true), "check overlaps between volumes during construction")
    ("glass", po::value<G4int>()->default_value(0), "DEPRECATED. Index to select glass type [VITROVEX = 0, Chiba = 1, Kopp = 2, myVitroVex = 3, myChiba = 4, WOMQuartz = 5, fusedSilica = 6]")
    ("gel", po::value<G4int>()->default_value(1), "DEPRECATED. Index to select gel type [Wacker = 0, Chiba = 1, IceCube = 2, Wacker_company = 3]")
    ("reflective_surface", po::value<G4int>()->default_value(0), "DEPRECATED. Index to select reflective surface type [Refl_V95Gel = 0, Refl_V98Gel = 1, Refl_Aluminium = 2, Refl_Total98 = 3]")
    ("pmt_model", po::value<G4int>()->default_value(0), "DEPRECATED. R15458 (mDOM) = 0,  R7081 (DOM) = 1, 4inch (LOM) = 2, R5912_20_100 (D-Egg)= 3");
}

spdlog::level::level_enum getLogLevelFromString(const std::string &pLevelString)
{
    static const std::unordered_map<std::string, spdlog::level::level_enum> lLevelMap = {
        {"trace", spdlog::level::trace},
        {"debug", spdlog::level::debug},
        {"info", spdlog::level::info},
        {"warn", spdlog::level::warn},
        {"error", spdlog::level::err},
        {"critical", spdlog::level::critical},
        {"off", spdlog::level::off}};
    auto it = lLevelMap.find(pLevelString);
    if (it != lLevelMap.end())
    {
        return it->second;
    }
    // Default log level if string is not recognized
    return spdlog::level::info;
}

void OMSim::configureLogger()
{
    G4cout << ":::::::::::::::::::" << G4endl;
    global_logger = spdlog::stdout_color_mt("console");
    std::string lLogLevel = OMSimCommandArgsTable::getInstance().get<std::string>("log_level");
    global_logger->set_level(getLogLevelFromString(lLogLevel)); // Set the desired log level
    global_logger->set_pattern("%^[%Y-%m-%d %H:%M:%S.%e][%l][%s:%#]%$ %v");
    spdlog::set_default_logger(global_logger);  
    log_trace("Logger configured to level {}", lLogLevel);
}


/**
 * @brief UIEx session is started for visualisation.
 */
void OMSim::startVisualisation()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    char *argumv[] = {"all", NULL};
    G4UIExecutive *UIEx = new G4UIExecutive(1, argumv);
    lUIinterface.applyCommand("/control/execute ../aux/init_vis.mac");
    UIEx->SessionStart();
    delete UIEx;
}
/**
 * @brief Ensure that the output directory for the simulation results exists.
 *
 * If the output directory does not exist, this function will create it.
 *
 * @param pFilePath The path to the output directory.
 */
void OMSim::ensureOutputDirectoryExists(const std::string &pFilePath)
{
    std::filesystem::path full_path(pFilePath);
    std::filesystem::path dir = full_path.parent_path();
    if (!dir.empty() && !std::filesystem::exists(dir))
    {
        std::filesystem::create_directories(dir);
    }
}

/*
OMSimDetectorConstruction* OMSim::getDetectorConstruction()
{
    return mDetector;
}
*/

    /**
     * @brief Initialize the simulation.
     *
     * This function sets up the necessary Geant4 components.
     */
void OMSim::initialiseSimulation(OMSimDetectorConstruction* pDetectorConstruction)
{
    configureLogger();

    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
    ensureOutputDirectoryExists(lArgs.get<std::string>("output_file"));

    std::string lFileName = lArgs.get<std::string>("output_file") + "_args.json";
    if (lArgs.get<bool>("save_args"))
        lArgs.writeToJson(lFileName);

    CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine(lArgs.get<long>("seed"), 3));

    mRunManager->SetUserInitialization(pDetectorConstruction);

    mPhysics = new OMSimPhysicsList;
    mRunManager->SetUserInitialization(mPhysics);

    mVisManager->Initialize();

    mGenAction = new OMSimPrimaryGeneratorAction();
    mRunManager->SetUserAction(mGenAction);

    mRunAction = new OMSimRunAction();
    mRunManager->SetUserAction(mRunAction);

    mEventAction = new OMSimEventAction();
    mRunManager->SetUserAction(mEventAction);

    mTracking = new OMSimTrackingAction();
    mRunManager->SetUserAction(mTracking);

    mStepping = new OMSimSteppingAction();
    mRunManager->SetUserAction(mStepping);

    mRunManager->Initialize();

    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.setUI(G4UImanager::GetUIpointer());

    mNavigator->SetWorldVolume(pDetectorConstruction->mWorldPhysical);
    mNavigator->LocateGlobalPointAndSetup(G4ThreeVector(0., 0., 0.));

    mHistory = mNavigator->CreateTouchableHistory();

    lUIinterface.applyCommand("/control/execute ", lArgs.get<bool>("visual"));
}

OMSim::~OMSim()
{
    delete mNavigator;
    delete mVisManager;
    delete mRunManager;
    double lFinishtime = clock() / CLOCKS_PER_SEC;
    G4cout << "Computation time: " << lFinishtime - mStartingTime << " seconds." << G4endl;
}