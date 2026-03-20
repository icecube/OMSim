/**
 * @file 
 * @ingroup EffectiveArea
 * @brief Main for the calculation of effective areas.
 * @details The effective area of a module is calculated simulating a plane wave from a certain direction.
 */

#include "OMSim.hh"
#include "OMSimAngularScan.hh"
#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimTools.hh"

std::shared_ptr<spdlog::logger> g_logger;
namespace po = boost::program_options;

void runEffectiveAreaSimulation()
{
	OMSimEffectiveAreaAnalyisis analysisManager;
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();
	AngularScan *scanner = new AngularScan(args.get<G4double>("radius"), args.get<G4double>("distance"), args.get<G4double>("wavelength"));

	analysisManager.m_outputFileName = args.get<std::string>("output_file") + ".dat";
	analysisManager.m_outputFileNameInfo = args.get<std::string>("output_file") + "_info.dat"; 
    analysisManager.m_outputFileNameMultiplicity = args.get<std::string>("output_file") + "_multiplicity.dat";

	bool writeHeader = !args.get<bool>("no_header");
	if (writeHeader) analysisManager.writeHeader("Phi", "Theta");

	// If angle file is provided, run over all angle pairs in file
	if (args.keyExists("angles_file"))
	{
		std::vector<G4PV2DDataVector> data = Tools::loadtxt(args.get<std::string>("angles_file"), true);
		std::vector<G4double> thetas = data.at(0);
		std::vector<G4double> phis = data.at(1);

		for (std::vector<int>::size_type i = 0; i != thetas.size(); i++)
		{
			scanner->runSingleAngularScan(phis.at(i), thetas.at(i));
			analysisManager.writeScan(phis.at(i), thetas.at(i),  args.get<G4double>("wavelength"));
			hitManager.reset();
		}
	}
	// If file with angle pairs was not provided, use the angle pairs provided through command-line arguments
// -----------------------------------------------------------------------------------------
	// ####### turn this block on to simulate only single particles for visualization #######
	// ############### and make sure to pass energy in OMSimAngularScan.cc ##################
	/*else
	{	
		for (int i = 0; i < OMSimCommandArgsTable::getInstance().get<G4int>("numevents"); ++i)
		{
			double randomTheta = 90*deg;
          	double randomPhi = 0*deg;
			//double randomTheta = G4UniformRand() * 90.0;
			//double randomPhi = G4UniformRand() * 360.0;
			scanner->runSingleAngularScan(randomPhi, randomTheta);
			analysisManager.writeScan(randomPhi, randomTheta);
			analysisManager.writeMultiplicity(args.get<G4double>("timeWindow_multiplicity"));
			hitManager.reset();
		}
	}
}*/

// -----------------------------------------------------------------------------------------
// ####### turn this block on to simulate the muon flux for a certain time #######
	else
	{
		//if (args.get<bool>("multiplicity_study"))
	//	{
			// --- Parameters from fit (adjust as needed) ---
		int T_sim = 100; // Simulation time in seconds
		//double A = 87.20;     // [s^-1]
		//double B = 25.57;     // [s^-1]
		//double n = 1.61;      // Cosine power
		//double phi_real = args.get<G4double>("phi_real");
		//double phi_real = 0.00287*4; // muons/cm²/s from integration 0.001796
// ----------------------------------------------		
		double A = 0.017;
		double B=0.0;
		double n=2.0;
		double phi_real=0.0356;
//------------------------------------------------
		int A_sim = 129214;   // Emission area in cm²
		int numEvents = static_cast<int>(T_sim * A_sim * phi_real);

		for (int i = 0; i < numEvents; ++i)
		{
			// Sample theta from A*cos^n(theta) + B
			//double theta_min = args.get<G4double>("theta_min");
			//double theta_max = args.get<G4double>("theta_max");
			double randomTheta, acceptProb;
			do {
				randomTheta = G4UniformRand() * 90.0; // degrees
				//randomTheta = theta_min + G4UniformRand() * (theta_max - theta_min);
				//acceptProb = (A * std::pow(std::cos(randomTheta * CLHEP::pi / 180.0), n) + B) / (A + B);
                                acceptProb = (std::pow(std::cos(randomTheta * CLHEP::pi / 180.0), n) + B);

			} while (G4UniformRand() > acceptProb);
        	//std::cout << "theta: " << randomTheta << std::endl;

			double randomPhi = G4UniformRand() * 360.0;
			// Sample energy from power-law spectrum
			G4double Emin = 1 * GeV;
			G4double Emax = 1000 * GeV;
			G4double alpha = 2.7;
			G4double r = G4UniformRand();
			G4double energyVal = std::pow(
				std::pow(Emin, 1.0 - alpha) +
				r * (std::pow(Emax, 1.0 - alpha) - std::pow(Emin, 1.0 - alpha)),
				1.0 / (1.0 - alpha)
			);

			OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
			uiInterface.applyCommand("/gps/energy " + std::to_string(energyVal / GeV) + " GeV");

			analysisManager.setMuonEnergy(energyVal);
			scanner->runSingleAngularScan(randomPhi, randomTheta);
			analysisManager.writeScan(randomPhi, randomTheta);
    		analysisManager.accumulateMultiplicity(args.get<G4double>("timeWindow_multiplicity"));
			//analysisManager.writeMultiplicity(args.get<G4double>("timeWindow_multiplicity"));

			hitManager.reset();
		}
		analysisManager.writeTotalMultiplicity(args.get<G4double>("timeWindow_multiplicity"));
		//}
	}
}



