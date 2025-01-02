#include "OMSimYieldGPS.hh"
#include "OMSimLogger.hh"
#include "OMSimUIinterface.hh"
#include <G4SystemOfUnits.hh>
#include <G4Poisson.hh>

void OMSimYieldGPS::limitThetaEmission(G4double pThetaMin, G4double pThetaMax)
{
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    lUIinterface.applyCommand("/gps/ang/maxtheta ", pThetaMax / deg, " deg");
    lUIinterface.applyCommand("/gps/ang/mintheta ", pThetaMin / deg, " deg");
}
void OMSimYieldGPS::configureGammaEmitter(G4double pEnergy, G4String pVolumeName)
{
    log_debug("Configuring gamma emitter of energy {} in volume {}", pEnergy, pVolumeName);
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

    lUIinterface.applyCommand("/run/initialize");
    lUIinterface.applyCommand("/gps/particle gamma");
    G4ThreeVector lPosition = mEmitterVolume->m_placedPositions.at(0);
    lUIinterface.applyCommand("/gps/pos/centre ", lPosition.x() / m, " ", lPosition.y() / m, " ", lPosition.z() / m, " m");
    lUIinterface.applyCommand("/gps/energy ", pEnergy / keV, " keV");
    lUIinterface.applyCommand("/gps/pos/type Volume");
    lUIinterface.applyCommand("/gps/pos/shape Sphere");
    lUIinterface.applyCommand("/gps/pos/radius ", mProductionRadius, " mm");
    lUIinterface.applyCommand("/gps/ang/type iso");

    if (lArgs.get<bool>("scint_off"))
    {
        log_debug("Inactivating scintillation process");
        lUIinterface.applyCommand("/process/inactivate Scintillation");
    }

    if (lArgs.get<bool>("cherenkov_off"))
    {
        log_debug("Inactivating Cerenkov process");
        lUIinterface.applyCommand("/process/inactivate Cerenkov");
    }

    lUIinterface.applyCommand("/gps/pos/confine " + pVolumeName);
}



void OMSimYieldGPS::configureAm241Emitter(G4String pVolumeName)
{
    log_debug("Configuring Am241 emitter in volume {}", pVolumeName);
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

    lUIinterface.applyCommand("/run/initialize");
    lUIinterface.applyCommand("/gps/particle alpha");
    G4ThreeVector lPosition = mEmitterVolume->m_placedPositions.at(0);
    lUIinterface.applyCommand("/gps/pos/centre ", lPosition.x() / m, " ", lPosition.y() / m, " ", lPosition.z() / m, " m");
    lUIinterface.applyCommand("/gps/energy 5478.56 keV");
    lUIinterface.applyCommand("/gps/pos/type Volume");
    lUIinterface.applyCommand("/gps/pos/shape Sphere");
    lUIinterface.applyCommand("/gps/pos/radius ", mProductionRadius, " mm");
    lUIinterface.applyCommand("/gps/ang/type iso");
    lUIinterface.applyCommand("/gps/pos/confine " + pVolumeName);
}



void OMSimYieldGPS::configureAm241EmitterForActivity(G4String pVolumeName)
{
    log_debug("Configuring Am241 emitter in volume {}", pVolumeName);
    OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();

    lUIinterface.applyCommand("/run/initialize");
    lUIinterface.applyCommand("/gps/particle opticalphoton");
    lUIinterface.applyCommand("/gps/energy", 1239.84193 / 450, "eV");
    G4ThreeVector lPosition = mEmitterVolume->m_placedPositions.at(0);
    lUIinterface.applyCommand("/gps/pos/centre ", lPosition.x() / m, " ", lPosition.y() / m, " ", lPosition.z() / m, " m");
    lUIinterface.applyCommand("/gps/pos/type Volume");
    lUIinterface.applyCommand("/gps/pos/shape Sphere");
    lUIinterface.applyCommand("/gps/pos/radius ", mProductionRadius, " mm");
    lUIinterface.applyCommand("/gps/ang/type iso");
    lUIinterface.applyCommand("/gps/pos/confine " + pVolumeName);
}
