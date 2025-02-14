/** 
 *  @todo  - Add main data cable
 */
#include "OMSimDEGGHarness.hh"
#include "OMSimDEGG.hh"
#include "OMSimTools.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4Sphere.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Tubs.hh>


DEggHarness::DEggHarness(DEGG *pDEGG): OMSimDetectorComponent(), m_opticalModule(pDEGG)
{
    construction();
}


void DEggHarness::construction()
{
    // Non-CAD
    //mainDataCable();
    /*
    G4VSolid *solidDEGGHarness = buildHarnessSolid(m_rMin, m_rMax, m_sPhi, m_dPhi, m_sTheta, m_dTheta);
    G4LogicalVolume *logicalDEGGHarness = new G4LogicalVolume(solidDEGGHarness, m_data->getMaterial("NoOptic_Reflector"), "");
    appendComponent(solidDEGGHarness, logicalDEGGHarness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "dEGG_Harness");
    */

    // CAD-based
    CADHarnessWaistband();
    CADHarnessRopes();
    CADHarnessPCA();
    //PlaceCADString();
}


G4VSolid *DEggHarness::buildHarnessSolid(G4double p_rMin, G4double p_rMax, G4double p_sPhi, G4double p_dPhi, G4double p_sTheta, G4double p_dTheta)
{
    return new G4Sphere("solidDEGGHarness", p_rMin, p_rMax, p_sPhi, p_dPhi, p_sTheta, p_dTheta);
}

void DEggHarness::CADHarnessWaistband()
{
    G4ThreeVector lOriginWaistband(0 * mm, 0 * mm, 0 * mm);
    G4RotationMatrix lRotationWaistband;
    lRotationWaistband.rotateX(90 * deg);
    lRotationWaistband.rotateZ(-90 * deg);

    Tools::AppendCADComponent(this, 
    1.0, 
    lOriginWaistband, 
    lRotationWaistband,
    "DEGG/Harness_WaistBand.obj",
    "CAD_WaistBand",
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}

void DEggHarness::CADHarnessPCA()
{
    G4ThreeVector lOriginPCA(-427.6845 * mm, 310.6396 * mm, 152.89 * mm);
    G4RotationMatrix lRotationPCA;

    Tools::AppendCADComponent(this, 
    1.0, 
    lOriginPCA, 
    lRotationPCA,
    "DEGG/Harness_PCA_simplified.obj",
    "CAD_PCA",
    m_data->getMaterial("NoOptic_Absorber"), 
    m_absorberVis,
    m_data->getOpticalSurface("Surf_BlackDuctTapePolished"));   
}

void DEggHarness::CADHarnessRopes()
{
    G4ThreeVector lOriginRopes(-427.6845 * mm, 318.6396 * mm, 152.89 * mm);
    G4RotationMatrix lRotationRopes;

    Tools::AppendCADComponent(this, 
    1.0, 
    lOriginRopes, 
    lRotationRopes,
    "DEGG/Harness_Ropes_simplified.obj",
    "CAD_Ropes",
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}

void DEggHarness::PlaceCADString()
{
    G4ThreeVector lOriginString(0 * mm, 455 * mm, 0 * mm);
    G4RotationMatrix lRotationString;
    lRotationString.rotateX(90*deg);

    Tools::AppendCADComponent(this, 
    1.0, 
    lOriginString, 
    lRotationString,
    "Shared/Gen1String.obj",
    "CAD_String_1",
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));


    lRotationString.rotateX(180*deg);
    Tools::AppendCADComponent(this, 
    1.0, 
    lOriginString, 
    lRotationString,
    "Shared/Gen1String.obj",
    "CAD_String_2",
    m_data->getMaterial("NoOptic_Absorber"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}

void DEggHarness::mainDataCable()
{
    const G4double dataCableRadius = 50.0 * mm; // Radius of the main data cable (according to Prof. Kappes)
    const G4double dataCableLength = 4.0 * m;   // Length of main data cable

    G4Tubs *dataCable= new G4Tubs("MainDataCable_solid", 0, dataCableRadius, dataCableLength / 2., 0, 2 * CLHEP::pi);

    G4LogicalVolume *lDataCableLogical = new G4LogicalVolume(dataCable, m_data->getMaterial("NoOptic_Absorber"), "MainDataCable_logical");
    new G4LogicalSkinSurface("MainDataCable_skin", lDataCableLogical, m_data->getOpticalSurface("Surf_BlackDuctTapePolished"));
    lDataCableLogical->SetVisAttributes(m_absorberVis);

    G4ThreeVector lDataCablePosition = G4ThreeVector((m_totalWidth + dataCableRadius + 0.5 * cm) * sin(m_harnessRotAngle),
                                                     (m_totalWidth + dataCableRadius + 0.5 * cm) * cos(m_harnessRotAngle), 0);

    appendComponent(dataCable, lDataCableLogical, lDataCablePosition, G4RotationMatrix(), "mainDataCable");
}