/**
 * @brief Add options for the user input arguments for the effective area module
 */

void addModuleOptions(OMSim* p_simulation)
{
	po::options_description effectiveAreaOptions("Effective area specific arguments");

	// Do not use G4String as type here...
	effectiveAreaOptions.add_options()
	("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")
	("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")
	("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")
	("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")
	("multiplicity_study", po::bool_switch(), "only multiplicity is calculated and written in output. Hit information is not written in output (file would be too large!).")
	("timeWindow_multiplicity,m", po::value<G4double>()->default_value(75.0), "Time window for multiplicity in ns")
	("theta_min", po::value<G4double>()->default_value(0.0), "minimum theta in deg")
	("theta_max", po::value<G4double>()->default_value(15.0), "maximum theta in deg")
	("phi_real", po::value<G4double>()->default_value(0.00287), "real muon flux per cm^2 per s")
	("angles_file,i", po::value<std::string>(), "The input angle pairs file to be scanned. The file should contain two columns, the first column with the theta (zenith) and the second with phi (azimuth) in degrees.")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written")
	("scint_off", po::bool_switch(), "deactivates scintillation process")
    	("yield_alphas", po::value<G4double>(), "scintillation yield for alpha particles")
    	("yield_electrons", po::value<G4double>(), "scintillation yield for electrons");
    	po::options_description moduleOptions("Module-specific options");
    	moduleOptions.add_options()
        ("energy,e", po::value<G4double>()->default_value(100.0), "Energy of the particle in MeV");
    	p_simulation->extendOptions(moduleOptions);
	p_simulation->extendOptions(effectiveAreaOptions);
}

int main(int p_argCount, char *p_argumentVector[])
{

	OMSim simulation;
	addModuleOptions(&simulation);
	bool successful = simulation.handleArguments(p_argCount, p_argumentVector);
	if (!successful)
		return 0;

	std::unique_ptr<OMSimEffectiveAreaDetector> detectorConstruction = std::make_unique<OMSimEffectiveAreaDetector>();
	simulation.initialiseSimulation(detectorConstruction.get());
	detectorConstruction.release();

	runEffectiveAreaSimulation();

	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		simulation.startVisualisation();

	return 0;
}




