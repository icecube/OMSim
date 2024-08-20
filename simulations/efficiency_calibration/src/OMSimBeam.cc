#include "OMSimBeam.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>


Beam::Beam(G4double pBeamRadius, G4double pBeamDistance) : mBeamRadius(pBeamRadius), mBeamDistance(pBeamDistance), mZcorrection(nullptr)
{

}


void Beam::setWavelength(double pWavelength){
    mWavelength = pWavelength;
}



void Beam::configureXYZScan_NKTLaser()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.applyCommand("/event/verbose 0");
    lUIinterface.applyCommand("/control/verbose 0");
    lUIinterface.applyCommand("/run/verbose 0");
    lUIinterface.applyCommand("/gps/particle opticalphoton");
    lUIinterface.applyCommand("/gps/energy", 1239.84193 / 459, "eV");

    lUIinterface.applyCommand("/gps/pos/type Beam");
    lUIinterface.applyCommand("/gps/ang/type beam1d");
    lUIinterface.applyCommand("/gps/pos/radius 0 mm");
    lUIinterface.applyCommand("/gps/pos/sigma_r 0.14 mm");
    lUIinterface.applyCommand("/gps/ang/sigma_r 0 deg");
    

    lUIinterface.applyCommand("/gps/pos/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/ang/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/pos/rot2 -1 0 0");
    lUIinterface.applyCommand("/gps/ang/rot2 -1 0 0");

    lUIinterface.applyCommand("/gps/pos/centre 0 0 30 mm");

}


/* For Erlangens QE measurement setup*/

void Beam::configureErlangenQESetup()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.applyCommand("/event/verbose 0");
    lUIinterface.applyCommand("/control/verbose 0");
    lUIinterface.applyCommand("/run/verbose 0");
    lUIinterface.applyCommand("/gps/particle opticalphoton");
    lUIinterface.applyCommand("/gps/energy", 1239.84193 / mWavelength, "eV");

    lUIinterface.applyCommand("/gps/pos/type Plane");
    lUIinterface.applyCommand("/gps/pos/shape Circle");
    lUIinterface.applyCommand("/gps/pos/radius 5 mm");
    lUIinterface.applyCommand("/gps/ang/type focused");
    lUIinterface.applyCommand("/gps/ang/focuspoint 0 0 279.5 mm");
    

    lUIinterface.applyCommand("/gps/pos/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/ang/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/pos/rot2 -1 0 0");
    lUIinterface.applyCommand("/gps/ang/rot2 -1 0 0");

    lUIinterface.applyCommand("/gps/pos/centre 0 0 535.5 mm");

}


void Beam::runErlangenQEBeam()
{
    configureErlangenQESetup();
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.runBeamOn();
}

/* For Münsters PicoQuant 3D scanner setup*/

void Beam::configureZCorrection_PicoQuant()
{
    mZcorrection = new TGraph("../common/data/PMTs/measurement_matching_data/setup_stuff/mDOMPMT_PicoQuant_Scan_Zcorrection.txt");
    mZcorrection->SetName("Zcorrection");
}

void Beam::configureXYZScan_PicoQuantSetup()
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.applyCommand("/event/verbose 0");
    lUIinterface.applyCommand("/control/verbose 0");
    lUIinterface.applyCommand("/run/verbose 0");
    lUIinterface.applyCommand("/gps/particle opticalphoton");
    lUIinterface.applyCommand("/gps/energy", 1239.84193 / 459, "eV");

    lUIinterface.applyCommand("/gps/pos/type Beam");
    lUIinterface.applyCommand("/gps/ang/type beam1d");
    lUIinterface.applyCommand("/gps/pos/radius 0 mm");
    lUIinterface.applyCommand("/gps/pos/sigma_r 0.3822 mm");
    lUIinterface.applyCommand("/gps/ang/sigma_r 1.05 deg");
    

    lUIinterface.applyCommand("/gps/pos/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/ang/rot1 0 1 0");
    lUIinterface.applyCommand("/gps/pos/rot2 -1 0 0");
    lUIinterface.applyCommand("/gps/ang/rot2 -1 0 0");

    lUIinterface.applyCommand("/gps/pos/centre 0 0 24.8 mm");
}

void Beam::runBeamPicoQuantSetup(G4double pX, G4double pY)
{
    double lZ = mZcorrection->Eval(std::sqrt(pX*pX+pY*pY));
    if (lZ<4.8) { lZ = 4.8;}
    G4cout << std::sqrt(pX*pX+pY*pY) <<" " << lZ << G4endl;
    configureXYZScan_PicoQuantSetup();

    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.applyCommand("/gps/pos/centre", pX, pY, lZ, "mm");
    lUIinterface.runBeamOn();
}
