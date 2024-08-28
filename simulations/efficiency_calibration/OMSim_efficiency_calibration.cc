
#include "OMSim.hh"
#include "OMSimBeam.hh"
#include "OMSimEffiCaliAnalyisis.hh"
#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimTools.hh"
std::shared_ptr<spdlog::logger> g_logger;

namespace po = boost::program_options;

void runQEbeamSimulation()
{
	OMSimEffiCaliAnalyisis lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	Beam *lScanner = new Beam(lArgs.get<G4double>("radius"), lArgs.get<G4double>("distance"));
	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";

	std::vector<double> lWavelengths = Tools::arange(250, 800, 5);

	for (const auto &wavelength : lWavelengths)
	{
		lScanner->setWavelength(wavelength);
		lScanner->runErlangenQEBeam();
		lAnalysisManager.writeHits(wavelength);
		lHitManager.reset();
	}
}

void runXYZfrontalScan()
{
	OMSimEffiCaliAnalyisis lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	Beam *lScanner = new Beam(0.3, lArgs.get<G4double>("distance"));
	lScanner->configureZCorrection_PicoQuant();
	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";
	lScanner->setWavelength(459);

	std::vector<double> lX = Tools::arange(-41, 42, 1);

	double lRlim = 42;

	for (const auto &x : lX)
	{
		for (const auto &y : lX)
		{
			if (std::sqrt(x * x + y * y) < lRlim)
			{
				lScanner->runBeamPicoQuantSetup(x, y);
				lAnalysisManager.writeHitPositionHistogram(x, y);
				lHitManager.reset();
			}
		}
	}

	// lX = Tools::arange(-7, 7, 0.3);

	// for (const auto &x : lX)
	// {
	// 	for (const auto &y : lX)
	// 	{
	// 		lScanner->runBeamPicoQuantSetup(x, y);
	// 		lAnalysisManager.writeHitPositionHistogram(x, y);
	// 		lHitManager.reset();
	// 	}
	// }
}

/**
 * @brief Add options for the user input arguments for the effective area module
 */
void addModuleOptions(OMSim *pSimulation)
{
	po::options_description lSpecific("Effective area specific arguments");

	// Do not use G4String as type here...
	lSpecific.add_options()("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")("radius,r", po::value<G4double>()->default_value(5.0), "plane wave radius in mm")("distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")("simulation_step", po::value<G4int>()->default_value(0), "simulation step to be performed (0, 1, 2)")("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	pSimulation->extendOptions(lSpecific);
}

int main(int pArgumentCount, char *pArgumentVector[])
{

	OMSim lSimulation;
	addModuleOptions(&lSimulation);
	bool lContinue = lSimulation.handleArguments(pArgumentCount, pArgumentVector);
	if (!lContinue)
		return 0;

	std::unique_ptr<OMSimEffectiveAreaDetector> lDetectorConstruction = std::make_unique<OMSimEffectiveAreaDetector>();
	lSimulation.initialiseSimulation(lDetectorConstruction.get());
	lDetectorConstruction.release();

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
	}

	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		lSimulation.startVisualisation();
	return 0;
}
