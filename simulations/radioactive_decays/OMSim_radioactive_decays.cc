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
#include "Randomize.hh"
#include <G4Poisson.hh>
#include <numeric>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

namespace {
inline std::string fmt_time_precise(double t_seconds) {
    std::ostringstream oss;
    oss.setf(std::ios::scientific);               // or defaultfloat
    oss << std::setprecision(17) << t_seconds << " s";
    return oss.str();
}
}
std::shared_ptr<spdlog::logger> g_logger;

namespace po = boost::program_options;

// this is for the energy spectrum simulation
namespace MuSurf {

    // curved-atmosphere approximation for cosθ*
    inline double cosThetaStar(double costh) {
        const double P1=0.102573, P2=-0.068287, P3=0.958633, P4=0.0407253, P5=0.817285;
        const double c = std::max(-1.0, std::min(1.0, costh));
        const double num = c*c + P1*P1 + P2*std::pow(std::fabs(c), P3) + P4*std::pow(std::fabs(c), P5);
        const double den = 1.0 + P1*P1 + P2 + P4;
        return std::sqrt(std::max(0.0, num/den));
    }

    inline double gaisserBracket(double E_GeV, double cst) {
        return 1.0/(1.0 + 1.1*E_GeV*cst/115.0) + 0.054/(1.0 + 1.1*E_GeV*cst/850.0);
    }

    // Low-energy correction: (1 + 3.64 / (E * cosθ*^1.29))^{-2.7}
    inline double lowEcorr(double E_GeV, double cst) {
        return std::pow(1.0 + 3.64/(E_GeV*std::pow(cst, 1.29)), -2.7);
    }

    // Rejection sampler for E given θ using proposal ~ E^{-2.7} on [Emin,Emax]
    // Uses a SAFE global envelope WMAX to avoid bias (accept–reject correctness).
    inline double sampleEnergy(double costh, double Emin_GeV=1.0, double Emax_GeV=1000.0) {
        const double alpha = 2.7;
        const double p = 1.0 - alpha;                 // -1.7
        const double A = std::pow(Emin_GeV, p);
        const double B = std::pow(Emax_GeV, p);
        const double cst = MuSurf::cosThetaStar(costh);

        // Safe global envelope (π+K ≤ 1.054, lowE ≤ 1)
        const double WMAX = 1.06;

        for (int it=0; it<100000; ++it) {
            const double u  = G4UniformRand();
            const double E  = std::pow(A + u*(B - A), 1.0/p);   // proposal ~ E^{-2.7}
            const double w  = MuSurf::gaisserBracket(E, cst) * MuSurf::lowEcorr(E, cst);
            if (G4UniformRand() * WMAX <= w) return E;          // accept → GeV
        }
        // Extremely unlikely fallback
        return Emin_GeV;
    }

}

