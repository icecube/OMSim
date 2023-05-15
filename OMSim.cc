#include "OMSimDetectorConstruction.hh"
#include "OMSimPhysicsList.hh"
#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimRunAction.hh"
#include "OMSimEventAction.hh"
#include "OMSimTrackingAction.hh"
#include "OMSimSteppingAction.hh"
#include "OMSimSteppingVerbose.hh"
#include "OMSimAnalysisManager.hh"
#include "OMSimPMTResponse.hh"
#include "OMSimArgumentTable.hh"

#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4UIterminal.hh"
#include "G4ThreeVector.hh"
#include "G4Navigator.hh"

#include "G4UItcsh.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"	//xxx

#include "argtable2.h"
#include <ctime>
#include <sys/time.h>

#include <sstream>
#include <iostream>
#include <fstream>
#include <string>

#include <cmath>	// for abs() of doubles
//since Geant4.10: include units manually
#include "G4SystemOfUnits.hh"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <TGraph.h>

#include <boost/program_options.hpp>
namespace po = boost::program_options;

OMSimPMTResponse* gPhotocathodeResponse = new OMSimPMTResponse();

unsigned int	stats_buffer_max_size = 10;	// how many hits to keep in memory before purging to file in EndOfEventAction
unsigned int	stats_buffer_last_purge_at = 0;	// at what hits count was the hits file last written to
// std::vector<G4int>	stats_PMT_hit;
// std::vector<G4int>	stats_OM_hit;

// double hc_eVnm = 1239.84193; // h*c in eV * nm
double pi = M_PI;
G4double	gworldsize;
G4int		gsimevents;
G4String	ggunfilename;
G4double	gDistance;
G4double	gBeamDiam;
G4double	gtheta;
G4double	gphi;
G4double    gwavelen;
G4String	ghitsfilename;
G4String	gHittype;
G4int		gPMT;
G4int		gEnvironment;
G4bool		gVisual;
G4bool		gCADImport;
G4bool 		gPlaceHarness;
G4bool		gInteractive;
G4bool		gHeader;

G4int		gGlass;
G4int		gGel;
G4double	gRefCone_angle;
G4int		gConeMat;
G4int		gHolderColor;
G4int		gDOM;
G4int       gHarness;
G4int       gRopeNumber;


// G4String	greffilename;

G4bool		gKillAll;
G4long		current_event_id;

struct timeval	gTime_Run_Start;
struct timeval	gTime_Run_End;
long randseed;

OMSimAnalysisManager gAnalysisManager;






// void clearstats() {
// 	stats_PMT_hit.clear();
// 	stats_OM_hit.clear();
// }

std::vector<G4String> explode(G4String s, char d) {
	std::vector<G4String> o;
	int i,j;
	i = s.find_first_of("#");
	if (i == 0) return o;
 	while (s.size() > 0) {
		i = s.find_first_of(d);
		j = s.find_last_of(d);
		o.push_back(s.substr(0, i));
		if (i == j) {
			o.push_back(s.substr(j+1));
			break;
		}
		s.erase(0,i+1);
 	}
	return o;// o beinhaltet s ohne d
}
std::vector<G4String> explode(char* cs, char d) {
	std::vector<G4String> o;
	G4String s = cs;
	return explode(s,d);
}
std::vector<double> readColumnDouble (G4String fn, int col) {
	std::vector<double>	values;
	unsigned int c;
	double	a;
	c = col;
	std::ifstream	infile;
	std::vector<G4String> n;
	char l[256];
	G4String l2;
	infile.open(fn);
	while (infile.good() && !infile.eof()) {
		infile.getline(l,255);
		l2 = l;
		n = explode(l2,'\t');
		if (n.size()>=c) {
			a = atof(n.at(c-1));
			values.push_back(a);
		}
	}
	infile.close();

	return values;//values enthält den c. Wert aus fn (aus jeder Spalte,welche  nach 255 zeichen oder durch \n beendet wird?)
}

