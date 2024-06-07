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

std::shared_ptr<spdlog::logger> globalLogger;

namespace po = boost::program_options;

/**
 * @brief Runs the decay simulation for the specified optical module.
 * @param pDetector Pointer to the OMSimRadDecaysDetector object representing the detector 
 *                  for which the decay simulation is to be performed.
 * @see OMSimDecaysGPS, OMSimDecaysAnalysis
 */
void runRadioactiveDecays(OMSimRadDecaysDetector *pDetector)
{
	OMSimDecaysAnalysis &lAnalysisManager = OMSimDecaysAnalysis::getInstance();
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

	lAnalysisManager.setOutputFileName(lArgs.get<std::string>("output_file"));

	OMSimDecaysGPS &lDecays = OMSimDecaysGPS::getInstance();
	lDecays.setOpticalModule(pDetector->mOpticalModule);
	lDecays.setProductionRadius(200*mm);
	const bool lSimulateVesselDecays = !lArgs.get<bool>("no_PV_decays");
	const bool lSimulatePMTDecays = !lArgs.get<bool>("no_PMT_decays");

	for (int i = 0; i < (int)lArgs.get<G4int>("numevents"); i++)
	{
		if (lSimulateVesselDecays)
		{
			lDecays.simulateDecaysInPressureVessel(lArgs.get<G4double>("time_window"));
		}

		if (lSimulatePMTDecays)
		{
			lDecays.simulateDecaysInPMTs(lArgs.get<G4double>("time_window"));
		}

		if (lArgs.get<bool>("multiplicity_study"))
		{
			lAnalysisManager.writeMultiplicity();
			lAnalysisManager.reset();
		}
	}
}


/**
 * @brief Add options for the user input arguments for the radioactive decays module
 */
void addModuleOptions(OMSim* pSimulation)
{
	po::options_description lSpecific("User arguments for radioactive decays simulation");

	// Do not use G4String as type here...
	lSpecific.add_options()
	("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("no_PV_decays", po::bool_switch(), "skips the simulation of decays in pressure vessel")
	("no_PMT_decays", po::bool_switch(), "skips the simulation of decays in PMT glass")
	("multiplicity_study", po::bool_switch(), "only multiplicity is calculated and written in output. Hit information is not written in output (file would be too large!).")
	("scint_off", po::bool_switch(), "deactivates scintillation process.")
	("cherenkov_off", po::bool_switch(), "deactivates Cherenkov process.")
	("temperature", po::value<std::string>(), "temperature in C° (scintillation is temperature dependent)")
	("time_window", po::value<G4double>()->default_value(60.0), "time length in which the decays are simulated.")
	("yield_alphas", po::value<G4double>(), "scintillation yield for alpha particles. This affects all materials with scintillation properties!")
	("yield_electrons", po::value<G4double>(), "scintillation yield for electrons. This affects all materials with scintillation properties!")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	pSimulation->extendOptions(lSpecific);
}

int main(int pArgumentCount, char *pArgumentVector[])
{

	OMSim lSimulation;
	addModuleOptions(&lSimulation);
	bool lContinue = lSimulation.handleArguments(pArgumentCount, pArgumentVector);
	if (!lContinue) return 0;

	OMSimRadDecaysDetector *lDetectorConstruction = new OMSimRadDecaysDetector();
	lSimulation.initialiseSimulation(lDetectorConstruction);

	runRadioactiveDecays(lDetectorConstruction);
	
	lSimulation.startVisualisationIfRequested();
	return 0;
}
