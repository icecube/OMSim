/** @file OMSimMDOMHarness.cc
 *  @brief Construction of mDOM harness.
 *
 *  @date November 2021
 */

#include "OMSimDEGGHarness.hh"
#include "CADMesh.hh"

#include <G4Sphere.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Tubs.hh>


DEggHarness::DEggHarness(DEGG *pDEGG, InputDataManager *pData)
{
    mOM = pDEGG;
    mData = pData;
    construction();
}


/**
 * The construction of each part is called
 */
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
        G4cout << "Penetrator not implemented for Geant4 native version, use -c if you need them" << G4endl;
        G4cout << "Ropes not implemented for Geant4 native version, use -c if you need them" << G4endl;
        G4VSolid *EggHarness = buildHarnessSolid(mRmin, mRmax, mSphi, mDphi, mStheta, mDtheta);
        G4LogicalVolume *lEggHarness = new G4LogicalVolume(EggHarness, mData->getMaterial("NoOptic_Reflector"), "");
        appendComponent(EggHarness, lEggHarness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "dEGG_Harness");
    }
}

G4VSolid *DEggHarness::buildHarnessSolid(G4double rmin, G4double rmax, G4double sphi, G4double dphi, G4double stheta, G4double dtheta)
{
    G4Sphere *EggHarnessSolid1 = new G4Sphere("EggHarness", rmin, rmax, sphi, dphi, stheta, dtheta);
    G4VSolid *EggHarnessSolid = EggHarnessSolid1;
    return EggHarnessSolid;
}

void DEggHarness::placeCADHarness()
{
    // select file
    std::stringstream CADfile;
    CADfile.str("");
    CADfile << "Harness.obj";
    G4cout << "using the following CAD file for Harness: " << CADfile.str() << G4endl;

    // load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/DEGG/" + CADfile.str());
    // G4ThreeVector CADoffset = G4ThreeVector(-427.6845*mm, 318.6396*mm, 152.89*mm); //measured from CAD file since origin =!= Module origin ... for no rotation
    G4ThreeVector CADoffset = G4ThreeVector(318.6396 * mm, 427.6845 * mm, 152.89 * mm); // measured from CAD file since origin =!= Module origin ... for -90° z rotation

    G4RotationMatrix lPenetratorRotation = G4RotationMatrix();
    lPenetratorRotation.rotateZ(-90 * deg);

    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    {
        G4LogicalVolume *lCADHarness = new G4LogicalVolume(solid, mData->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
        lCADHarness->SetVisAttributes(mAluVis);
        appendComponent(solid, lCADHarness, CADoffset, lPenetratorRotation, "CAD_Harness");
    }
}

void DEggHarness::placeCADPenetrator()
{
    // select file
    std::stringstream CADfile;
    CADfile.str("");
    CADfile << "Penetrator.obj";
    G4cout << "using the following CAD file for Penetrator: " << CADfile.str() << G4endl;

    // load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/DEGG/" + CADfile.str());
    G4double xoffset = 110.211 * mm;
    // G4double zoffset = 34.39*mm;
    G4double zoffset = 77.817 * mm;

    G4double rotation = 4 * deg;

    G4ThreeVector CADoffset = G4ThreeVector(-xoffset * cos(rotation), 0 * mm, zoffset * cos(rotation)); // measured from CAD file since origin =!= Module origin
    // G4ThreeVector CADoffset = G4ThreeVector(0,0,0); //measured from CAD file since origin =!= Module origin
    G4RotationMatrix lPenetratorRotation = G4RotationMatrix();
    lPenetratorRotation.rotateX(rotation);
    lPenetratorRotation.rotateZ(90 * deg);

    mesh->SetScale(25.4); // did a mistake...this ONE file needs inch -> mm -> *2.54 * 10

    // mesh->SetOffset(CADoffset); Don't set the offset here, it is done in appendComponent

    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    {
        G4LogicalVolume *lCADPenetrator = new G4LogicalVolume(solid, mData->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
        lCADPenetrator->SetVisAttributes(mAluVis);
        appendComponent(solid, lCADPenetrator, CADoffset, lPenetratorRotation, "CAD_Penetrator");
    }
}

/**
 * build main data cable and append it to component vector
 */
void DEggHarness::mainDataCable()
{
    const G4double lDataCableRadius = mData->getValueWithUnit(mDataKey, "jDataCableRadius"); // Radius of the main data cable (according to Prof. Kappes)
    const G4double lDataCableLength = mData->getValueWithUnit(mDataKey, "jDataCableLength"); // Length of main data cable

    G4Tubs *lDataCableSolid = new G4Tubs("MainDataCable_solid", 0, lDataCableRadius, lDataCableLength / 2., 0, 2 * CLHEP::pi);

    G4LogicalVolume *lDataCableLogical = new G4LogicalVolume(lDataCableSolid, mData->getMaterial("NoOptic_Absorber"), "MainDataCable_logical");
    new G4LogicalSkinSurface("MainDataCable_skin", lDataCableLogical, mData->getOpticalSurface("Refl_BlackDuctTapePolished"));
    lDataCableLogical->SetVisAttributes(mAbsorberVis);

    G4ThreeVector lDataCablePosition = G4ThreeVector((mTotalWidth + lDataCableRadius + 0.5 * cm) * sin(mHarnessRotAngle),
                                                     (mTotalWidth + lDataCableRadius + 0.5 * cm) * cos(mHarnessRotAngle), 0);

    appendComponent(lDataCableSolid, lDataCableLogical, lDataCablePosition, G4RotationMatrix(), "mainDataCable");
}
