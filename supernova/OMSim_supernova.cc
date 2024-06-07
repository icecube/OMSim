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

std::shared_ptr<spdlog::logger> globalLogger;

// TODO: change this global. getInstance?
G4Navigator* gNavigator =nullptr;
void setGlobalNavigator(G4Navigator* pNavigator){gNavigator = pNavigator;}

namespace po = boost::program_options;

void prepareAnalysis() {
	OMSimSNAnalysis &lAnalysisManager = OMSimSNAnalysis::getInstance();
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	G4String lDataOutputName = lArgs.get<std::string>("output_file") + ".dat";
    G4String lInfoOutputName = lArgs.get<std::string>("output_file") + "_info.dat";
    if (!lArgs.get<bool>("SNfixEnergy")) {
    	lAnalysisManager.maininfofile.open(lInfoOutputName, std::ios::out| std::ios::trunc); 
    }
    lAnalysisManager.datafile.open(lDataOutputName, std::ios::out| std::ios::trunc); 
    lAnalysisManager.WriteHeaders();
}

void runSupernovaNeutrinoSimulation()
{
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	//TODO: Check whether this is right, since runmanager and primarygenerators are being
	//called also in OMSim.cc
	OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
	lUIinterface.applyCommand("/selectGun", lArgs.getInstance().get<G4int>("SNgun"));

	prepareAnalysis();

    lUIinterface.runBeamOn();

}


/**
 * @brief Add options for the user input arguments for the SN module
 */
void addModuleOptions(OMSim* pSimulation)
{
	po::options_description lSpecific("Effective area specific arguments");

	// Do not use G4String as type here...
	lSpecific.add_options()
	("wheight,wh", po::value<G4double>()->default_value(20), "Height of cylindrical world volume, in m")
	("wradius,wr", po::value<G4double>()->default_value(20), "Radius of cylindrical world volume, in m")
	("SNtype", po::value<G4int>()->default_value(0), "0=27 solar mass type II (ls220), 1=9.6 solar mass type II (ls220). Models 2,3,4 correspond to old tests with other models.")
	("SNgun", po::value<G4int>()->default_value(0), "Select interaction to simulate: 0=IBD (no neutron capture included), 1=ENES")
	("SNfixEnergy", po::bool_switch(), "Instead of using the energy distribution of the model, it generates all neutrinos from an energy distribution with fixed mean energy and alpha")
	("SNmeanE", po::value<G4double>()->default_value(10.0), "If --SNfixEnergy, use this mean energy to generate the neutrinos ")
	("SNalpha", po::value<G4double>()->default_value(2.5), "If --SNfixEnergy, pinching (alpha) parameter of nu/nubar energy spectrum");


	pSimulation->extendOptions(lSpecific);
}


int main(int pArgumentCount, char *pArgumentVector[])
{

	OMSim lSimulation;
	addModuleOptions(&lSimulation);
	bool lContinue = lSimulation.handleArguments(pArgumentCount, pArgumentVector);
	if (!lContinue) return 0;

	OMSimSNdetector* lDetectorConstruction = new OMSimSNdetector();
	lSimulation.initialiseSimulation(lDetectorConstruction);

	runSupernovaNeutrinoSimulation();

	lSimulation.startVisualisationIfRequested();
	return 0;
}
