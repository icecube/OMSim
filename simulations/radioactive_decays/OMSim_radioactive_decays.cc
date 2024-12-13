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

std::shared_ptr<spdlog::logger> g_logger;

namespace po = boost::program_options;

/**
 * @brief Runs the decay simulation for the specified optical module.
 * @param p_detector Pointer to the OMSimRadDecaysDetector object representing the detector 
 *                  for which the decay simulation is to be performed.
 * @see OMSimDecaysGPS, OMSimDecaysAnalysis
 */
void runRadioactiveDecays(OMSimRadDecaysDetector *p_detector)
{
	OMSimDecaysAnalysis &analysisManager = OMSimDecaysAnalysis::getInstance();
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();

	OMSimDecaysGPS &decaysGPS = OMSimDecaysGPS::getInstance();
	decaysGPS.setOpticalModule(p_detector->m_opticalModule);
	decaysGPS.setProductionRadius(280*mm);
	const bool simulateVesselDecays = !args.get<bool>("no_PV_decays");
	const bool simulatePMTDecays = !args.get<bool>("no_PMT_decays");

	for (int i = 0; i < (int)args.get<G4int>("numevents"); i++)
	{
		if (simulateVesselDecays)
		{
			decaysGPS.simulateDecaysInPressureVessel(args.get<G4double>("time_window"));
		}

		if (simulatePMTDecays)
		{
			decaysGPS.simulateDecaysInPMTs(args.get<G4double>("time_window"));
		}

		if (args.get<bool>("multiplicity_study"))
		{
			G4double coincidenceTimeWindow = args.get<double>("multiplicity_time_window")*ns;
			analysisManager.writeMultiplicity(coincidenceTimeWindow);
			analysisManager.reset();
		}
	}
}


/**
 * @brief Add options for the user input arguments for the radioactive decays module
 */
void addModuleOptions(OMSim* p_simulation)
{
	po::options_description moduleOptions("User arguments for radioactive decays simulation");

	// Do not use G4String as type here...
	moduleOptions.add_options()
	("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("no_PV_decays", po::bool_switch(), "skips the simulation of decays in pressure vessel")
	("no_PMT_decays", po::bool_switch(), "skips the simulation of decays in PMT glass")
	("multiplicity_study", po::bool_switch(), "only multiplicity is calculated and written in output. Hit information is not written in output (file would be too large!).")
	("scint_off", po::bool_switch(), "deactivates scintillation process.")
	("cherenkov_off", po::bool_switch(), "deactivates Cherenkov process.")
	("temperature", po::value<std::string>(), "temperature in C° (scintillation is temperature dependent)")
	("time_window", po::value<G4double>()->default_value(60.0), "time length in which the decays are simulated.")
	("multiplicity_time_window", po::value<double>()->default_value(20.), "time window in ns for coincidences in multiplicity calculation")
	("yield_alphas", po::value<G4double>(), "scintillation yield for alpha particles. This affects all materials with scintillation properties!")
	("yield_electrons", po::value<G4double>(), "scintillation yield for electrons. This affects all materials with scintillation properties!")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	p_simulation->extendOptions(moduleOptions);
}

int main(int p_argumentCount, char *p_argumentVector[])
{

	OMSim simulation;
	addModuleOptions(&simulation);
	bool successful = simulation.handleArguments(p_argumentCount, p_argumentVector);
	if (!successful) return 0;

	OMSimRadDecaysDetector *detectorConstruction = new OMSimRadDecaysDetector();
	simulation.initialiseSimulation(detectorConstruction);

	runRadioactiveDecays(detectorConstruction);
	
	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		simulation.startVisualisation();
	return 0;
}
