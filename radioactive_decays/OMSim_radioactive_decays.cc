/**
 * @file 
 * @ingroup radioactive
 * @brief Main of the radioactive decay study.
 * @details @see radioactive
 * @warning
 *    This study has been tested only for Vitrovex glass (mDOM/LOM16) and the 80mm mDOM PMTs. Okamoto glass (D-Egg/LOM18) is currently under investigation.
 */
#include "OMSim.hh"
#include "OMSimDecaysGPS.hh"
#include "OMSimHitManager.hh"
#include "OMSimDecaysAnalysis.hh"
#include "OMSimRadDecaysDetector.hh"

namespace po = boost::program_options;

void decaySimulation(OMSimRadDecaysDetector *pDetector)
{
	OMSimDecaysAnalysis &lAnalysisManager = OMSimDecaysAnalysis::getInstance();
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

	lAnalysisManager.setOutputFileName(lArgs.get<std::string>("output_file"));

	OMSimDecaysGPS &lDecays = OMSimDecaysGPS::getInstance();

	lDecays.setOpticalModule(pDetector->mOpticalModule);

	for (int i = 0; i < (int)lArgs.get<G4int>("numevents"); i++)
	{

		lDecays.simulateDecaysInOpticalModule(lArgs.get<G4double>("time_window"));

		if (lArgs.get<bool>("multiplicity_study"))
		{
			lAnalysisManager.writeMultiplicity();
			lAnalysisManager.reset();
		}
	}
}

int main(int argc, char *argv[])
{
	try
	{
		OMSim lSimulation;
		// Do not use G4String as type here...
		po::options_description lSpecific("Radioactive decays specific arguments");

		lSpecific.add_options()
		("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
		("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
		("detector_type", po::value<G4int>()->default_value(2), "module type [custom = 0, Single PMT = 1, mDOM = 2, pDDOM = 3, LOM16 = 4]")
		("no_PV_decays",po::bool_switch(),"skips the simulation of decays in pressure vessel")
		("no_PMT_decays",po::bool_switch(),"skips the simulation of decays in PMT glass")
		("multiplicity_study",po::bool_switch(),"only multiplicity is calculated and written in output. Hit information is not written in output (file would be too large!).")
		("scint_off",po::bool_switch(),"deactivates scintillation process.")
		("cherenkov_off",po::bool_switch(),"deactivates Cherenkov process.")
		("temperature",po::value<std::string>(),"temperature in C° (scintillation is temperature dependent)")
		("time_window",po::value<G4double>()->default_value(60.0),"time length in which the decays are simulated.")
		("yield_alphas",po::value<G4double>(),"scintillation yield for alpha particles. This affects all materials with scintillation properties!")
		("yield_electrons",po::value<G4double>(),"scintillation yield for electrons. This affects all materials with scintillation properties!")
		("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

		po::options_description lAllargs("Allowed input arguments");
		lAllargs.add(lSimulation.mGeneralArgs).add(lSpecific);

		po::variables_map lVariablesMap;
		try
		{
			po::store(po::parse_command_line(argc, argv, lAllargs), lVariablesMap);
		}
		catch (std::invalid_argument &e)
		{
			std::cerr << "Invalid argument: " << e.what() << std::endl;
		}
		catch (std::exception &e)
		{
			std::cerr << "An exception occurred: " << e.what() << std::endl;
		}
		catch (...)
		{
			std::cerr << "An unknown exception occurred." << std::endl;
		}

		po::notify(lVariablesMap);

		if (lVariablesMap.count("help"))
		{
			std::cout << lAllargs << "\n";
			return 1;
		}

		OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

		// Now store the parsed parameters in the OMSimCommandArgsTable instance
		for (const auto &option : lVariablesMap)
		{
			lArgs.setParameter(option.first, option.second.value());
		}

		// Now that all parameters are set, "finalize" the OMSimCommandArgsTable instance so that the parameters cannot be modified anymore
		lArgs.finalize();

		OMSimRadDecaysDetector* lDetectorConstruction = new OMSimRadDecaysDetector();
		lSimulation.initialiseSimulation(lDetectorConstruction);
		decaySimulation(lDetectorConstruction);

		if (lArgs.get<bool>("visual"))
			lSimulation.startVisualisation();
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
