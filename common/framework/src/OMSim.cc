/**
 * @file OMSim.cc
 * @brief Implementation of the OMSim class.
 * 
 * @warning
 * There are a few material related arguments that are depracated as for example the glass and gel arguments. This were used to easily change materials during the OM development phase. Check @link OMSimInputData::getMaterial @endlink and modify the respective OM class if you want to use these args.
 * 
 * @ingroup common
 */

#include "OMSim.hh"
#include "OMSimTools.hh"
#include "OMSimLogger.hh"
#include "OMSimActionInitialization.hh"
#include <boost/program_options.hpp>

namespace po = boost::program_options;
extern std::shared_ptr<spdlog::logger> globalLogger;

OMSim::OMSim() : 
mStartingTime(std::chrono::high_resolution_clock::now()), 
mGeneralOptions("General options"), 
mRunManager(nullptr),
mVisManager(nullptr),
mNavigator(nullptr)
{
    OMSimCommandArgsTable::init();
    setGeneralOptions();
    initialLoggerConfiguration();
}

void OMSim::setGeneralOptions()
{
    mGeneralOptions.add_options()("help", "produce help message")
    ("log_level", po::value<std::string>()->default_value("info"), "Granularity of logger, defaults to info [trace, debug, info, warn, error, critical, off]")
    ("output_file,o", po::value<std::string>()->default_value("output"), "filename for output")
    ("numevents,n", po::value<G4int>()->default_value(0), "number of events")
    ("visual,v", po::bool_switch()->default_value(false), "shows visualization of module after run")
    ("save_args", po::bool_switch()->default_value(true), "if true a json file with the args and seed is saved")
    ("seed", po::value<long>(), "seed for random engine. If none is given a seed from CPU time is used")
    ("environment", po::value<G4int>()->default_value(0), "medium in which the setup is emmersed [AIR = 0, ice = 1, spice = 2]")
    ("depth_pos", po::value<int>()->default_value(75), "index for choosing the depth for ice properties. [DustLayer=65, MeanICUProperties(approx)=75, CleanestIce=88]")
    ("detail_pmt", po::bool_switch(), "if given, simulate PMT with internal reflections and thin photocathode")
    ("QE_file", po::value<std::string>()->default_value("default"), "file path for custom QE file (file should contain two columns separated by tab, QE should not be in %!)")
    ("QE_cut", po::bool_switch(), "if given, the photons will be deleted if they don't pass QE. DO NOT USE WITH detail_pmt.")
    ("pmt_response", po::bool_switch(), "if given, simulates PMT response using scan data (currently only for mDOM PMT)")
    ("place_harness",po::bool_switch(),"place OM harness (if implemented)")
	("detector_type", po::value<G4int>()->default_value(2), "module type [custom = 0, Single PMT = 1, mDOM = 2, pDDOM = 3, LOM16 = 4]")
    ("check_overlaps", po::bool_switch()->default_value(false), "check overlaps between volumes during construction")
    ("glass", po::value<G4int>()->default_value(0), "DEPRECATED. Index to select glass type [VITROVEX = 0, Chiba = 1, Kopp = 2, myVitroVex = 3, myChiba = 4, WOMQuartz = 5, fusedSilica = 6]")
    ("gel", po::value<G4int>()->default_value(1), "DEPRECATED. Index to select gel type [Wacker = 0, Chiba = 1, IceCube = 2, Wacker_company = 3]")
    ("reflective_surface", po::value<G4int>()->default_value(0), "DEPRECATED. Index to select reflective surface type [Surf_V95Gel = 0, Surf_V98Gel = 1, Surf_Aluminium = 2, Surf_Total98 = 3]")
    ("pmt_model", po::value<G4int>()->default_value(0), "DEPRECATED. R15458 (mDOM) = 0,  R7081 (DOM) = 1, 4inch (LOM) = 2, R5912_20_100 (D-Egg)= 3")
    ("threads", po::value<int>()->default_value(1), "number of threads to use.");
}

void OMSim::initialLoggerConfiguration()
{
    globalLogger = spdlog::stdout_color_mt("console");
    globalLogger->set_level(spdlog::level::info); // Set the desired log level
    globalLogger->set_pattern("%^[%H:%M:%S.%e][t %t][%l][%s:%#]%$ %v");
    spdlog::set_default_logger(globalLogger); 
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
    std::string lLogLevel = OMSimCommandArgsTable::getInstance().get<std::string>("log_level");
    globalLogger->set_level(getLogLevelFromString(lLogLevel)); // Set the desired log level
    spdlog::set_default_logger(globalLogger);  
    log_trace("Logger configured to level {}", lLogLevel);
}

/**
 * @brief UIEx session is started for visualisation.
 */
void OMSim::startVisualisation()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    char lArg0[] = "all";
    char *lArgv[] = {lArg0, NULL};
    G4UIExecutive *UIEx = new G4UIExecutive(1, lArgv);
    lUIinterface.applyCommand("/control/execute ../common/data/vis/init_vis.mac");
    UIEx->SessionStart();
    delete UIEx;
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