int OMSim() {
	struct timeval time_for_randy;
	gettimeofday(&time_for_randy, NULL);

	randseed = time_for_randy.tv_sec+4294*time_for_randy.tv_usec;
	CLHEP::HepRandom::setTheEngine(new CLHEP::RanluxEngine(randseed,3));
	
	std::stringstream command;

	G4RunManager* runManager = new G4RunManager;

	OMSimDetectorConstruction* detector;
	detector = new OMSimDetectorConstruction();
	runManager->SetUserInitialization(detector);


	G4VUserPhysicsList* physics = new OMSimPhysicsList;
	runManager->SetUserInitialization(physics);


 		auto visManager = new G4VisExecutive;
 		visManager->Initialize();
 		

	G4VUserPrimaryGeneratorAction* gen_action = new OMSimPrimaryGeneratorAction();
	runManager->SetUserAction(gen_action);

	G4UserRunAction* run_action = new OMSimRunAction();
	runManager->SetUserAction(run_action);

	G4UserEventAction* event_action = new OMSimEventAction();
	runManager->SetUserAction(event_action);

 	G4UserTrackingAction* tracking_action = new OMSimTrackingAction();
 	runManager->SetUserAction(tracking_action);

	G4UserSteppingAction* stepping_action = new OMSimSteppingAction();
	runManager->SetUserAction(stepping_action);

	runManager->Initialize();
    
	auto UI = G4UImanager::GetUIpointer();
	
    G4Navigator* navigator = new G4Navigator();
    navigator->SetWorldVolume(detector->mWorldPhysical);
    navigator->LocateGlobalPointAndSetup(G4ThreeVector(0., 0., 0.));
    
    G4TouchableHistory* history = navigator -> CreateTouchableHistory();
    
    G4cout << history -> GetHistoryDepth ()<< G4endl;
    G4cout << history->GetVolume(0)->GetName() << G4endl; 
    G4cout << history->GetVolume(1)->GetName() << G4endl; 
    G4cout << history->GetVolume(2)->GetName() << G4endl; 
    G4cout << history->GetVolume(3)->GetName() << G4endl; 
	// setting up light source:
	command.str("");
	command << "/control/execute " << ggunfilename;
	UI->ApplyCommand(command.str());
	
	double BeamRad = 0.5 * gBeamDiam;
	double cos_phi, sin_phi, cos_theta, sin_theta;
	double rho, posX,posY,posZ;
	
	// for moving around
	double phi;
	
	// preparing file for results.
	std::ofstream datafile;

	//change here...for mDOM with tgRefCone_angle tgConeMat tgHolderColor, else without. add hits_x by PMTnumber
	if (gHeader && gHittype == "collective") {
		//if (gDOM == 0){
			datafile.open(ghitsfilename.c_str(), std::ios::out|std::ios::app);
			datafile << "gtheta\tgphi\tgwavelen\tBeamRad\tgDOM\tgPMT\tgRefCone_angle\tgConeMat\tgGlass\tgGel\tgHolderColor\tgHarness\tgRopeNumber\tgEnvironment\tgsimevents";
			datafile << "\thits_0\thits_1\thits_2\thits_3\thits_4\thits_5\thits_6\thits_7\thits_8\thits_9\thits_10\thits_11\thits_12\thits_13\thits_14\thits_15\thits_16\thits_17\thits_18\thits_19\thits_20\thits_21\thits_22\thits_23\thits_total" << G4endl;	
			datafile.close();
		//}
		/*
		else if (gDOM == 1){
			datafile.open(ghitsfilename.c_str(), std::ios::out|std::ios::app);
			datafile << "gtheta\tgphi\tgwavelen\tBeamRad\tgGlass\tgGel\tgHarness\tgRopeNumber\tgEnvironment\tgsimevents";
			datafile << "\thits_0\thits_total" << G4endl;
			datafile.close();	
		}
		*/
	}	

    boost::property_tree::ptree lJsonTree;
    boost::property_tree::read_json("angles_and_wavelengths.dat", lJsonTree);
	//boost::property_tree::read_json("wavelengths.dat", lJsonTree);
	std::vector<G4double> lPhis;
	std::vector<G4double> lThetas;
	std::vector<G4double> lWavelengths;

	for (boost::property_tree::ptree::value_type &ridx : lJsonTree.get_child("phi")) lPhis.push_back(ridx.second.get_value<G4double>());
	for (boost::property_tree::ptree::value_type &ridx : lJsonTree.get_child("theta")) lThetas.push_back(ridx.second.get_value<G4double>());
	for (boost::property_tree::ptree::value_type &ridx : lJsonTree.get_child("wavelength")) lWavelengths.push_back(ridx.second.get_value<G4double>());
     double startingtime = clock() / CLOCKS_PER_SEC;

	for(std::vector<int>::size_type i = 0; i != lThetas.size(); i++) { 
		//G4cout << lWavelengths.at(i) << G4endl;
	//for(std::vector<int>::size_type i = 0; i != lWavelengths.size(); i++) { 
		//gtheta = lThetas.at(i);
		//gphi = lPhis.at(i);
		//gwavelen = lWavelengths.at(i);
		cos_phi = cos(gphi*deg);
		sin_phi = sin(gphi*deg);
		cos_theta = cos(gtheta*deg);
		sin_theta = sin(gtheta*deg);

		rho = gDistance*sin_theta;
		posX = rho*cos_phi;
		posY = rho*sin_phi;
		posZ = gDistance*cos_theta;
        
// 		command.str("");
// 		command << "/gps/particle opticalphoton";
// 		UI->ApplyCommand(command.str());

// 	command.str("");
// 	command << "/control/execute OMSim.gps";
// 	UI->ApplyCommand(command.str());
		
		command.str("");
		command << "/gps/pos/centre "<< posX <<" "<< posY <<" "<< posZ <<" mm";
		UI->ApplyCommand(command.str());
		
		command.str("");
		command << "/gps/energy " << (1239.84193 / gwavelen) << " eV ";
		UI->ApplyCommand(command.str());
		
		command.str("");
		command << "/gps/pos/radius " << BeamRad << " mm";
		UI->ApplyCommand(command.str());

		double x,y,z; // vector entries for plane positioning

		x = -sin_phi;	// d/dphi of positionVector (original divided by sin_theta, because length one not needed)
		y = cos_phi; 
		z = 0;
		command.str("");
		command << "/gps/pos/rot1 " << x <<" "<< y <<" "<< z;
		UI->ApplyCommand(command.str());
		command.str("");
		command << "/gps/ang/rot1 " << x <<" "<< y <<" "<< z;
		UI->ApplyCommand(command.str());

		x = -cos_phi * cos_theta;	// -d/dtheta of positionVector (divided by sin_theta, because length one not needed)
		y = -sin_phi * cos_theta;
		z = sin_theta;
		command.str("");
		command << "/gps/pos/rot2 " << x <<" "<< y <<" "<< z;
		UI->ApplyCommand(command.str());
		command.str("");
		command << "/gps/ang/rot2 " << x <<" "<< y <<" "<< z;
		UI->ApplyCommand(command.str());
		
		
		if (gDOM == 1){
			datafile.open(ghitsfilename.c_str(), std::ios::out|std::ios::app);
			datafile << std::fixed << std::setprecision(1) 
					<< gtheta 
					<< "\t" << gphi 
					<< "\t" << gwavelen 
					<< "\t" << BeamRad;	
			datafile 
					<< "\t" << gDOM
					<< "\t" << gPMT
					<< "\t" << gRefCone_angle 
					<< "\t" << gConeMat 
					<< "\t" << gGlass
					<< "\t" << gGel
					<< "\t" << gHolderColor
					<< "\t" << gHarness
					<< "\t" << gRopeNumber
					<< "\t" << gEnvironment
					<< "\t" << gsimevents;
			datafile.close();
		}
		else{
			datafile.open(ghitsfilename.c_str(), std::ios::out|std::ios::app);
			datafile << std::fixed << std::setprecision(1) 
					<< gtheta 
					<< "\t" << gphi 
					<< "\t" << gwavelen 
					<< "\t" << BeamRad;	
			datafile 
					<< "\t" << gDOM
					<< "\t" << gPMT
					<< "\t" << gGlass
					<< "\t" << gGel
					<< "\t" << gHarness
					<< "\t" << gRopeNumber
					<< "\t" << gEnvironment
					<< "\t" << gsimevents;
			datafile.close();
		}
		

	// starting run (analysis manager takes care of results)
		if (gsimevents > 0) {
			command.str("");
			command << "/run/beamOn " << gsimevents;
			UI->ApplyCommand(command.str());
			}		
	} 
	

// opening user interface prompt and visualization after simulation was run
	if (gInteractive){
		int argumc = 1;
		char* argumv[] = {"all", NULL};
		G4UIExecutive* UIEx = new G4UIExecutive(argumc, argumv);
		if (gVisual){
			UI->ApplyCommand("/control/execute ../aux/init_vis.mac");
			command.str("");
			command << "/run/beamOn " << gsimevents;
			UI->ApplyCommand(command.str());
		}
		UIEx->SessionStart();
		delete UIEx;
	}
double finishtime=clock() / CLOCKS_PER_SEC;
G4cout << "Computation time: " << finishtime-startingtime << " seconds." << G4endl;

	delete visManager;


	delete runManager;
	return 0;
}