/**

@file
@ingroup EffectiveArea
@brief Main for the calculation of effective areas.
@details The effective area of a module is calculated simulating a plane wave from a certain direction. */
/*
#include "OMSim.hh"
#include "OMSimAngularScan.hh"
#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimEffectiveAreaDetector.hh"
#include "OMSimLogger.hh"
#include "OMSimTools.hh"
#include <numeric>

//#include "G4UniformRand.hh" #include "Randomize.hh"
//#include <chrono> #include <chrono>

std::shared_ptr<spdlog::logger> g_logger;
namespace po = boost::program_options;

void runEffectiveAreaSimulation()
{
    OMSimEffectiveAreaAnalyisis analysisManager;
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
    OMSimHitManager &hitManager = OMSimHitManager::getInstance();
    AngularScan *scanner = new AngularScan(args.get<G4double>("radius"), args.get<G4double>("distance"), args.get<G4double>("wavelength"));

    analysisManager.m_outputFileName = args.get<std::string>("output_file") + ".dat";
    analysisManager.m_outputFileNameMultiplicity = args.get<std::string>("output_file") + "_multiplicity.dat";
    analysisManager.m_outputFileNameInfo = args.get<std::string>("output_file") + "_info.dat";
    bool lWriteHeader = !args.get<bool>("no_header");
    if (lWriteHeader) analysisManager.writeHeader("Phi", "Theta", "Wavelength");

    // If angle file is provided, run over all angle pairs in file
    if (args.keyExists("angles_file"))
    {
		std::vector<G4PV2DDataVector> data = Tools::loadtxt(args.get<std::string>("angles_file"), true);
		std::vector<G4double> thetas = data.at(0);
		std::vector<G4double> phis = data.at(1);

        for (std::vector<int>::size_type i = 0; i != thetas.size(); i++)
        {
            scanner->runSingleAngularScan(phis.at(i), thetas.at(i));
            analysisManager.writeScan(phis.at(i), thetas.at(i),  args.get<G4double>("wavelength"));
            analysisManager.writeMultiplicity(args.get<G4double>("timeWindow_multiplicity"));
            hitManager.reset();
        }
    }

    // If file was not provided, use random generated theta and phi values
    else
    {
        // FIT from real measurement with scintillation panels (area 365.5 cm^2): A*cos(theta)^n + B
        // A and B are calculated for area A_sim=129214.22 cm^2 and per s
        // Fit for area 365.5 cm^2: A=14.80 min^-1, B=4.34 min^-1, n=1.61
        int T_sim = 1;         // since we are dealing in seconds, this is the simulation time in seconds
        double B = 25.57;        // offset [s^-1]
        double n = 1.61;         // power
        double A = 87.20;        // amplitude [s^-1]
        int A_sim = 100;//129214.22;   // emission plane area [cm^2] (in OMSimAngularScan.cc)
        double phi_real = 0.001796;// "real" muon flux in muons/cm^2/s (derived from fit parameters A, B and n; integration of A*cos(theta)^n+B over hemissphere)
// ---------------------------------------------------------------------------------------------------------------------------//
        /*  ----- FOR IDEAL FLUX -------
        double B = 0; 
        double n = 2;
        double A = 0.017; // ideal muon flux at sea level; muons/cm^2/s
        double phi_real = 0.036;*/
