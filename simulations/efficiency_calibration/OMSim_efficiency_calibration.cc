
#include "OMSim.hh"
#include "OMSimBeam.hh"
#include "OMSimEffiCaliAnalyisis.hh"
#include "OMSimEffiCaliDetector.hh"
#include "OMSimTools.hh"
std::shared_ptr<spdlog::logger> g_logger;

namespace po = boost::program_options;

void runQEbeamSimulation()
{
	OMSimEffiCaliAnalyisis analysisManager;
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();

	Beam *scanner = new Beam(args.get<G4double>("radius"), args.get<G4double>("distance"));
	analysisManager.m_outputFileName = args.get<std::string>("output_file") + ".dat";

	std::vector<double> wavelengths = Tools::arange(250, 800, 5);
	//std::vector<double> wavelengths = Tools::arange(250, 305, 5); //UV range for higher statistics


	for (const auto &wavelength : wavelengths)
	{
		scanner->setWavelength(wavelength);
		scanner->runErlangenQEBeam();
		analysisManager.writeHits(wavelength);
		hitManager.reset();
	}
}

void runXYZfrontalScan()
{
	OMSimEffiCaliAnalyisis analysisManager;
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();

	Beam *scanner = new Beam(0.3, args.get<G4double>("distance"));
	scanner->configureZCorrection_PicoQuant();
	analysisManager.m_outputFileName = args.get<std::string>("output_file") + ".dat";
	scanner->setWavelength(459);

	std::vector<double> grid = Tools::arange(-41, 42, 0.75); //mDOM
    //std::vector<double> grid = Tools::arange(-52.8, 52.8, 1.25); //LOM...1.6 in scan but causes two 0 values

	double rLim = 42; //mDOM
	//double rLim = 53; //LOM

	for (const auto &x : grid)
	{
		for (const auto &y : grid)
		{
			if (std::sqrt(x * x + y * y) < rLim)
			{
				scanner->runBeamPicoQuantSetup(x, y);
				analysisManager.writeHitPositionHistogram(x, y);
				hitManager.reset();
			}
		}
	}

	grid = Tools::arange(-7, 7, 0.5); // so more data at centre

	for (const auto &x : grid)
	{
		for (const auto &y : grid)
		{
			scanner->runBeamPicoQuantSetup(x, y);
			analysisManager.writeHitPositionHistogram(x, y);
			hitManager.reset();
		}
	}
	//scanner->runBeamPicoQuantSetup(20,0); //for visualisation checking
}


void runfrontalXYScannNKT()
{
	OMSimEffiCaliAnalyisis analysisManager;
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();

	Beam *scanner = new Beam(0.3, args.get<G4double>("distance"));
	scanner->configureZCorrection_PicoQuant();
	analysisManager.m_outputFileName = args.get<std::string>("output_file") + ".dat";
	double wavelength = 459;
	scanner->setWavelength(wavelength);

	std::vector<double> grid = Tools::arange(-41, 42, 1); //mDOM
    //std::vector<double> grid = Tools::arange(-52.8, 52.8, 1.25); //LOM...1.6 in scan but causes two 0 values

	double rLim = 42; //mDOM
	//double rLim = 53; //LOM

	for (const auto &x : grid)
	{
		for (const auto &y : grid)
		{
			if (std::sqrt(x * x + y * y) < rLim)
			{
				scanner->runBeamNKTSetup(x, y);
				//scanner->runBeamPicoQuantSetup(x, y);
				analysisManager.writePositionPulseStatistics(x, y, wavelength);
				hitManager.reset();
			}
		}
	}
	
}

void runfrontalProfileScannNKT()
{
	OMSimEffiCaliAnalyisis analysisManager;
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();

	Beam *scanner = new Beam(0.3, args.get<G4double>("distance"));
	scanner->configureZCorrection_PicoQuant();
	analysisManager.m_outputFileName = args.get<std::string>("output_file") + ".dat";
	double wavelength = 459;
	scanner->setWavelength(wavelength);

	std::vector<double> profile = Tools::arange(0, 42, 0.05); //mDOM
    //std::vector<double> profile = Tools::arange(0, 52.8, 0.1); //LOM.

	double rLim = 42; //mDOM
	//double rLim = 53; //LOM

	for (const auto &x : profile)
	{
		scanner->runBeamNKTSetup(x, 0);
		//scanner->runBeamPicoQuantSetup(x, 0);
		analysisManager.writePositionStatistics(x, wavelength);
		hitManager.reset();
	}
}

/**
 * @brief Add options for the user input arguments for the effective area module
 */
void addModuleOptions(OMSim *p_simulation)
{
	po::options_description extraOptions("Effective area specific arguments");

	// Do not use G4String as type here...
	extraOptions.add_options()("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(5.0), "plane wave radius in mm")(
		"distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")
		("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")
		("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")
		("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")
		("simulation_step", po::value<G4int>()->default_value(0), "simulation step to be performed (0, 1, 2)")
		("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	p_simulation->extendOptions(extraOptions);
}

int main(int p_argumentCount, char *p_argumentVector[])
{

	OMSim simulation;
	addModuleOptions(&simulation);
	bool successful = simulation.handleArguments(p_argumentCount, p_argumentVector);
	if (!successful)
		return 0;

	std::unique_ptr<OMSimEffiCaliDetector> detectorConstruction = std::make_unique<OMSimEffiCaliDetector>();
	simulation.initialiseSimulation(detectorConstruction.get());
	detectorConstruction.release();

	switch (OMSimCommandArgsTable::getInstance().get<G4int>("simulation_step"))
	{
	case 1:
	{
		runQEbeamSimulation();
		break;
	}
	case 2:
	{
		runQEbeamSimulation();
		break;
	}
	case 3:
	{
		runXYZfrontalScan();
		break;
	}
	case 4:
	{
		runfrontalProfileScannNKT();
		break;
	}
	case 5:
	{
		runfrontalXYScannNKT();
		break;
	}
	}
	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		simulation.startVisualisation();
	return 0;
}