/*
int main(int argc,char *argv[])
{
	struct arg_dbl  *worldsize	= arg_dbl0("wW", "world","<n>","\t\tradius of world sphere in m");
	struct arg_dbl  *diameter	= arg_dbl0("dD", "diam","<n>","\t\tbeam diameter in mm");
	struct arg_dbl  *distance	= arg_dbl0("rR", "dist, rad","<n>","\t\temitter distance from origin, in mm");
	struct arg_dbl  *theta		= arg_dbl0("tT", "theta","<n>","\t\ttheta (= zenith) in deg");
 	struct arg_dbl  *phi		= arg_dbl0("fF", "phi","<n>","\t\tphi (= azimuth) in deg");
    struct arg_dbl  *wavelen	= arg_dbl0("lL", "lambda","<n>","\t\twavelength of incoming light in nm");
	struct arg_int  *events		= arg_int0("nN", "numevents,nevents","<n>","\tnumber of photons emitted per angle");
	struct arg_file *gunfile	= arg_file0("gG","gun","<file.txt>","\t\tfile containing GPS parameters");
	struct arg_int  *pmt		= arg_int0("pP", "pmt,PMT","<n>","\t\tPMT type [12199S, etel, 12199e]");

	struct arg_int  *glass		= arg_int0("uU", "glass","<n>","\t\t\tglass type [VITROVEX, Chiba, Kopp, myVitroVex, myChiba, WOMQuartz, fusedSilica]");
	struct arg_int	*gel 		= arg_int0("jJ", "gel", "<n>", "\t\t\tgel type [Wacker, Chiba, IceCube, Wacker_company]");
	struct arg_dbl  *cone_ang   = arg_dbl0("aA", "cone_ang","<n>","\t\t\topening semi-angle of cone; (51 deg)");	
	struct arg_int	*conemat 	= arg_int0("kK", "conemat", "<n>", "\t\t\tcone material [V95, v98, aluminium, total98]");
	struct arg_int	*dom 		= arg_int0("mM", "om, dom", "<n>", "\t\t\tmodule type [Single PMT, MDOM, PDom, Lom16, custom]");
	struct arg_lit	*CADimport 	= arg_lit0("cC", "cad", "\t\t\tactivates CAD import for supported modules (LOM16)");
	struct arg_lit	*PlaceHarness 	= arg_lit0("sS", "harness", "\t\t\tactivates harness [OFF, ON]");
	
	
	struct arg_int  *environment= arg_int0("eE", "environment","<n>","\t\tmedium in which the setup is emmersed [AIR, ice, spice]");
	struct arg_file *outputfile	= arg_file0("oO","output","<file.txt>","\t\tfilename for hits data");
	struct arg_int  *hittype	= arg_int0("hH", "hits","<n>","\t\thit collection [individual, COLLECTIVE]");
	struct arg_lit	*interactive= arg_lit0("iI","interact","\t\topens user interface after run");
	struct arg_lit	*visual		= arg_lit0("vV","visual","\t\tshows visualization of module after run (also calls interactive)");
	struct arg_lit	*nohead		= arg_lit0("qQ","nh, nohead","\t\tno header in outputfile");
	struct arg_lit	*help		= arg_lit0(NULL,"help","\t\tprint this help and exit");
	struct arg_end  *end		= arg_end(20);
	
	void* argtable[] = {worldsize,
						diameter,
						distance,
						theta,
						phi,
						wavelen,
						events,
						gunfile,
						pmt,
						
						glass,
						gel,
						cone_ang,
						conemat,
						dom,
						CADimport,
						PlaceHarness,
						
						environment,
						outputfile,
						hittype,
						interactive,
						visual,
						nohead,
						help, end};
						
	const char* progname = "OMSim";
	int nerrors;
	int exitcode=0;

	// verify the argtable[] entries were allocated sucessfully
	if (arg_nullcheck(argtable) != 0) {
		// NULL entries were detected, some allocations must have failed 
		printf("%s: insufficient memory\n",progname);
		exitcode=1;
		arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
		return exitcode;
	}

	// set any command line default values prior to parsing
	worldsize->dval[0] = 3.0;	// world diameter in meters
	//diameter->dval[0] = 420.0;	// 400 mm # for 14" sphere, 480 mm # for 17" sphere 
	diameter->dval[0] = 560.0;	// LOM18 has 270*2 = 540 mm pole to pole length + 10mm safety margin
	
	//distance->dval[0] = 0.5 * 356 + 27.5 + 2; // here value for mDOM scan with 2*mm safety margin
	//distance->dval[0] = 270 + 20; // value for LOM18 glass with 20 mm saftety margin
	distance->dval[0] = 2000; // close to world border for harness simulations...no scattering in ice so it doesn't matter how far away it it
	


	theta->dval[0] = 0.0;
	phi->dval[0] = 0.0;
    wavelen->dval[0] = 400.0;	// [nm]
	events->ival[0] = 0;
	gunfile->filename[0] = "OMSim.gps";
	pmt->ival[0] = 0;			// use new R12199 version as default
	
	glass->ival[0] = 0;	// use VITROVEX as default
	gel->ival[0] = 1;	// use Chiba
	cone_ang->dval[0] = 51.0; // [degrees]	
	conemat->ival[0] = 0;	// use Alemco V95 as default
	dom->ival[0] = 1;	// use mDOM as default
	
	environment->ival[0] = 0;	// use air as default
	outputfile->filename[0] = "mdom_testoutput.txt";
	hittype->ival[0] = 1;		// store information on collective hits as default

	// Parse the command line as defined by argtable[] 
    nerrors = arg_parse(argc,argv,argtable);

    // special case: '--help' takes precedence over error reporting 
    if (help->count > 0)
	{
        printf("\nGEANT4 simulation of the mDOM: angular acceptance scan\n");
        printf("\nUsage: %s", progname);
        arg_print_syntax(stdout,argtable,"\n");
        arg_print_glossary(stdout,argtable,"  %-25s %s\n");
        printf("\n");
        exitcode=0;
        goto hell;
	}

    // If the parser returned any errors then display them and exit 
    if (nerrors > 0)
	{
        // Display the error details contained in the arg_end struct.
        arg_print_errors(stdout,end,progname);
        printf("Try '%s --help' for more information.\n",progname);
        exitcode=1;
        goto hell;
	}

    // special case: uname with no command line options induces brief help 
    if (argc==1)
	{
        printf("Try '%s --help' for more information.\n",progname);
        exitcode=0;
        goto hell;
	}

//	assign command-line arguments to variables:
	gworldsize = worldsize->dval[0];
	gBeamDiam = diameter->dval[0];
	gDistance = distance->dval[0];
	gtheta 	= theta->dval[0];
	gphi 	= phi->dval[0];
	gwavelen = wavelen->dval[0];
	gsimevents = events->ival[0];
	ggunfilename = gunfile->filename[0];
	gPMT = pmt->ival[0];
	
	gGlass = glass->ival[0];
	gGel = gel->ival[0];
	gRefCone_angle = cone_ang->dval[0];	
	gConeMat = conemat->ival[0];
	gDOM = dom->ival[0];

	
	gEnvironment = environment->ival[0];
	ghitsfilename = outputfile->filename[0];
	if (hittype->ival[0]==0){
		gHittype = "individual";
	}
	if (hittype->ival[0]==1){
		gHittype = "collective";
	}
	if (interactive->count > 0) gInteractive = true; else gInteractive = false;
	if (CADimport->count > 0) gCADImport = true; else gCADImport = false;
	if (PlaceHarness->count > 0) gPlaceHarness = true; else gPlaceHarness = false;
	if (visual->count > 0) {
		gVisual = true;
		gInteractive = true;
	}
	else {
		gVisual = false;
	}
	if (nohead->count > 0) gHeader = false; else gHeader = true;
	
	//	check params for sanity
	OMSim();

hell:
    // deallocate each non-null entry in argtable[] 
	arg_freetable(argtable,sizeof(argtable)/sizeof(argtable[0]));
	//	return exitcode;

}
*/


