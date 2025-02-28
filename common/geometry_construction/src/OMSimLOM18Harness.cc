/** 
 *  @todo  - Verify main data cable
 */
#include "OMSimLOM18Harness.hh"
#include "OMSimLOM18.hh"
#include "OMSimTools.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4Sphere.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Tubs.hh>


LOM18Harness::LOM18Harness(LOM18 *pLOM18): OMSimDetectorComponent(), m_opticalModule(pLOM18)
{
    construction();
}

void LOM18Harness::construction()
{
    // Non-CAD
    //mainDataCable();

    // CAD-based
    CADHarnessWaistband();
    CADHarnessRopes();
    //CADHarnessPCA();
    //CADString();
}


void LOM18Harness::CADHarnessWaistband()
{
    G4ThreeVector lCADorigin(0 * mm, 0 * mm, 0 * mm);
    G4RotationMatrix lRotationCAD;
    //lRotationCAD.rotateX(90 * deg);
    lRotationCAD.rotateZ(45 * deg);

    Tools::AppendCADComponent(this, 
    1.0, 
    lCADorigin, 
    lRotationCAD,
    "LOM18/Harness_Waistband.obj",
    "CAD_Waistband",
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}

void LOM18Harness::CADHarnessPCA()
{
    G4ThreeVector lCADorigin(0 * mm, 0 * mm, -0.5 * mm);
    G4RotationMatrix lRotationCAD;
    lRotationCAD.rotateX(180 * deg);

    Tools::AppendCADComponent(this, 
    1.0, 
    lCADorigin, 
    lRotationCAD,
    "LOM18/Harness_PCA_simplified_250213.obj",
    "CAD_PCA",
    m_data->getMaterial("NoOptic_Absorber"), 
    m_absorberVis,
    m_data->getOpticalSurface("Surf_BlackDuctTapePolished"));   
}

void LOM18Harness::CADHarnessRopes()
{
    G4ThreeVector lCADorigin(0 * mm, 0 * mm, 0 * mm);
    G4RotationMatrix lRotationCAD;
    //lRotationCAD.rotateX(90 * deg);
    lRotationCAD.rotateZ(45 * deg);

    Tools::AppendCADComponent(this, 
    1.0, 
    lCADorigin, 
    lRotationCAD,
    "LOM18/Harness_Ropes.obj",
    "CAD_Ropes",
    m_data->getMaterial("NoOptic_Stahl"), 
    m_steelVis,
    m_data->getOpticalSurface("Surf_StainlessSteelGround"));
}

void LOM18Harness::CADString()
{
    G4ThreeVector lOriginString(0 * mm, 448.3 * mm, 0 * mm);
    G4RotationMatrix lRotationString;
    lRotationString.rotateX(90*deg);
    lRotationString.rotateZ(90*deg);

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

void LOM18Harness::mainDataCable()
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
