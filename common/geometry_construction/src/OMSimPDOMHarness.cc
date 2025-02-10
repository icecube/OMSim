/** 
 *  @todo  - Add main data cable
 */
#include "OMSimPDOM.hh"
#include "OMSimPDOMHarness.hh"
#include "OMSimTools.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4LogicalSkinSurface.hh>
#include <G4Tubs.hh>
#include <G4Sphere.hh>
#include <G4Orb.hh>
#include <G4Polycone.hh>

DOMHarness::DOMHarness(DOM *pDOM): OMSimDetectorComponent(), m_opticalModule(pDOM)
{
    construction();
}

void DOMHarness::construction()
{
    //Non-CAD
    //mainDataCable();
    //buildHarnessSolid();

    // CAD-based
    CADHarnessWaistband();
    CADHarnessRopes();
    //PlaceCADString();
}

void DOMHarness::buildHarnessSolid()
{
    G4double harnessInner[] = {0, 0, 0, 0};
    G4double harnessRadii[] = {(0.5 * 365.76 - 8.3) * mm, 0.5 * 365.76 * mm, 0.5 * 365.76 * mm, (0.5 * 365.76 - 8.3) * mm};
    G4double harnessZplanes[] = {-31.75 * mm, -10 * mm, 10 * mm, 31.75 * mm};

    G4Orb *glassSphereSolid = new G4Orb("PDOM_GlassSphere solid", 0.5 * 13 * 25.4 * mm);
    G4Polycone *harnessAuxSolid = new G4Polycone("PDOM_HarnessAux solid", 0, 2 * CLHEP::pi, 4, harnessZplanes, harnessInner, harnessRadii);
    
    G4SubtractionSolid *harnessSolid = new G4SubtractionSolid("PDOM_Harness solid", harnessAuxSolid, glassSphereSolid);
    G4LogicalVolume *harnessLogical = new G4LogicalVolume(harnessSolid,
                                                           m_data->getMaterial("NoOptic_Stahl"),
                                                           "PDOM_Harness logical");
    
    appendComponent(harnessSolid, harnessLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "pDOM_Harness");
    
    new G4LogicalSkinSurface("PDOM_Harness_skin", harnessLogical, m_data->getOpticalSurface("Surf_StainlessSteelGround"));
    harnessLogical->SetVisAttributes(m_steelVis);
}

void DOMHarness::CADHarnessWaistband()
{
    G4ThreeVector lOriginWaistband(0 * mm, 0 * mm, 0 * mm);
    G4RotationMatrix lRotationWaistband;
    lRotationWaistband.rotateX(90 * deg);

    Tools::AppendCADComponent(this, 
    1.0, 
    lOriginWaistband, 
    lRotationWaistband,
    "DOM/Harness_WaistBand.obj",
    "CAD_WaistBand",
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}

void DOMHarness::CADHarnessRopes()
{
    G4ThreeVector lOriginRopes(0 * mm, 0 * mm, 0 * mm);
    G4RotationMatrix lRotationRopes;
    lRotationRopes.rotateX(90 * deg);

    Tools::AppendCADComponent(this, 
    1.0, 
    lOriginRopes, 
    lRotationRopes,
    "DOM/Harness_Ropes.obj",
    "CAD_Ropes",
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}


void DOMHarness::PlaceCADString()
{
    G4ThreeVector lOriginString(0 * mm, 0 * mm, 0 * mm);
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
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}



void DOMHarness::mainDataCable()
{
    const G4double dataCableRadius = m_data->getValueWithUnit(mDataKey, "jDataCableRadius"); // Radius of the main data cable (according to Prof. Kappes)
    const G4double dataCableLength = m_data->getValueWithUnit(mDataKey, "jDataCableLength"); // Length of main data cable

    G4Tubs *dataCable= new G4Tubs("MainDataCable_solid", 0, dataCableRadius, dataCableLength / 2., 0, 2 * CLHEP::pi);

    G4LogicalVolume *lDataCableLogical = new G4LogicalVolume(dataCable, m_data->getMaterial("NoOptic_Absorber"), "MainDataCable_logical");
    new G4LogicalSkinSurface("MainDataCable_skin", lDataCableLogical, m_data->getOpticalSurface("Surf_BlackDuctTapePolished"));
    lDataCableLogical->SetVisAttributes(m_absorberVis);

    G4ThreeVector lDataCablePosition = G4ThreeVector((m_totalWidth + dataCableRadius + 0.5 * cm) * sin(m_harnessRotAngle),
                                                     (m_totalWidth + dataCableRadius + 0.5 * cm) * cos(m_harnessRotAngle), 0);

    appendComponent(dataCable, lDataCableLogical, lDataCablePosition, G4RotationMatrix(), "mainDataCable");
}