int main(int argc, char *argv[]) {
    try {
        OMSimArgumentTable& lConfig = OMSimArgumentTable::getInstance();

        po::options_description desc("Allowed options");
		desc.add_options()
			("help,h", "produce help message")
			("world,w", po::value<double>(&lConfig.worldSize)->default_value(3.0), "radius of world sphere in m")
			("diam,d", po::value<double>(&lConfig.beamDiam)->default_value(560.0), "beam diameter in mm")
			("dist,r", po::value<double>(&lConfig.distance)->default_value(2000), "emitter distance from origin, in mm")
			("theta,t", po::value<double>(&lConfig.theta)->default_value(0.0), "theta (= zenith) in deg")
			("phi,f", po::value<double>(&lConfig.phi)->default_value(0.0), "phi (= azimuth) in deg")
			("wavelength,l", po::value<double>(&lConfig.waveLen)->default_value(400.0), "wavelength of incoming light in nm")
			("numevents,n", po::value<int>(&lConfig.simEvents)->default_value(0), "number of photons emitted per angle")
			("gunfile,g", po::value<std::string>(&lConfig.gunFilename)->default_value("OMSim.gps"), "file containing GPS parameters")
			("pmt,p", po::value<int>(&lConfig.pmt)->default_value(0), "PMT type [12199S, etel, 12199e]")
			("glass", po::value<int>(&lConfig.glass)->default_value(0), "glass type [VITROVEX, Chiba, Kopp, myVitroVex, myChiba, WOMQuartz, fusedSilica]")
			("gel", po::value<int>(&lConfig.gel)->default_value(1), "gel type [Wacker, Chiba, IceCube, Wacker_company]")
			("refConeAngle", po::value<double>(&lConfig.refConeAngle)->default_value(51.0), "opening semi-angle of cone; (51 deg)")
			("conemat", po::value<int>(&lConfig.coneMat)->default_value(0), "cone material [V95, v98, aluminium, total98]")
			("om", po::value<int>(&lConfig.dom)->default_value(1), "module type [Single PMT, MDOM, PDom, Lom16, custom]")
			("cad", po::bool_switch(&lConfig.CADImport), "activates CAD import for supported modules (LOM16)")
			("harness", po::bool_switch(&lConfig.placeHarness), "activates harness [OFF, ON]")
			("environment", po::value<int>(&lConfig.environment)->default_value(0), "medium in which the setup is emmersed [AIR, ice, spice]")
			("output", po::value<std::string>(&lConfig.hitsFilename)->default_value("mdom_testoutput.txt"), "filename for hits data")
			("hits", po::value<int>(&lConfig.hitType)->default_value(1), "hit collection [individual, COLLECTIVE]")
			("interact", po::bool_switch(&lConfig.interactive), "opens user interface after run")
			("visual,v", po::bool_switch(&lConfig.visual), "shows visualization of module after run (also calls interactive)")
			("nohead", po::bool_switch(&lConfig.noHeader), "no header in outputfile");
			
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << "\n";
            return 1;
        }

        OMSim();

    } catch(std::exception& e) {
        std::cerr << "error: " << e.what() << "\n";
        return 1;
    } catch(...) {
        std::cerr << "Exception of unknown type!\n";
    }

    return 0;
}
