#include "OMSimDEGGHarness.hh"
#include "CADMesh.hh"
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

    // mainDataCable();

    if (true)
    {
        placeCADHarness();
        placeCADPenetrator();
    }
    else
    {
        log_warning("Penetrator and harness not implemented for Geant4 native version, use cad implementation if you need them");
        G4VSolid *solidDEGGHarness = buildHarnessSolid(m_rMin, m_rMax, m_sPhi, m_dPhi, m_sTheta, m_dTheta);
        G4LogicalVolume *logicalDEGGHarness = new G4LogicalVolume(solidDEGGHarness, m_data->getMaterial("NoOptic_Reflector"), "");
        appendComponent(solidDEGGHarness, logicalDEGGHarness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "dEGG_Harness");
    }
}

G4VSolid *DEggHarness::buildHarnessSolid(G4double p_rMin, G4double p_rMax, G4double p_sPhi, G4double p_dPhi, G4double p_sTheta, G4double p_dTheta)
{
    return new G4Sphere("solidDEGGHarness", p_rMin, p_rMax, p_sPhi, p_dPhi, p_sTheta, p_dTheta);
}

void DEggHarness::placeCADHarness()
{
    // select file
    std::stringstream fileCAD;
    fileCAD.str("");
    fileCAD << "Harness.obj";
    G4cout << "using the following CAD file for Harness: " << fileCAD.str() << G4endl;

    // load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/DEGG/" + fileCAD.str());
    // G4ThreeVector offsetCAD = G4ThreeVector(-427.6845*mm, 318.6396*mm, 152.89*mm); //measured from CAD file since origin =!= Module origin ... for no rotation
    G4ThreeVector offsetCAD = G4ThreeVector(318.6396 * mm, 427.6845 * mm, 152.89 * mm); // measured from CAD file since origin =!= Module origin ... for -90° z rotation

    G4RotationMatrix penetratorRotation = G4RotationMatrix();
    penetratorRotation.rotateZ(-90 * deg);

    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    {
        G4LogicalVolume *harnessLogical = new G4LogicalVolume(solid, m_data->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
        harnessLogical->SetVisAttributes(m_aluVis);
        appendComponent(solid, harnessLogical, offsetCAD, penetratorRotation, "CAD_Harness");
    }
}

void DEggHarness::placeCADPenetrator()
{
    // select file
    std::stringstream fileCAD;
    fileCAD.str("");
    fileCAD << "Penetrator.obj";
    G4cout << "using the following CAD file for Penetrator: " << fileCAD.str() << G4endl;

    // load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/DEGG/" + fileCAD.str());
    G4double xoffset = 110.211 * mm;
    // G4double zoffset = 34.39*mm;
    G4double zoffset = 77.817 * mm;

    G4double rotation = 4 * deg;

    G4ThreeVector offsetCAD = G4ThreeVector(-xoffset * cos(rotation), 0 * mm, zoffset * cos(rotation)); // measured from CAD file since origin =!= Module origin
    // G4ThreeVector offsetCAD = G4ThreeVector(0,0,0); //measured from CAD file since origin =!= Module origin
    G4RotationMatrix penetratorRotation = G4RotationMatrix();
    penetratorRotation.rotateX(rotation);
    penetratorRotation.rotateZ(90 * deg);

    mesh->SetScale(25.4); // did a mistake...this ONE file needs inch -> mm -> *2.54 * 10

    // mesh->SetOffset(offsetCAD); Don't set the offset here, it is done in appendComponent

    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    {
        G4LogicalVolume *logicalCADPenetrator = new G4LogicalVolume(solid, m_data->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
        logicalCADPenetrator->SetVisAttributes(m_aluVis);
        appendComponent(solid, logicalCADPenetrator, offsetCAD, penetratorRotation, "CAD_Penetrator");
    }
}


void DEggHarness::mainDataCable()
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