//############//###################################################//##########################################################################################
// -----------------------   to simulate muons for visualization   ----------------------- //
/*void runRadioactiveDecays(OMSimRadDecaysDetector *p_detector) {
    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
    OMSimDecaysGPS &decaysGPS   = OMSimDecaysGPS::getInstance();
    OMSimDecaysAnalysis &analysisManager = OMSimDecaysAnalysis::getInstance();

    decaysGPS.setOpticalModule(p_detector->m_opticalModule);

    for (int i = 0; i < (int)args.get<G4int>("numevents"); i++) {
       // double randomTheta = G4UniformRand() * 90.0;   // [deg]
       // double randomPhi   = G4UniformRand() * 360.0;  // [deg]
        double randomTheta = 0*deg;   // [deg]
        double randomPhi   = 0*deg; 
        double energyVal   = 10.0 * GeV;               // fixed energy, clearer in vis
        double time        = 0.0;                      // all start at t=0

        OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
        if (G4UniformRand() < 0.5)
            uiInterface.applyCommand("/gps/particle mu+");
        else
            uiInterface.applyCommand("/gps/particle mu-");

        uiInterface.applyCommand("/gps/time " + std::to_string(time) + " s");
        uiInterface.applyCommand("/gps/energy " + std::to_string(energyVal/GeV) + " GeV");

        decaysGPS.runSingleAngularScan(randomPhi, randomTheta);
    }
    analysisManager.mergeFiles();
}*/
//################//###################################################//######################################################################################
void runRadioactiveDecays(OMSimRadDecaysDetector *p_detector) {
    OMSimDecaysAnalysis &analysisManager = OMSimDecaysAnalysis::getInstance();
    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
    OMSimDecaysGPS &decaysGPS = OMSimDecaysGPS::getInstance();

    decaysGPS.setOpticalModule(p_detector->m_opticalModule);
    decaysGPS.setProductionRadius(280 * mm);

    const G4double T_sim = args.get<G4double>("time_window");

    // --- Set plane/disk parameters once and reuse ---
    decaysGPS.setMuonScanParameters();
    decaysGPS.configureScan();
    const double R_disk = decaysGPS.getBeamRadius() / m;    // <-- use the exact radius used by GPS
    const double A_disk = CLHEP::pi * R_disk * R_disk;   // [m^2]

    // --- intensity law from fit I(θ)=I0 cos^n θ ---
    const double I0 = 46.3;//30.4;//12.2;49.5;//70.0;//9.36;//31.0;//9.36;     // [m^-2 s^-1 sr^-1]
    const double n_mu = 2.5;//2.5;//2.0;//0.78;//1.5;//0.78;
    const double Phi_hemi = (2.0 * CLHEP::pi * I0) / (n_mu + 1.0);  // integral I dΩ over hemisphere ≈  m^-2 s^-1

    // --- Number of muons to shoot over T_sim ---
    const int numMuonEvents = G4Poisson(Phi_hemi * A_disk * T_sim);
    G4cout << "[SRC] R_disk = " << R_disk << " m,  A_disk = " << A_disk
        << " m^2,  Phi_hemi = " << Phi_hemi
        << " m^-2 s^-1,  mean N = " << (Phi_hemi*A_disk*T_sim) << G4endl;

    // --- Muon timestamps (uniform in [0, T_sim]) ---
    std::vector<double> muonTimes;
    muonTimes.reserve(numMuonEvents);
    for (int i = 0; i < numMuonEvents; ++i) muonTimes.push_back(G4UniformRand() * T_sim);
    std::sort(muonTimes.begin(), muonTimes.end());

    // --- Optional: intrinsic radioactivity ---
    const bool simulateVesselDecays = !args.get<bool>("no_PV_decays");
    const bool simulatePMTDecays    = !args.get<bool>("no_PMT_decays");
    if (simulateVesselDecays) decaysGPS.simulateDecaysInPressureVessel(T_sim);
    if (simulatePMTDecays)    decaysGPS.simulateDecaysInPMTs(T_sim);



   // std::ofstream muonLog("muons_generated.dat");   // open new log file
   // muonLog << "#time[s]\tE[GeV]\ttheta[deg]\tphi[deg]\n";

    // --- Shoot muons with correct angular PDF over solid angle ---
    for (double time : muonTimes) {
        // Sample direction: mu = cosθ ~ mu^n  =>  mu = U^{1/(n+1)}
        const double U  = G4UniformRand();
        const double mu = std::pow(U, 1.0/(n_mu + 1.0));
        const double th = std::acos(mu);                        // [rad]
        const double ph = 2.0 * CLHEP::pi * G4UniformRand();    // [rad]
        const double randomTheta = th * 180.0 / CLHEP::pi;      // [deg]
        const double randomPhi   = ph * 180.0 / CLHEP::pi;      // [deg]

// ------------------------ only power law spectrum ----------------------------------------
  /*      // Energy ~ E^{-2.7}, 1–1000 GeV (as before)
        const G4double Emin = 1 * GeV, Emax = 1000 * GeV, alpha = 2.7;
        const G4double rE   = G4UniformRand();
        const G4double E1ma = std::pow(Emin, 1.0 - alpha);
        const G4double E2ma = std::pow(Emax, 1.0 - alpha);
        const G4double energyVal = std::pow(E1ma + rE*(E2ma - E1ma), 1.0/(1.0 - alpha));*/
// -----------------------------------------------------------------------------------------
// ----------- realistic energy spectrum from modifies Gaisser model (Guan et.al 2015) -------------
        const double costh = std::cos(th);
        const double E_GeV = MuSurf::sampleEnergy(costh, 1.0, 10000.0);  // 1 GeV to 10 TeV
// -----------------------------------------------------------------------------------------


     //   muonLog << time << "\t"
     //         //  << energyVal/GeV << "\t"
     //           << E_GeV << "\t"
     //           << randomTheta << "\t"
     //           << randomPhi << "\n";

        OMSimUIinterface &uiInterface = OMSimUIinterface::getInstance();
       // uiInterface.applyCommand("/gps/particle mu+");
        if (G4UniformRand() < 0.57)
        uiInterface.applyCommand("/gps/particle mu+");
        else
        uiInterface.applyCommand("/gps/particle mu-");
        // Timestamp and fire one muon
        //uiInterface.applyCommand("/gps/time " + std::to_string(time) + " s");
        uiInterface.applyCommand(std::string("/gps/time ") + fmt_time_precise(time));

       // uiInterface.applyCommand("/gps/energy " + std::to_string(energyVal / GeV) + " GeV");
        uiInterface.applyCommand("/gps/energy " + std::to_string(E_GeV) + " GeV");

        //decaysGPS.runSingleAngularScan(randomPhi, randomTheta);
    
        // set muon direction
        double ux = sin(th)*cos(ph);
        double uy = sin(th)*sin(ph);
        double uz = -cos(th);
        uiInterface.applyCommand(fmt::format("/gps/ang/direction {} {} {}", ux, uy, uz));

        // run muon
        uiInterface.runBeamOn(1);

    }
   // muonLog.close();
    

    if (args.get<bool>("multiplicity_study")) {
        G4double coincidenceTimeWindow = args.get<double>("multiplicity_time_window") * ns;
        analysisManager.writeMultiplicity(coincidenceTimeWindow);

        std::vector<int> multiplicity = OMSimHitManager::getInstance().calculateMultiplicity(coincidenceTimeWindow);
        double totalMultiplicity = std::accumulate(multiplicity.begin(), multiplicity.end(), 0.0);
        double multiplicityRate  = totalMultiplicity / T_sim;

        std::cout << "T_sim: " << T_sim << " s\n";
       // std::cout << "Total Multiplicity Rate: " << multiplicityRate << " s^-1\n";
       // std::cout << "Multiplicity Rates (per second):\n";
       // for (size_t i = 0; i < multiplicity.size(); ++i)
       //     std::cout << "Multiplicity " << i << ": " << (multiplicity[i] / T_sim) << " s^-1\n";

        analysisManager.reset();
    }

    analysisManager.mergeFiles();
}


