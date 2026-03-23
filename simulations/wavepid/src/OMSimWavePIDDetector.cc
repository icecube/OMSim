/**
 * @file OMSimWavePIDDetector.cc
 * @brief Implementation of detector construction for WavePID study.
 */
#include "OMSimWavePIDDetector.hh"
#include "OMSimPDOM.hh"
#include "OMSimMDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimLogger.hh"

#include <G4Orb.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4SystemOfUnits.hh>
#include <CLHEP/Units/SystemOfUnits.h>

/**
 * @brief Creates a rotation matrix from zenith and azimuth angles.
 */
static G4RotationMatrix* MakeRotFromZenithAzimuth(G4double zenith,
                                                   G4double azimuth,
                                                   bool angles_are_degrees = false)
{
    if (angles_are_degrees) {
        zenith *= CLHEP::deg;
        azimuth *= CLHEP::deg;
    }

    const G4double s = std::sin(zenith);
    const G4ThreeVector uz(s * std::cos(azimuth),
                           s * std::sin(azimuth),
                           std::cos(zenith));

    G4ThreeVector tmp(0, 0, 1);
    if (std::abs(uz.z()) > 0.9999) tmp = G4ThreeVector(0, 1, 0);
    G4ThreeVector ux = (tmp.cross(uz)).unit();
    G4ThreeVector uy = (uz.cross(ux)).unit();

    return new G4RotationMatrix(ux, uy, uz);
}

/**
 * @brief Constructs the world volume using the environment material from --environment flag.
 */
void OMSimWavePIDDetector::constructWorld()
{
    m_worldSolid = new G4Orb("World", 30 * m);
    m_worldLogical = new G4LogicalVolume(m_worldSolid, m_data->getMaterial("argWorld"), "World_log", 0, 0, 0);
    m_worldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.),
                                        m_worldLogical, "World_phys", 0, false, 0);

    G4VisAttributes* worldVis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.));
    m_worldLogical->SetVisAttributes(worldVis);

    log_debug("World volume constructed: 30m radius sphere");
}

/**
 * @brief Constructs and places the selected optical module at the origin.
 */
void OMSimWavePIDDetector::constructDetector()
{
    OMSimCommandArgsTable& args = OMSimCommandArgsTable::getInstance();

    bool placeHarness = args.get<bool>("place_harness");
    G4double zenith = args.get<G4double>("DOM_zenith");
    G4double azimuth = args.get<G4double>("DOM_azimuth");

    log_info("Constructing detector (type {}) at origin with zenith={} deg, azimuth={} deg",
             args.get<G4int>("detector_type"), zenith, azimuth);

    OMSimOpticalModule* opticalModule = nullptr;

    switch (args.get<G4int>("detector_type"))
    {
    case 0:
        log_critical("No custom detector implemented!");
        break;
    case 1:
    {
        log_info("Constructing single PMT");
        OMSimPMTConstruction* managerPMT = new OMSimPMTConstruction();
        managerPMT->selectPMT("argPMT");
        managerPMT->includeHAcoating();
        managerPMT->construction();
        managerPMT->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), m_worldLogical, "_0");
        OMSimHitManager::getInstance().setNumberOfPMTs(1, 0);
        managerPMT->configureSensitiveVolume(this, "/PMT/0");
        break;
    }
    case 2:
        opticalModule = new mDOM(placeHarness);
        break;
    case 3:
        opticalModule = new DOM(placeHarness);
        break;
    case 4:
        opticalModule = new LOM16(placeHarness);
        break;
    case 5:
        opticalModule = new LOM18(placeHarness);
        break;
    case 6:
        opticalModule = new DEGG(placeHarness);
        break;
    case 7:
        opticalModule = new DOM(placeHarness, true);
        break;
    }

    if (opticalModule)
    {
        G4RotationMatrix* rot = MakeRotFromZenithAzimuth(zenith, azimuth, true);
        G4ThreeVector position(0.0 * m, 0.0 * m, 0.0 * m);
        opticalModule->placeIt(position, *rot, m_worldLogical, "");
        opticalModule->configureSensitiveVolume(this);
        log_info("Optical module placed at origin (0, 0, 0)");
    }
}
