/**
 * @file OMSimWavePIDDetector.cc
 * @brief Implementation of detector construction for WavePID study.
 */
#include "OMSimWavePIDDetector.hh"
#include "OMSimPDOM.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimLogger.hh"

#include <G4Orb.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4MaterialPropertiesTable.hh>
#include <G4Material.hh>
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
 * @brief Constructs the world volume filled with IceCube ice.
 */
void OMSimWavePIDDetector::constructWorld()
{
    // Create spherical world volume (30m radius is sufficient)
    m_worldSolid = new G4Orb("World", 30 * m);

    // Get IceCube ice material
    G4Material* ice = G4Material::GetMaterial("IceCubeICE_SPICE");
    if (!ice) {
        log_critical("IceCubeICE_SPICE material not found!");
        return;
    }

    // Add refractive index if not present
    if (!ice->GetMaterialPropertiesTable()) {
        G4MaterialPropertiesTable* iceMPT = new G4MaterialPropertiesTable();
        const G4int nEntries = 2;
        G4double photonEnergies[nEntries] = {2.0 * eV, 3.5 * eV};
        G4double refractiveIndices[nEntries] = {1.33, 1.33};
        iceMPT->AddProperty("RINDEX", photonEnergies, refractiveIndices, nEntries);
        ice->SetMaterialPropertiesTable(iceMPT);
    }

    m_worldLogical = new G4LogicalVolume(m_worldSolid, ice, "World_log", 0, 0, 0);
    m_worldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.),
                                        m_worldLogical, "World_phys", 0, false, 0);

    // Set visualization: transparent blue for ice (same as original WavePID)
    G4VisAttributes* worldVis = new G4VisAttributes(G4Colour(0.0, 0.0, 1.0, 0.2));
    worldVis->SetVisibility(true);
    worldVis->SetForceSolid(true);
    m_worldLogical->SetVisAttributes(worldVis);

    log_debug("World volume constructed: 40m radius IceCube ice sphere");
}

/**
 * @brief Constructs and places the DOM detector at the origin.
 */
void OMSimWavePIDDetector::constructDetector()
{
    OMSimCommandArgsTable& args = OMSimCommandArgsTable::getInstance();

    bool placeHarness = args.get<bool>("place_harness");
    G4double zenith = args.get<G4double>("DOM_zenith");
    G4double azimuth = args.get<G4double>("DOM_azimuth");

    log_info("Constructing pDOM at origin (0, 0, 0) with zenith={} deg, azimuth={} deg",
             zenith, azimuth);

    // Create pDOM (using DOM class which is the pDOM/Gen1 DOM)
    DOM* opticalModule = new DOM(placeHarness);

    // Calculate rotation from zenith/azimuth
    G4RotationMatrix* rot = MakeRotFromZenithAzimuth(zenith, azimuth, true);

    // Place at origin
    G4ThreeVector position(0.0 * m, 0.0 * m, 0.0 * m);
    opticalModule->placeIt(position, *rot, m_worldLogical, "");

    // Configure sensitive volume for hit detection
    opticalModule->configureSensitiveVolume(this);

    log_info("pDOM placed at origin (0, 0, 0)");
}