// -----------------------------------------------------------------------------------------------------------------------------//
 /*       int numEvents = static_cast<int>(T_sim * A_sim * phi_real);

        for (int i=0;i<numEvents;i++)
        {
            // cosine distribution A*cos(theta)^n
            double randomTheta, acceptProb;
            do {
                randomTheta = G4UniformRand() * 90.0;
                acceptProb = (A * std::pow(std::cos(randomTheta * M_PI / 180.0), n) + B) / (A + B);
            }
            while (G4UniformRand() > acceptProb);

            double randomPhi = G4UniformRand() * 360.0;


    //Sample energy
            G4double Emin = 1 * GeV;
            G4double Emax = 1000 * GeV;
            G4double alpha = 2.7;
            G4double r = G4UniformRand();
            G4double energyVal = std::pow(std::pow(Emin, 1.0 - alpha) + r * (std::pow(Emax, 1.0 - alpha) - std::pow(Emin, 1.0 - alpha)),1.0 / (1.0 - alpha));

            // Set the energy in GPS
            OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
            lUIinterface.applyCommand("/gps/energy " + std::to_string(energyVal/GeV) + " GeV");

         //   G4cout << "Muon Energy sampled: " << energyVal/GeV << " GeV" << G4endl;

            analysisManager.setMuonEnergy(energyVal);
            scanner->runSingleAngularScan(randomPhi, randomTheta);
            analysisManager.writeScan(randomPhi, randomTheta);
            analysisManager.writeMultiplicity(args.get<G4double>("timeWindow_multiplicity"));
            hitManager.reset();
        }
       /* for (int i = 0; i < OMSimCommandArgsTable::getInstance().get<G4int>("numevents"); ++i)
        {   
            OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

            double randomTheta = G4UniformRand() * 90.0;
            double randomPhi = G4UniformRand() * 360.0;
            // double randomTheta = 0*deg;
           //  double randomPhi = 0*deg;
            G4double energyVal = 80*MeV;
            lUIinterface.applyCommand("/gps/energy " + std::to_string(energyVal));
            analysisManager.setMuonEnergy(energyVal);
            scanner->runSingleAngularScan(randomPhi, randomTheta);
          //  analysisManager.writeScan(randomPhi, randomTheta);
            analysisManager.writeMultiplicity(args.get<G4double>("timeWindow_multiplicity"));
           // analysisManager.writeHitInformation(i);
            hitManager.reset();
        }*/
//    }
//}
/*
void addModuleOptions(OMSim* p_simulation)
{
	po::options_description effectiveAreaOptions("Effective area specific arguments");

	// Do not use G4String as type here...
	effectiveAreaOptions.add_options()
	("world_radius,w", po::value<G4double>()->default_value(3.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("distance,d", po::value<G4double>()->default_value(2000), "plane wave distance from origin, in mm")
	("theta,t", po::value<G4double>()->default_value(0.0), "theta (= zenith) in deg")
	("phi,f", po::value<G4double>()->default_value(0.0), "phi (= azimuth) in deg")
	("wavelength,l", po::value<G4double>()->default_value(400.0), "wavelength of incoming light in nm")
	("multiplicity_study", po::bool_switch(), "only multiplicity is calculated and written in output. Hit information is not written in output (file would be too large!).")
	("timeWindow_multiplicity,m", po::value<G4double>()->default_value(75.0), "Time window for multiplicity in ns")
	("angles_file,i", po::value<std::string>(), "The input angle pairs file to be scanned. The file should contain two columns, the first column with the theta (zenith) and the second with phi (azimuth) in degrees.")
	("no_header", po::bool_switch(), "if given, the header of the output file will not be written")
    ("scint_off", po::bool_switch(), "deactivates scintillation process")
    ("yield_alphas", po::value<G4double>(), "scintillation yield for alpha particles")
    ("yield_electrons", po::value<G4double>(), "scintillation yield for electrons");
    po::options_description lAllargs("Allowed input arguments");
	p_simulation->extendOptions(effectiveAreaOptions);
}

int main(int p_argCount, char *p_argumentVector[])
{

	OMSim simulation;
	addModuleOptions(&simulation);
	bool successful = simulation.handleArguments(p_argCount, p_argumentVector);
	if (!successful)
		return 0;

	std::unique_ptr<OMSimEffectiveAreaDetector> detectorConstruction = std::make_unique<OMSimEffectiveAreaDetector>();
	simulation.initialiseSimulation(detectorConstruction.get());
	detectorConstruction.release();

	runEffectiveAreaSimulation();

	if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
		simulation.startVisualisation();

	return 0;
}*/
