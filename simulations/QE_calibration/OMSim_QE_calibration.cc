/**
 * @file
 * @ingroup EffectiveArea
 * @brief Main for the calculation of effective areas.
 * @details The effective area of a module is calculated simulating a plane wave from a certain direction.
 */
#include "OMSim.hh"
#include "OMSimBeam.hh"
#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimTools.hh"
std::shared_ptr<spdlog::logger> globalLogger;

namespace po = boost::program_options;

void modifyPhotocathodeAbsorptionLength(G4double pAbs)
{
	G4OpticalSurface *lSurface = OMSimInputData::getInstance().getOpticalSurface("Surf_Generic_Photocathode_20nm");
	G4MaterialPropertiesTable* mMPT = lSurface->GetMaterialPropertiesTable();
	mMPT->RemoveProperty("ABSLENGTH");
	G4double energies[] = {1.5 * eV, 9 * eV};
	G4double absLengths[] = {pAbs, pAbs};
	mMPT->AddProperty("ABSLENGTH", energies, absLengths, 2);
}



void runQEbeamSimulationVaryingAbsorptionLength()
{
	OMSimEffectiveAreaAnalyisis lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	Beam *lScanner = new Beam(lArgs.get<G4double>("radius"), lArgs.get<G4double>("distance"));
	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";

	bool lWriteHeader = !lArgs.get<bool>("no_header");
	if (lWriteHeader)
		lAnalysisManager.writeHeader("Wavelength", "Abs. length");

	std::vector<double> lWavelengths = Tools::linspace(275, 750, 96);
	std::vector<double> lAbsLengths = Tools::logspace(-8.5, -5, 20);

	for (const auto &wavelength : lWavelengths)
	{
		lScanner->setWavelength(wavelength);
		
		for (const auto &abslength : lAbsLengths)
		{
			modifyPhotocathodeAbsorptionLength(abslength*m);
			lScanner->runErlangenQEBeam();
			lAnalysisManager.writeScan(wavelength, abslength);
			lHitManager.reset();
		}
	}
}

void runQEbeamSimulation()
{
	OMSimEffectiveAreaAnalyisis lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	Beam *lScanner = new Beam(lArgs.get<G4double>("radius"), lArgs.get<G4double>("distance"));
	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";

	bool lWriteHeader = !lArgs.get<bool>("no_header");
	if (lWriteHeader)
		lAnalysisManager.writeHeader("Wavelength");

	std::vector<double> lWavelengths = Tools::linspace(250, 750, 200);

	for (const auto &wavelength : lWavelengths)
	{
		lScanner->setWavelength(wavelength);
		lScanner->runErlangenQEBeam();
		lAnalysisManager.writeScan(wavelength);
		lHitManager.reset();
		
	}
}


void runXYZfrontalScan()
{
	OMSimEffectiveAreaAnalyisis lAnalysisManager;
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();

	Beam *lScanner = new Beam(0.3, lArgs.get<G4double>("distance"));
	lScanner->configureZCorrection_PicoQuant();
	lAnalysisManager.mOutputFileName = lArgs.get<std::string>("output_file") + ".dat";
	lScanner->setWavelength(459);

	std::vector<double> lX = Tools::arange(-41, 42, 1);
	double lRlim = 42;

	for (const auto &x : lX)
	{
		for (const auto &y : lX)
		{
			if (std::sqrt(x*x+y*y)< lRlim)
			{
				lScanner->runBeamPicoQuantSetup(x, y);
				lAnalysisManager.writeHitPositionHistogram(x,y);
				lHitManager.reset();
			}
		}
	}
}

/**
 * @brief Add options for the user input arguments for the effective area module
 */
void addModuleOptions(OMSim *pSimulation)
{
	po::options_description lSpecific("Effective area specific arguments");

	// Do not use G4String as type here...
	lSpecific.add_options()("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(5.0), "plane wave radius in mm")
	("distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")
	("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")
	("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")
	("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")
	("simulation_step", po::value<G4int>()->default_value(0), "simulation step to be performed (0, 1, 2)")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written");

	pSimulation->extendOptions(lSpecific);
}

int main(int pArgumentCount, char *pArgumentVector[])
{

	OMSim lSimulation;
	addModuleOptions(&lSimulation);
	bool lContinue = lSimulation.handleArguments(pArgumentCount, pArgumentVector);
	if (!lContinue)
		return 0;

	std::unique_ptr<OMSimEffectiveAreaDetector> lDetectorConstruction = std::make_unique<OMSimEffectiveAreaDetector>();
	lSimulation.initialiseSimulation(lDetectorConstruction.get());
	lDetectorConstruction.release();

   switch (OMSimCommandArgsTable::getInstance().get<G4int>("simulation_step"))
    {

    case 0:
    {
		//Vary absorption length of photocathode for different wavelengths to determine amount of detected photons
		runQEbeamSimulationVaryingAbsorptionLength();
		break;
    }
    case 1:
    {
		//After fitting the QE vs abs. length curves and making the corresponding parameter files, check that QE is being mapped correctly
		runQEbeamSimulation();
		break;
    }
    case 2:
    {
		//Scan photocathode to save absorption position. Needed to calculate collection efficiency weight.
		runXYZfrontalScan();
		break;
    }
    }


	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		lSimulation.startVisualisation();
	return 0;
}