/**
 * @brief Initialize the simulation constructing all Geant instances.
 */
void OMSim::initialiseSimulation(OMSimDetectorConstruction* pDetectorConstruction)
{
    OMSimHitManager::init();
    configureLogger();
    
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
    Tools::ensureDirectoryExists(lArgs.get<std::string>("output_file"));

    std::string lFileName = lArgs.get<std::string>("output_file") + "_args.json";
    if (lArgs.get<bool>("save_args"))
        lArgs.writeToJson(lFileName);

    //CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine(lArgs.get<long>("seed"), 3));
    //CLHEP::HepRandom::setTheEngine(new CLHEP::MixMaxRng(lArgs.get<long>("seed")));
    //G4Random::setTheEngine(new CLHEP::MixMaxRng(lArgs.get<long>("seed")));
    long seed = lArgs.get<long>("seed");
    G4Random::setTheEngine(new CLHEP::MixMaxRng(seed));
    G4Random::setTheSeed(seed);

    mRunManager = std::make_unique<G4MTRunManager>();
    //mRunManager->SetVerboseLevel(2);
    mVisManager = std::make_unique<G4VisExecutive>();
    mNavigator = std::make_unique<G4Navigator>();

    int nThreads = determineNumberOfThreads();
    mRunManager->SetStoreTrajectory(true);
    mRunManager->SetNumberOfThreads(nThreads);

    mRunManager->SetUserInitialization(pDetectorConstruction);

    mPhysics = std::make_unique<OMSimPhysicsList>();
    mRunManager->SetUserInitialization(mPhysics.get());
    mPhysics.release();

    mVisManager->Initialize();

    OMSimActionInitialization* actionInitialization = new OMSimActionInitialization();
    mRunManager->SetUserInitialization(actionInitialization);
    mRunManager->Initialize();
    

    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.setUI(G4UImanager::GetUIpointer());

    mNavigator.get()->SetWorldVolume(pDetectorConstruction->mWorldPhysical);
    mNavigator.get()->LocateGlobalPointAndSetup(G4ThreeVector(0., 0., 0.));

    mHistory = std::unique_ptr<G4TouchableHistory>(mNavigator->CreateTouchableHistory());

}

/**
 * @brief Adds options from the different simulation modules to the option description list (what is printed in --help).
 */
void OMSim::extendOptions(po::options_description pNewOptions)
{
	mGeneralOptions.add(pNewOptions);
}


/**
 * @brief Parses user terminal arguments to a variables map 
 */
po::variables_map OMSim::parseArguments(int pArgumentCount, char *pArgumentVector[])
{   
	po::variables_map lVariablesMap;
	try {
		po::store(po::parse_command_line(pArgumentCount, pArgumentVector, mGeneralOptions), lVariablesMap);
	} catch (std::invalid_argument& e) {
		std::cerr << "Invalid argument: " << e.what() << std::endl;
	} catch (std::exception& e) {
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	} catch (...) {
		std::cerr << "An unknown exception occurred." << std::endl;
	}
	po::notify(lVariablesMap);

	return lVariablesMap;
}

/**
 * @brief Sets variables from a variables map to the instance of OMSimCommandArgsTable
 */
void OMSim::setUserArgumentsToArgTable(po::variables_map pVariablesMap)
{
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	for (const auto &option : pVariablesMap)
	{
		lArgs.setParameter(option.first, option.second.value());
	}
	// Now that all parameters are set, "finalize" the OMSimCommandArgsTable instance so that the parameters cannot be modified anymore
	lArgs.finalize();
}

/**
 * @brief Parses the user arguments into variables that can be accessed in the simulation via OMSimCommandArgsTable. 
 * @return true if simulation should continue, if --help is called it will return false and stop the program
 */
bool OMSim::handleArguments(int pArgumentCount, char *pArgumentVector[])
{
	
	po::variables_map lVariablesMap = parseArguments(pArgumentCount, pArgumentVector);

	//check if user needs help
	if (lVariablesMap.count("help"))
	{
		std::cout << mGeneralOptions << "\n";
		return false;
	}

	//If no help needed continue and set arguments to arg table
	setUserArgumentsToArgTable(lVariablesMap);
	return true;
}




OMSim::~OMSim()
{
    log_trace("Resetting mHistory");
    mHistory.reset();
    
    log_trace("Resetting mNavigator");
    mNavigator.reset();
    
    log_trace("Resetting mPhysics");
    mPhysics.reset();
    
    log_trace("Resetting mVisManager");
    mVisManager.reset();

    log_trace("Resetting mRunManager");
    mRunManager.reset();

    log_trace("Deleting OMSimHitManager");
    OMSimHitManager::shutdown();

    log_trace("Deleting OMSimCommandArgsTable");
    OMSimCommandArgsTable::shutdown();

    log_trace("OMSim destructor finished");
    std::chrono::high_resolution_clock::time_point lFinishtime = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> lDiff = lFinishtime - mStartingTime;
    log_info("Computation time: {} {}", lDiff.count(), " seconds.");
}