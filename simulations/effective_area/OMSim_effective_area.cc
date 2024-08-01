/**
 * @file 
 * @ingroup EffectiveArea
 * @brief Main for the calculation of effective areas.
 * @details The effective area of a module is calculated simulating a plane wave from a certain direction.
 */
#include "OMSim.hh"
#include "OMSimAngularScan.hh"
#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimTools.hh"

std::shared_ptr<spdlog::logger> globalLogger;

namespace po = boost::program_options;

void runEffectiveAreaSimulation()
{
	OMSimEffectiveAreaAnalyisis lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
	AngularScan *lScanner = new AngularScan(lArgs.get<G4double>("radius"), lArgs.get<G4double>("distance"), lArgs.get<G4double>("wavelength"));

	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";

	bool lWriteHeader = !lArgs.get<bool>("no_header");
	if (lWriteHeader) lAnalysisManager.writeHeader("Phi", "Theta", "Wavelength");

	// If angle file is provided, run over all angle pairs in file
	if (lArgs.keyExists("angles_file"))
	{
		std::vector<G4PV2DDataVector> data = Tools::loadtxt(lArgs.get<std::string>("angles_file"), true);
		std::vector<G4double> lThetas = data.at(0);
		std::vector<G4double> lPhis = data.at(1);

		for (std::vector<int>::size_type i = 0; i != lThetas.size(); i++)
		{
			lScanner->runSingleAngularScan(lPhis.at(i), lThetas.at(i));
			lAnalysisManager.writeScan(lPhis.at(i), lThetas.at(i),  lArgs.get<G4double>("wavelength"));
			lHitManager.reset();
		}
	}
	// If file with angle pairs was not provided, use the angle pairs provided through command-line arguments
	else
	{
		lScanner->runSingleAngularScan(lArgs.get<G4double>("phi"), lArgs.get<G4double>("theta"));
		lAnalysisManager.writeScan(lArgs.get<G4double>("phi"), lArgs.get<G4double>("theta"),  lArgs.get<G4double>("wavelength"));
		lHitManager.reset();
	}
}

/**
 * @brief Add options for the user input arguments for the effective area module
 */
void addModuleOptions(OMSim* pSimulation)
{
	po::options_description lSpecific("Effective area specific arguments");

	// Do not use G4String as type here...
	lSpecific.add_options()
	("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")
	("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")
	("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")
	("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")
	("angles_file,i", po::value<std::string>(), "The input angle pairs file to be scanned. The file should contain two columns, the first column with the theta (zenith) and the second with phi (azimuth) in degrees.")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

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

	runEffectiveAreaSimulation();

	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		lSimulation.startVisualisation();
	return 0;
}