//-------------------------------------------------------------------------------------------------------------

// only radioactive decays
/*
void runRadioactiveDecays(OMSimRadDecaysDetector *p_detector)
{
    OMSimDecaysAnalysis &analysisManager = OMSimDecaysAnalysis::getInstance();
    OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();

    OMSimDecaysGPS &decaysGPS = OMSimDecaysGPS::getInstance();
    decaysGPS.setOpticalModule(p_detector->m_opticalModule);
    decaysGPS.setProductionRadius(280*mm);
    const bool simulateVesselDecays = !args.get<bool>("no_PV_decays");
    const bool simulatePMTDecays    = !args.get<bool>("no_PMT_decays");

    for (int i = 0; i < (int)args.get<G4int>("numevents"); i++)
    {
        if (simulateVesselDecays)
            decaysGPS.simulateDecaysInPressureVessel(args.get<G4double>("time_window"));

        if (simulatePMTDecays)
            decaysGPS.simulateDecaysInPMTs(args.get<G4double>("time_window"));

        if (args.get<bool>("multiplicity_study"))
        {
            G4double coincidenceTimeWindow = args.get<double>("multiplicity_time_window")*ns;
            analysisManager.writeMultiplicity(coincidenceTimeWindow);
            analysisManager.reset();
        }
    }
    analysisManager.mergeFiles();
}*/


/**
 * @brief Add options for the user input arguments for the radioactive decays module
 */
void addModuleOptions(OMSim* p_simulation)
{
	po::options_description moduleOptions("User arguments for radioactive decays simulation");

	// Do not use G4String as type here...
	moduleOptions.add_options()
	("world_radius,w", po::value<G4double>()->default_value(20.0), "radius of world sphere in m")
	("radius,r", po::value<G4double>()->default_value(300.0), "plane wave radius in mm")
	("no_PV_decays", po::bool_switch(), "skips the simulation of decays in pressure vessel")
	("no_PMT_decays", po::bool_switch(), "skips the simulation of decays in PMT glass")
	("multiplicity_study", po::bool_switch(), "only multiplicity is calculated and written in output. Hit information is not written in output (file would be too large!).")
	("scint_off", po::bool_switch(), "deactivates scintillation process.")
	("cherenkov_off", po::bool_switch(), "deactivates Cherenkov process.")
	("temperature", po::value<std::string>(), "temperature in C° (scintillation is temperature dependent)")
	("time_window", po::value<G4double>()->default_value(60.0), "time length in which the decays are simulated.")
	("multiplicity_time_window", po::value<double>()->default_value(75.), "time window in ns for coincidences in multiplicity calculation")
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