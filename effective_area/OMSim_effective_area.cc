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

std::shared_ptr<spdlog::logger> global_logger;

namespace po = boost::program_options;

void effectiveAreaSimulation()
{
	OMSimEffectiveAreaAnalyisis lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
	AngularScan *lScanner = new AngularScan(lArgs.get<G4double>("radius"), lArgs.get<G4double>("distance"), lArgs.get<G4double>("wavelength"));

	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";

	bool lWriteHeader = !lArgs.get<bool>("no_header");
	if (lWriteHeader) lAnalysisManager.writeHeader();

	// If angle file is provided, run over all angle pairs in file
	if (lArgs.keyExists("angles_file"))
	{
		std::vector<G4PV2DDataVector> data = InputDataManager::loadtxt(lArgs.get<std::string>("angles_file"), false);
		std::vector<G4double> lThetas = data.at(0);
		std::vector<G4double> lPhis = data.at(1);

		for (std::vector<int>::size_type i = 0; i != lThetas.size(); i++)
		{
			lScanner->runSingleAngularScan(lPhis.at(i), lThetas.at(i));
			lAnalysisManager.writeScan(lPhis.at(i), lThetas.at(i));
			lHitManager.reset();
		}
	}
	// If file with angle pairs was not provided, use the angle pairs provided through command-line arguments
	else
	{
		lScanner->runSingleAngularScan(lArgs.get<G4double>("phi"), lArgs.get<G4double>("theta"));
		lAnalysisManager.writeScan(lArgs.get<G4double>("phi"), lArgs.get<G4double>("theta"));
		lHitManager.reset();
	}
}

int main(int argc, char *argv[])
{
	try
	{
		OMSim lSimulation;
		// Do not use G4String as type here...
		po::options_description lSpecific("Effective area specific arguments");

		lSpecific.add_options()
		("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
		("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
		("distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")
		("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")
		("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")
		("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")
		("angles_file,i", po::value<std::string>(), "The input angle pairs file to be scanned. The file should contain two columns, the first column with the theta (zenith) and the second with phi (azimuth) in degrees.")
		("no_header", po::bool_switch(), "if given, the header of the output file will not be written");


		po::options_description lAllargs("Allowed input arguments");
		lAllargs.add(lSimulation.mGeneralArgs).add(lSpecific);


		po::variables_map lVariablesMap;
		try {
			po::store(po::parse_command_line(argc, argv, lAllargs), lVariablesMap);
		} catch (std::invalid_argument& e) {
			std::cerr << "Invalid argument: " << e.what() << std::endl;
		} catch (std::exception& e) {
			std::cerr << "An exception occurred: " << e.what() << std::endl;
		} catch (...) {
			std::cerr << "An unknown exception occurred." << std::endl;
		}

		po::notify(lVariablesMap);

		if (lVariablesMap.count("help"))
		{
			std::cout << lAllargs << "\n";
			return 0;
		}

		OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

		// Now store the parsed parameters in the OMSimCommandArgsTable instance
		for (const auto &option : lVariablesMap)
		{
			lArgs.setParameter(option.first, option.second.value());
		}

		// Now that all parameters are set, "finalize" the OMSimCommandArgsTable instance so that the parameters cannot be modified anymore
		lArgs.finalize();

		OMSimEffectiveAreaDetector* lDetectorConstruction = new OMSimEffectiveAreaDetector();
		lSimulation.initialiseSimulation(lDetectorConstruction);

		effectiveAreaSimulation();
		if(lArgs.get<bool>("visual")) lSimulation.startVisualisation();
	
	}
	catch (std::exception &e)
	{
		std::cerr << "error: " << e.what() << "\n";
		return 1;
	}
	catch (...)
	{
		std::cerr << "Exception of unknown type!\n";
	}

	return 0;
}
