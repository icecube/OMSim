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

std::shared_ptr<spdlog::logger> g_logger;
namespace po = boost::program_options;

void runEffectiveAreaSimulation()
{
	OMSimEffectiveAreaAnalyisis analysisManager;
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();
	AngularScan *scanner = new AngularScan(args.get<G4double>("radius"), args.get<G4double>("distance"), args.get<G4double>("wavelength"));

	analysisManager.m_outputFileName = args.get<std::string>("output_file") + ".dat";

	bool writeHeader = !args.get<bool>("no_header");
	if (writeHeader) analysisManager.writeHeader("Phi", "Theta", "Wavelength");

	std::vector<double> wavelengths = Tools::arange(480, 680, 20);
	std::vector<double> phis = Tools::arange(0, 180, 5);

	for (const auto &wavelength : wavelengths)
	{	scanner->setWaveLength(wavelength);
		for (const auto &phi : phis)
		{	
			scanner->runSingleAngularScan(phi, args.get<G4double>("theta"));
			analysisManager.writeScan(phi, args.get<G4double>("theta"), wavelength);
			hitManager.reset();
		}
	}
}

/**
 * @brief Add options for the user input arguments for the effective area module
 */
void addModuleOptions(OMSim* p_simulation)
{
	po::options_description effectiveAreaOptions("Effective area specific arguments");

	// Do not use G4String as type here...
	effectiveAreaOptions.add_options()
	("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")
	("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")
	("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")
	("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")
	("angles_file,i", po::value<std::string>(), "The input angle pairs file to be scanned. The file should contain two columns, the first column with the theta (zenith) and the second with phi (azimuth) in degrees.")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	p_simulation->extendOptions(effectiveAreaOptions);
}

int main(int p_argCount, char *p_argumentVector[])
{

	OMSim simulation;
	addModuleOptions(&simulation);
	bool successful = simulation.handleArguments(p_argCount, p_argumentVector);
	if (!successful)
		return 0;

	std::unique_ptr<OMSimEffectiveAreaDetector> detectorConstruction = std::make_unique<OMSimEffectiveAreaDetector>();
	simulation.initialiseSimulation(detectorConstruction.get());
	detectorConstruction.release();

	runEffectiveAreaSimulation();

	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		simulation.startVisualisation();

	return 0;
}
