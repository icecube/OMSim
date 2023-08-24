/**
 * @file OMSim_effective_area.cc
 * @ingroup EffectiveArea
 * @brief Main for the calculation of effective areas.
 * @details The effective area of a module is calculated simulating a plane wave from a certain direction.
 * The photon generation is made with the module AngularScan, running the method runSingleAngularScan(phi, theta) once for each direction to be investigated.
 * Check command line arguments with --help.
 */
#include "OMSim.hh"
#include "mdomAnalysisManager.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "mdomPrimaryGeneratorAction.hh"

namespace po = boost::program_options;

void SupernovaNeutrinoSimulation()
{
	mdomAnalysisManager lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
	AngularScan *lScanner = new AngularScan(lArgs.get<G4double>("radius"), lArgs.get<G4double>("distance"), lArgs.get<G4double>("wavelength"));

	// #TODO 
	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";
	lAnalysisManager.writeHeader();
	// #TODO


	// If file with angle pairs is not provided, use arg theta & phi
	if (!lArgs.keyExists("angles_file"))
	{
		// Use the angle pairs provided through command-line arguments
		lScanner->runSingleAngularScan(lArgs.get<G4double>("phi"), lArgs.get<G4double>("theta"));
		lAnalysisManager.writeScan(lArgs.get<G4double>("phi"), lArgs.get<G4double>("theta"));
		lHitManager.reset();
	}
	// File is provided, run over all angle pairs
	else
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
}


int main(int argc, char *argv[])
{
	try
	{
		OMSim lSimulation;
		// Do not use G4String as type here...
		po::options_description lSpecific("SN simulation specific arguments");

		lSpecific.add_options()
		("wheight,wh", po::value<G4double>()->default_value(20), "Height of cylindrical world volume, in m")
		("wradius,wr", po::value<G4double>()->default_value(20), "Radius of cylindrical world volume, in m")
		("SNtype", po::value<G4int>()->default_value(0), "0=27 solar mass type II (ls220), 1=9.6 solar mass type II (ls220)")
		("SNgun", po::value<G4int>()->default_value(0), "Select interaction to simulate: 0=ENES, 1=IBD (no neutron capture included)")
		("SNmeanE", po::value<G4double>()->default_value(10.0), "Instead of using SNmodel, use this mean energy to generate the neutrinos ")
		("SNalpha", po::value<G4double>()->default_value(2.5), "Pinching parameter of nu/nubar energy spectrum when using --SNmeanE");


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

		lSimulation.initialiseSimulation();

		//TODO: Check whether this is right, since runmanager and primarygenerators are being
		//called also in OMSim.cc
	    G4RunManager *mRunManager = new G4RunManager;
		G4VUserPrimaryGeneratorAction* gen_action = new mdomPrimaryGeneratorAction();
		mRunManager->SetUserAction(gen_action);

		OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
		lUIinterface.applyCommand("/selectGun", lArgs.getInstance().get<G4double>("SNgun"));

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
