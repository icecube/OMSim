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

std::shared_ptr<spdlog::logger> g_logger;

namespace po = boost::program_options;

void runSupernovaNeutrinoSimulation()
{
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();

	// TODO: Check whether this is right, since runmanager and primarygenerators are being
	// called also in OMSim.cc
	OMSimUIinterface &ui = OMSimUIinterface::getInstance();
	ui.applyCommand("/selectGun", args.getInstance().get<G4int>("SNgun"));
	ui.runBeamOn();
}

/**
 * @brief Add options for the user input arguments for the SN module
 */
void addModuleOptions(OMSim* p_simulation)
{
	po::options_description extraOptions("Effective area specific arguments");

	// Do not use G4String as type here...
	extraOptions.add_options()
	("wheight,wh", po::value<G4double>()->default_value(20), "Height of cylindrical world volume, in m")
	("wradius,wr", po::value<G4double>()->default_value(20), "Radius of cylindrical world volume, in m")
	("SNtype", po::value<G4int>()->default_value(0), "0=27 solar mass type II (ls220), 1=9.6 solar mass type II (ls220). Models 2,3,4 correspond to old tests with other models.")
	("SNgun", po::value<G4int>()->default_value(0), "Select interaction to simulate: 0=IBD (no neutron capture included), 1=ENES")
	("SNfixEnergy", po::bool_switch(), "Instead of using the energy distribution of the model, it generates all neutrinos from an energy distribution with fixed mean energy and alpha")
	("SNmeanE", po::value<G4double>()->default_value(10.0), "If --SNfixEnergy, use this mean energy to generate the neutrinos ")
	("SNalpha", po::value<G4double>()->default_value(2.5), "If --SNfixEnergy, pinching (alpha) parameter of nu/nubar energy spectrum");


	p_simulation->extendOptions(extraOptions);
}

int main(int p_argumentCount, char *p_argumentVector[])
{

	OMSim simulation;
	addModuleOptions(&simulation);
	bool successful = simulation.handleArguments(p_argumentCount, p_argumentVector);
	if (!successful)
		return 0;

	OMSimSNdetector *detectorConstruction = new OMSimSNdetector();
	simulation.initialiseSimulation(detectorConstruction);
	runSupernovaNeutrinoSimulation();

	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		simulation.startVisualisation();
	return 0;
}
