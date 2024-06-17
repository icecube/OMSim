/**
 * @file
 * @ingroup radioactive
 * @brief Main of the radioactive decay study.
 * @details @see radioactive
 * @warning
 *    This study has been tested only for Vitrovex glass (mDOM/LOM16) and the 80mm mDOM PMTs. Okamoto glass (D-Egg/LOM18) is currently under investigation.
 */
#include "OMSim.hh"
#include "OMSimYieldGPS.hh"
#include "OMSimHitManager.hh"
#include "OMSimDecaysAnalysis.hh"
#include "OMSimYieldDetector.hh"

std::shared_ptr<spdlog::logger> globalLogger;

namespace po = boost::program_options;

void runRadioactiveDecays(OMSimRadDecaysDetector *pDetector)
{
	OMSimDecaysAnalysis &lAnalysisManager = OMSimDecaysAnalysis::getInstance();
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
	lAnalysisManager.setOutputFileName(lArgs.get<std::string>("output_file"));

	OMSimYieldGPS &lDecays = OMSimYieldGPS::getInstance();

	switch (lArgs.get<G4int>("detector_type"))
	{
	case 0:
		lDecays.setEmitterVolume(pDetector->mSource);
		lDecays.setProductionRadius(200 * mm);
		lDecays.configureGammaEmitter();
		lUIinterface.runBeamOn();
		break;
	}
}

/**
 * @brief Add options for the user input arguments for the radioactive decays module
 */
void addModuleOptions(OMSim *pSimulation)
{
	po::options_description lSpecific("User arguments for radioactive decays simulation");

	// Do not use G4String as type here...
	lSpecific.add_options()("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")("no_PV_decays", po::bool_switch(), "skips the simulation of decays in pressure vessel")("no_PMT_decays", po::bool_switch(), "skips the simulation of decays in PMT glass")("multiplicity_study", po::bool_switch(), "only multiplicity is calculated and written in output. Hit information is not written in output (file would be too large!).")("scint_off", po::bool_switch(), "deactivates scintillation process.")("cherenkov_off", po::bool_switch(), "deactivates Cherenkov process.")("temperature", po::value<std::string>(), "temperature in C° (scintillation is temperature dependent)")("time_window", po::value<G4double>()->default_value(60.0), "time length in which the decays are simulated.")("yield_alphas", po::value<G4double>(), "scintillation yield for alpha particles. This affects all materials with scintillation properties!")("yield_electrons", po::value<G4double>(), "scintillation yield for electrons. This affects all materials with scintillation properties!")("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	pSimulation->extendOptions(lSpecific);
}

int main(int pArgumentCount, char *pArgumentVector[])
{

	OMSim lSimulation;
	addModuleOptions(&lSimulation);
	bool lContinue = lSimulation.handleArguments(pArgumentCount, pArgumentVector);
	if (!lContinue)
		return 0;

	OMSimRadDecaysDetector *lDetectorConstruction = new OMSimRadDecaysDetector();
	lSimulation.initialiseSimulation(lDetectorConstruction);

	runRadioactiveDecays(lDetectorConstruction);

	lSimulation.startVisualisationIfRequested();
	return 0;
}
