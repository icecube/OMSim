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

std::shared_ptr<spdlog::logger> g_logger;

namespace po = boost::program_options;

void runYieldSetup(OMSimYieldDetector *pDetector)
{
	OMSimDecaysAnalysis &lAnalysisManager = OMSimDecaysAnalysis::getInstance();
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
	//lAnalysisManager.setOutputFileName(lArgs.get<std::string>("output_file"));

	OMSimYieldGPS &lDecays = OMSimYieldGPS::getInstance();

	switch (lArgs.get<G4int>("detector_type"))
	{
	case 0:
	{
		lDecays.setEmitterVolume(pDetector->m_source);
		lDecays.setProductionRadius(1*cm);
		lDecays.configureGammaEmitter(661.7*keV, "Cs137");
		//lDecays.limitThetaEmission(90*deg, 180*deg);
		lUIinterface.runBeamOn();
		break;
	}
	case 1:
	{
		lDecays.setEmitterVolume(pDetector->m_source);
		lDecays.setProductionRadius(1.1*cm);
		lDecays.configureAm241Emitter("Emitter");
		//lDecays.limitThetaEmission(90*deg, 180*deg);
		lUIinterface.runBeamOn();
		break;
	}
	case 2:
	{
		lDecays.setEmitterVolume(pDetector->m_source);
		lDecays.setProductionRadius(1.1*cm);
		lDecays.configureAm241EmitterForActivity("Emitter");
		//lDecays.limitThetaEmission(90*deg, 180*deg);
		lUIinterface.runBeamOn();
		break;
	}
	}
	lAnalysisManager.writeHitInformation();
	lAnalysisManager.countHits();
}

/**
 * @brief Add options for the user input arguments for the radioactive decays module
 */
void addModuleOptions(OMSim *pSimulation)
{
	po::options_description lSpecific("User arguments for radioactive decays simulation");

	// Do not use G4String as type here...
	lSpecific.add_options()("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("scint_off", po::bool_switch(), "deactivates scintillation process.")
	("cherenkov_off", po::bool_switch(), "deactivates Cherenkov process.")
	("temperature", po::value<std::string>(), "temperature in C° (scintillation is temperature dependent)")
	("yield_alphas", po::value<G4double>(), "scintillation yield for alpha particles. This affects all materials with scintillation properties!")
	("yield_electrons", po::value<G4double>(), "scintillation yield for electrons. This affects all materials with scintillation properties!")
	("systematics",  po::bool_switch(), "The construction of the setup is varied each run sampling using the given uncertainties")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	pSimulation->extendOptions(lSpecific);
}

int main(int p_argumentCount, char *p_argumentVector[])
{

	OMSim simulation;
	addModuleOptions(&simulation);
	bool successful = simulation.handleArguments(p_argumentCount, p_argumentVector);
	if (!successful) return 0;

	OMSimYieldDetector *detectorConstruction = new OMSimYieldDetector();

	simulation.initialiseSimulation(detectorConstruction);

	runYieldSetup(detectorConstruction);

	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		simulation.startVisualisation();
	return 0;
}
