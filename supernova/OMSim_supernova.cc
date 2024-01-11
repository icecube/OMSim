/**
 * @file 
 * @ingroup sngroup
 * @brief Main of the supernova study.
 * @details @see sngroup
 */
#include "OMSim.hh"
#include "OMSimSNAnalysis.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimHitManager.hh"
#include "G4Navigator.hh"
#include "OMSimSNdetector.hh"

std::shared_ptr<spdlog::logger> global_logger;

// TODO: change this global. getInstance?
G4Navigator* gNavigator =nullptr;
void setGlobalNavigator(G4Navigator* pNavigator){gNavigator = pNavigator;}

namespace po = boost::program_options;

void PrepareAnalysis() {
	OMSimSNAnalysis &lAnalysisManager = OMSimSNAnalysis::getInstance();
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	G4String ldataoutputname = lArgs.get<std::string>("output_file") + ".dat";
    G4String linfooutputname = lArgs.get<std::string>("output_file") + "_info.dat";
    if (!lArgs.get<bool>("SNfixEnergy")) {
    	lAnalysisManager.maininfofile.open(linfooutputname, std::ios::out| std::ios::trunc); 
    }
    lAnalysisManager.datafile.open(ldataoutputname, std::ios::out| std::ios::trunc); 
    lAnalysisManager.WriteHeaders();
}

void SupernovaNeutrinoSimulation()
{
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	//TODO: Check whether this is right, since runmanager and primarygenerators are being
	//called also in OMSim.cc
	OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
	lUIinterface.applyCommand("/selectGun", lArgs.getInstance().get<G4int>("SNgun"));

	PrepareAnalysis();

    lUIinterface.runBeamOn();

}


int main(int argc, char *argv[])
{
	try
	{
		OMSim lSimulation;
		setGlobalNavigator(lSimulation.getNavigator());
		// Do not use G4String as type here...
		po::options_description lSpecific("SN simulation specific arguments");

		lSpecific.add_options()
		("wheight,wh", po::value<G4double>()->default_value(20), "Height of cylindrical world volume, in m")
		("wradius,wr", po::value<G4double>()->default_value(20), "Radius of cylindrical world volume, in m")
		("SNtype", po::value<G4int>()->default_value(0), "0=27 solar mass type II (ls220), 1=9.6 solar mass type II (ls220). Models 2,3,4 correspond to old tests with other models.")
		("SNgun", po::value<G4int>()->default_value(0), "Select interaction to simulate: 0=IBD (no neutron capture included), 1=ENES")
		("SNfixEnergy", po::bool_switch(), "Instead of using the energy distribution of the model, it generates all neutrinos from an energy distribution with fixed mean energy and alpha")
		("SNmeanE", po::value<G4double>()->default_value(10.0), "If --SNfixEnergy, use this mean energy to generate the neutrinos ")
		("SNalpha", po::value<G4double>()->default_value(2.5), "If --SNfixEnergy, pinching (alpha) parameter of nu/nubar energy spectrum");


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

		OMSimSNdetector* lDetectorConstruction = new OMSimSNdetector();
		lSimulation.initialiseSimulation(lDetectorConstruction);

		SupernovaNeutrinoSimulation();

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
