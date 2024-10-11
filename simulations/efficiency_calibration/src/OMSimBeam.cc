#include "OMSimBeam.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>


Beam::Beam(G4double pBeamRadius, G4double pBeamDistance) : m_beamRadius(pBeamRadius), m_beamDistance(pBeamDistance), m_zCorrection(nullptr)
{

}


void Beam::setWavelength(double p_wavelength){
    m_wavelength = p_wavelength;
}



void Beam::configureXYZScan_NKTLaser()
{
    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    ui.applyCommand("/event/verbose 0");
    ui.applyCommand("/control/verbose 0");
    ui.applyCommand("/run/verbose 0");
    ui.applyCommand("/gps/particle opticalphoton");
    ui.applyCommand("/gps/energy", 1239.84193 / 459, "eV");

    ui.applyCommand("/gps/pos/type Beam");
    ui.applyCommand("/gps/ang/type beam1d");
    ui.applyCommand("/gps/pos/radius 0 mm");
    ui.applyCommand("/gps/pos/sigma_r 0.14 mm");
    ui.applyCommand("/gps/ang/sigma_r 0 deg");
    

    ui.applyCommand("/gps/pos/rot1 0 1 0");
    ui.applyCommand("/gps/ang/rot1 0 1 0");
    ui.applyCommand("/gps/pos/rot2 -1 0 0");
    ui.applyCommand("/gps/ang/rot2 -1 0 0");

    ui.applyCommand("/gps/pos/centre 0 0 30 mm");

}

void Beam::runBeamNKTSetup(G4double p_x, G4double p_y)
{
    double z = m_zCorrection->Eval(std::sqrt(p_x*p_x+p_y*p_y));
    if (z<4.8) { z = 4.8;} //mDOM
    //if (z<6.9) { z = 6.9;} //LOM
    configureXYZScan_NKTLaser();

    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    ui.applyCommand("/gps/pos/centre", p_x, p_y, z, "mm");
    ui.runBeamOn();
}



/* For Erlangens QE measurement setup*/

void Beam::configureErlangenQESetup()
{
    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    ui.applyCommand("/event/verbose 0");
    ui.applyCommand("/control/verbose 0");
    ui.applyCommand("/run/verbose 0");
    ui.applyCommand("/gps/particle opticalphoton");
    ui.applyCommand("/gps/energy", 1239.84193 / m_wavelength, "eV");

    ui.applyCommand("/gps/pos/type Plane");
    ui.applyCommand("/gps/pos/shape Circle");
    ui.applyCommand("/gps/pos/radius 5 mm");
    ui.applyCommand("/gps/ang/type focused");
    ui.applyCommand("/gps/ang/focuspoint 0 0 279.5 mm");
    //ui.applyCommand("/gps/ang/focuspoint 0 0 480 mm"); //Rough Münster QE setup
    

    ui.applyCommand("/gps/pos/rot1 0 1 0");
    ui.applyCommand("/gps/ang/rot1 0 1 0");
    ui.applyCommand("/gps/pos/rot2 -1 0 0");
    ui.applyCommand("/gps/ang/rot2 -1 0 0");

    ui.applyCommand("/gps/pos/centre 0 0 535.5 mm");

}


void Beam::runErlangenQEBeam()
{
    configureErlangenQESetup();
    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    ui.runBeamOn();
}

/* For Münsters PicoQuant 3D scanner setup*/

void Beam::configureZCorrection_PicoQuant()
{
    m_zCorrection = new TGraph("../common/data/PMTs/measurement_matching_data/setup_stuff/mDOMPMT_PicoQuant_Scan_Zcorrection.txt"); //mDOM
    //m_zCorrection = new TGraph("../common/data/PMTs/measurement_matching_data/setup_stuff/4inch_used_z_compensation.txt"); //LOM
    m_zCorrection->SetName("Zcorrection");
}

void Beam::configureXYZScan_PicoQuantSetup()
{
    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    ui.applyCommand("/event/verbose 0");
    ui.applyCommand("/control/verbose 0");
    ui.applyCommand("/run/verbose 0");
    ui.applyCommand("/gps/particle opticalphoton");
    ui.applyCommand("/gps/energy", 1239.84193 / 459, "eV");

    ui.applyCommand("/gps/pos/type Beam");
    ui.applyCommand("/gps/ang/type beam1d");
    ui.applyCommand("/gps/pos/radius 0 mm");
    ui.applyCommand("/gps/pos/sigma_r 0.3822 mm");
    ui.applyCommand("/gps/ang/sigma_r 0.83 deg");
    

    ui.applyCommand("/gps/pos/rot1 0 1 0");
    ui.applyCommand("/gps/ang/rot1 0 1 0");
    ui.applyCommand("/gps/pos/rot2 -1 0 0");
    ui.applyCommand("/gps/ang/rot2 -1 0 0");

    ui.applyCommand("/gps/pos/centre 0 0 24.8 mm");
}

void Beam::runBeamPicoQuantSetup(G4double p_x, G4double p_y)
{
    double focalPoint = 9.5*mm;
    double distanceTipToCentre = 23.7245*mm; //mDOM + LOM...
    
    double z = focalPoint + distanceTipToCentre - m_zCorrection->Eval(std::sqrt(p_x*p_x+p_y*p_y))*mm;
    if (z<focalPoint) { z = focalPoint;}
    
    configureXYZScan_PicoQuantSetup();

    OMSimUIinterface &ui = OMSimUIinterface::getInstance();
    ui.applyCommand("/gps/pos/centre", p_x, p_y, z, "mm");
    ui.runBeamOn();
}
