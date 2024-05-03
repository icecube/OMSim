/** 
 *  @todo   - 18/07/22 Solve collisions of CAD with PMTs
 *          - Clean up magic number variables and comment their meaning
 *          - Write documentation and parse current comments into Doxygen style
 */
#include "OMSimLOM16.hh"
#include "OMSimLogger.hh"
#include "CADMesh.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4Cons.hh>
#include <G4Ellipsoid.hh>
#include <G4IntersectionSolid.hh>
#include <G4Polycone.hh>
#include <G4EllipticalCone.hh>

LOM16::~LOM16()
{
    // delete mHarness;
}

LOM16::LOM16(InputDataManager *pData, G4bool pPlaceHarness)
{
    log_info("Constructing LOM16");
    mCheckOverlaps = OMSimCommandArgsTable::getInstance().get<bool>("check_overlaps");
    mData = pData;
    mPMTManager = new OMSimPMTConstruction(mData);
    mPMTManager->includeHAcoating();
    mPMTManager->selectPMT(mPMTModel);
    mPMTManager->construction();
    mPMToffset = mPMTManager->getDistancePMTCenterToTip();
    mMaxPMTRadius = mPMTManager->getMaxPMTRadius() + 2 * mm;

    mPlaceHarness = pPlaceHarness;
    if (mPlaceHarness)
    {
        // mHarness = new mDOMHarness(this, mData);
        // integrateDetectorComponent(mHarness, G4ThreeVector(0,0,0), G4RotationMatrix(), "");
        log_error("LOM16 harness not implemented yet");
    }

    construction();
}


void LOM16::construction()
{
    G4VSolid *lGlassSolid = pressureVessel(mGlassOutRad, "Glass");
    G4VSolid *lAirSolid = pressureVessel(mGlassInRad, "Gel"); // Fill entire vessel with gel as logical volume (not placed) for intersectionsolids with gelpads

    // Set positions and rotations of PMTs and gelpads
    setPMTAndGelpadPositions();

    // CAD internal components

    placeCADSupportStructure();
    lGlassSolid = substractToVolume(lGlassSolid, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Glass");
    lAirSolid = substractToVolume(lAirSolid, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Gel");
    
    // Logicals
    G4LogicalVolume *lGlassLogical = new G4LogicalVolume(lGlassSolid, mData->getMaterial("RiAbs_Glass_Vitrovex"), " Glass_log"); // Vessel
    G4LogicalVolume *lInnerVolumeLogical = new G4LogicalVolume(lAirSolid, mData->getMaterial("Ri_Air"), "Inner volume logical"); // Inner volume of vessel (mothervolume of all internal components)
    createGelpadLogicalVolumes(lAirSolid);                                                                                       // logicalvolumes of all gelpads saved globally to be placed below

    // Placements
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lInnerVolumeLogical, "Gel_physical", lGlassLogical, false, 0, mCheckOverlaps);

    placePMTs(lInnerVolumeLogical);
    placeGelpads(lInnerVolumeLogical);

    appendComponent(lGlassSolid, lGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel_" + std::to_string(mIndex));
    appendEquatorBand();

    // ---------------- visualisation attributes --------------------------------------------------------------------------------
    lGlassLogical->SetVisAttributes(mGlassVis);
    lInnerVolumeLogical->SetVisAttributes(mInvisibleVis); // Material defined as Ri_Air
    for (int i = 0; i <= mTotalNrPMTs - 1; i++)
    {
        mGelPad_logical[i]->SetVisAttributes(mGelVis); // mGelVis
    }
}

// ---------------- Component functions --------------------------------------------------------------------------------

G4UnionSolid *LOM16::pressureVessel(const G4double pOutRad, G4String pSuffix)
{
    G4Ellipsoid *lTopSolid = new G4Ellipsoid("SphereTop solid" + pSuffix, pOutRad, pOutRad, pOutRad, -5 * mm, pOutRad + 5 * mm);
    G4Ellipsoid *lBottomSolid = new G4Ellipsoid("SphereBottom solid" + pSuffix, pOutRad, pOutRad, pOutRad, -(pOutRad + 5 * mm), 5 * mm);

    G4double zCorners[] = {mCylHigh * 1.001, mCylHigh, 0, -mCylHigh, -mCylHigh * 1.001};
    G4double rCorners[] = {0, pOutRad, pOutRad + mCylHigh * sin(mCylinderAngle), pOutRad, 0};
    G4Polycone *lCylinderSolid = new G4Polycone("Cylinder solid" + pSuffix, 0, 2 * CLHEP::pi, 5, rCorners, zCorners);

    G4UnionSolid *lTempUnion = new G4UnionSolid("temp" + pSuffix, lCylinderSolid, lTopSolid, 0, G4ThreeVector(0, 0, mCylHigh));
    G4UnionSolid *lUnionSolid = new G4UnionSolid("OM body" + pSuffix, lTempUnion, lBottomSolid, 0, G4ThreeVector(0, 0, -mCylHigh));
    return lUnionSolid;
}

void LOM16::appendEquatorBand()
{

    G4Box *lCuttingBox = new G4Box("Cutter", mGlassOutRad + 10 * mm, mGlassOutRad + 10 * mm, mEqBandWidth / 2.);
    G4UnionSolid *lOuter = pressureVessel(mGlassOutRad + mEqBandThickness, "BandOuter");

    G4IntersectionSolid *lIntersectionSolid = new G4IntersectionSolid("BandThicknessBody", lCuttingBox, lOuter, 0, G4ThreeVector(0, 0, 0));
    G4SubtractionSolid *lEquatorBandSolid = new G4SubtractionSolid("Equatorband_solid", lIntersectionSolid,
                                                                   mComponents.at("PressureVessel_" + std::to_string(mIndex)).VSolid, 0, G4ThreeVector(0, 0, 0));

    G4LogicalVolume *lEquatorbandLogical = new G4LogicalVolume(lEquatorBandSolid, mData->getMaterial("NoOptic_Absorber"), "Equatorband_log");
    lEquatorbandLogical->SetVisAttributes(mAbsorberVis);
    appendComponent(lEquatorBandSolid, lEquatorbandLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTape");
}

// ---------------- Module specific funtions below --------------------------------------------------------------------------------

// Places internal components based on a CAD file. OBJ filetype, modified by python script, tesselated via mesh.cc / Documentation here: https://github.com/christopherpoole/CADMesh
// To do:
// Recreate CAD obj files (no SetScale & more variety to choose from)
// Each component has its own label in visualizer -> just one ... another function for penetrator, dummy main boards, ...
void LOM16::placeCADSupportStructure()
{

    G4String lFilePath = "../common/data/CADmeshes/LOM16/LOM_Internal_WithPen_WithCali.obj";
    G4String mssg = "Using the following CAD file for LOM16 internal structure: " + lFilePath;
    log_info(mssg);

    // load mesh
    auto lMesh = CADMesh::TessellatedMesh::FromOBJ(lFilePath);
    G4ThreeVector lCADoffset = G4ThreeVector(mInternalCAD_x, mInternalCAD_y, mInternalCAD_z);
    lMesh->SetOffset(lCADoffset);

    // lMesh->SetScale(10); //did a mistake...this LOM_Internal file needs cm -> mm -> x10
    // Place all of the meshes it can find in the file as solids individually.
    for (auto iSolid : lMesh->GetSolids())
    {
        G4LogicalVolume *lSupportStructureLogical = new G4LogicalVolume(iSolid, mData->getMaterial("NoOptic_Absorber"), "SupportStructureCAD_Logical");
        lSupportStructureLogical->SetVisAttributes(mAbsorberVis);
        appendComponent(iSolid, lSupportStructureLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SupportStructureCAD");
    }
}

// ToDo:
// sin(90 +- ...) -> cos(...)
void LOM16::setPMTAndGelpadPositions()
{
    const G4double lTotalLenght = mData->getValueWithUnit(mPMTModel, "jOuterShape.jTotalLenght");
    const G4double lCenterModuleToBottomPMTPolar = 70.9 * mm;
    const G4double lCenterModuleToBottomPMTEquatorial = 25.4 * mm;

    const G4double lZ_centertobottomPMT = lTotalLenght - mPMToffset; // Distance from bottom base of PMT to center of the Geant4 solid
    G4double lPMT_theta, lPMT_phi, lPMT_x, lPMT_y, PMT_z, lGelPad_x, lGelPad_y, lGelPad_z, lPMT_rho, lGelPad_rho;
    // calculate PMT and pads positions in the usual 4 for loop way
    for (int i = 0; i <= mTotalNrPMTs - 1; i++)
    {

        // upper polar
        if (i >= 0 && i <= mNrPolarPMTs - 1)
        {
            lPMT_theta = mThetaPolar;
            lPMT_phi = mPolEqPMTPhiPhase + i * 90.0 * deg;

            lPMT_rho = (147.7406 - 4.9) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = lCenterModuleToBottomPMTPolar + lZ_centertobottomPMT * sin(90 * deg - lPMT_theta);
            lGelPad_z = PMT_z + 2 * mGelPadDZ * sin(90 * deg - lPMT_theta);
        }

        // upper equatorial
        if (i >= mNrPolarPMTs && i <= mNrPolarPMTs + mNrEqPMTs - 1)
        {
            lPMT_theta = mThetaEquatorial;
            lPMT_phi = i * 90.0 * deg;

            lPMT_rho = (120.1640 - 5) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = lCenterModuleToBottomPMTEquatorial + lZ_centertobottomPMT * sin(90 * deg - lPMT_theta);
        }

        // lower equatorial
        if (i >= mNrPolarPMTs + mNrEqPMTs && i <= mNrPolarPMTs + mNrEqPMTs + mNrEqPMTs - 1)
        {
            lPMT_theta = 180 * deg - mThetaEquatorial; // 118*deg; // 118.5*deg;
            lPMT_phi = i * 90.0 * deg;

            lPMT_rho = (120.1640 - 5) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = (-lCenterModuleToBottomPMTEquatorial) - lZ_centertobottomPMT * sin(90 * deg - (180 * deg - lPMT_theta)); // rodo to cos ...
        }

        // lower polar
        if (i >= mTotalNrPMTs - mNrPolarPMTs && i <= mTotalNrPMTs - 1)
        {
            lPMT_theta = 180 * deg - mThetaPolar; // 144*deg; //152*deg;
            lPMT_phi = mPolEqPMTPhiPhase + ((i)*90.0) * deg;

            lPMT_rho = (147.7406 - 4.9) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = (-lCenterModuleToBottomPMTPolar) - lZ_centertobottomPMT * sin(90 * deg - (180 * deg - lPMT_theta));
            lGelPad_z = PMT_z - 2 * mGelPadDZ * sin(90 * deg - (180 * deg - lPMT_theta));
        }

        // PMTs
        lPMT_x = (lPMT_rho * mm) * sin(lPMT_theta) * cos(lPMT_phi);
        lPMT_y = (lPMT_rho * mm) * sin(lPMT_theta) * sin(lPMT_phi);

        // Gelpads
        lGelPad_x = lGelPad_rho * sin(lPMT_theta) * cos(lPMT_phi);
        lGelPad_y = lGelPad_rho * sin(lPMT_theta) * sin(lPMT_phi);

        // save positions in arrays
        mPMTPositions.push_back(G4ThreeVector(lPMT_x, lPMT_y, PMT_z));
        mGelpadPositions.push_back(G4ThreeVector(lGelPad_x, lGelPad_y, lGelPad_z));

        // save angles in arrays
        mPMT_theta.push_back(lPMT_theta);
        mPMT_phi.push_back(lPMT_phi);
    }
}

// Todo
// 4*mGelPadDZ -> 2 2*mGelPadDZ -> 1 ...  not needed if mGelPadDZ is long enough
// tra and transformers declaration uniformely.
// rename some stuff for clarity
void LOM16::createGelpadLogicalVolumes(G4VSolid *lGelSolid)
{
    // getting the PMT solid
    G4VSolid *lPMTsolid = mPMTManager->getPMTSolid();

    // Definition of helper volumes
    G4Cons *lGelPadBasicSolid;
    G4EllipticalCone *lGelPadBasicSolid_eq;
    G4IntersectionSolid *Cut_cone;
    G4LogicalVolume *GelPad_logical;
    G4SubtractionSolid *Cut_cone_final;

    // Definition of semiaxes in (elliptical section) cone for titled gel pads
    G4double dx = std::cos(mEqTiltAngle) / (2 * (1 + std::sin(mEqTiltAngle) * std::tan(mEqPadOpeningAngle))) * mMaxPMTRadius * 2; // semiaxis y at -ztop
    G4double ztop = 2 * mGelPadDZ;
    G4double dy = mMaxPMTRadius;                                // semiaxis x at -ztop
    G4double Dy = dy + 2 * ztop * std::tan(mEqPadOpeningAngle); // semiaxis x at +ztop
    G4double Dx = dx * Dy / dy;                                 //     Dx/dx=Dy/dy always ;  //semiaxis y at -ztop
    G4double xsemiaxis = (Dx - dx) / (2 * ztop);                // Best way it can be defined
    G4double ysemiaxis = (Dy - dy) / (2 * ztop);                // Best way it can be defined
    G4double zmax = (Dx + dx) / (2 * xsemiaxis);                // Best way it can be defined

    // For the placement of tilted  equatorial pads
    G4double dz = -dx * std::sin(mEqTiltAngle);
    G4double dY3 = mMaxPMTRadius - (ztop * std::sin(mEqTiltAngle) + dx * std::cos(mEqTiltAngle));

    // create logical volume for each gelpad
    for (int k = 0; k <= mTotalNrPMTs - 1; k++)
    {
        G4Transform3D *tra;
        G4Transform3D *tra2;

        mConverter.str("");
        mConverter2.str("");
        mConverter << "GelPad_" << k << "_solid";
        mConverter2 << "Gelpad_final" << k << "_logical";

        // polar gel pads
        if (k <= mNrPolarPMTs - 1 or k >= mTotalNrPMTs - mNrPolarPMTs)
        {
            lGelPadBasicSolid = new G4Cons("GelPadBasic", 0, mMaxPMTRadius, 0, mMaxPMTRadius + 4 * mGelPadDZ * tan(mPolPadOpeningAngle), 2 * mGelPadDZ, 0, 2 * CLHEP::pi);

            // rotation and position of gelpad
            G4RotationMatrix *rotation = new G4RotationMatrix();
            rotation->rotateY(mPMT_theta[k]);
            rotation->rotateZ(mPMT_phi[k]);

            tra = new G4Transform3D(*rotation, G4ThreeVector(mGelpadPositions[k]));
            G4Transform3D transformers = G4Transform3D(*rotation, G4ThreeVector(mPMTPositions[k]));

            // creating volumes ... basic cone, subtract PMT, logical volume of gelpad
            Cut_cone = new G4IntersectionSolid(mConverter.str(), lGelSolid, lGelPadBasicSolid, *tra);
            Cut_cone_final = new G4SubtractionSolid(mConverter.str(), Cut_cone, lPMTsolid, transformers);
            GelPad_logical = new G4LogicalVolume(Cut_cone_final, mData->getMaterial("RiAbs_Gel_Shin-Etsu"), mConverter2.str());
        };

        // upper equatorial
        if (k >= mNrPolarPMTs && k <= mNrPolarPMTs + mNrEqPMTs - 1)
        {
            // the tilted pad is a cone with elliptical section. Moreover, there is an union with a disc called "Tube". This union does not affect the geometry, but it is used to shift the center of the geant solid so it can be placed in the same way as the PMT. In addition to that, Cut_tube is used to do a better substraction of the residual part that lies below the photocatode.
            lGelPadBasicSolid_eq = new G4EllipticalCone("cone", xsemiaxis, ysemiaxis, zmax, ztop);

            // rotation and position of tilted gelpad
            G4RotationMatrix *rotation = new G4RotationMatrix();
            rotation->rotateY((180 * deg + mEqTiltAngle));
            tra2 = new G4Transform3D(*rotation, G4ThreeVector(-dY3, 0, ztop * std::cos(mEqTiltAngle) + dz));

            // place tilted cone
            G4Tubs *Tube = new G4Tubs("tube", 0, mMaxPMTRadius, 0.1 * mm, 0., 2 * CLHEP::pi);
            G4UnionSolid *Tilted_cone = new G4UnionSolid("union", Tube, lGelPadBasicSolid_eq, *tra2);

            // For subtraction of gelpad reaching below the photocathode
            rotation = new G4RotationMatrix();
            tra = new G4Transform3D(*rotation, G4ThreeVector(0, 0, -30 * mm)); // 30 -> thickness/width of subtraction box ... just has to be large eough to cut overlapping gelpads under photocathode

            // rotation and position of PMT
            G4RotationMatrix *rot = new G4RotationMatrix();
            rot->rotateY(mPMT_theta[k]);
            rot->rotateZ(mPMT_phi[k]);
            G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(mPMTPositions[k]));

            // creating volumes ... basic cone, tilted cone, subtract PMT, logical volume of gelpad
            G4Tubs *Cut_tube = new G4Tubs("tub", 0, 2 * mMaxPMTRadius, 30 * mm, 0, 2 * CLHEP::pi);
            G4SubtractionSolid *Tilted_final = new G4SubtractionSolid("cut", Tilted_cone, Cut_tube, *tra);
            Cut_cone = new G4IntersectionSolid(mConverter.str(), lGelSolid, Tilted_final, transformers);
            Cut_cone_final = new G4SubtractionSolid(mConverter.str(), Cut_cone, lPMTsolid, transformers);
            GelPad_logical = new G4LogicalVolume(Cut_cone_final, mData->getMaterial("RiAbs_Gel_Shin-Etsu"), mConverter2.str());
        };

        // lower equatorial
        if (k >= mNrPolarPMTs + mNrEqPMTs && k <= mTotalNrPMTs - mNrPolarPMTs - 1)
        {
            lGelPadBasicSolid_eq = new G4EllipticalCone("cone", xsemiaxis, ysemiaxis, zmax, ztop);

            // rotation and position of tilted gelpad
            G4RotationMatrix *rotation = new G4RotationMatrix();
            rotation->rotateY((180 * deg - mEqTiltAngle));
            tra2 = new G4Transform3D(*rotation, G4ThreeVector(dY3, 0, ztop * std::cos(mEqTiltAngle) + dz));

            // place tilted cone
            G4Tubs *Tube = new G4Tubs("tube", 0, mMaxPMTRadius, 0.1 * mm, 0., 2 * CLHEP::pi);
            G4UnionSolid *Tilted_cone = new G4UnionSolid("union", Tube, lGelPadBasicSolid_eq, *tra2);

            // For subtraction of gelpad reaching below the photocathode
            rotation = new G4RotationMatrix();
            tra = new G4Transform3D(*rotation, G4ThreeVector(0, 0, -30 * mm));

            // rotation and position of PMT
            G4RotationMatrix *rot = new G4RotationMatrix();
            rot->rotateY(mPMT_theta[k]);
            rot->rotateZ(mPMT_phi[k]);
            G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(mPMTPositions[k]));

            // creating volumes ... basic cone, tilted cone, subtract PMT, logical volume of gelpad
            G4Tubs *Cut_tube = new G4Tubs("tub", 0, 2 * mMaxPMTRadius, 30 * mm, 0, 2 * CLHEP::pi);
            G4SubtractionSolid *Tilted_final = new G4SubtractionSolid("cut", Tilted_cone, Cut_tube, *tra);
            Cut_cone = new G4IntersectionSolid(mConverter.str(), lGelSolid, Tilted_final, transformers);
            Cut_cone_final = new G4SubtractionSolid(mConverter.str(), Cut_cone, lPMTsolid, transformers);
            GelPad_logical = new G4LogicalVolume(Cut_cone_final, mData->getMaterial("RiAbs_Gel_Shin-Etsu"), mConverter2.str());
        };

        // save logicalvolume of gelpads in array
        mGelPad_logical.push_back(GelPad_logical); //
    }
}

void LOM16::placePMTs(G4LogicalVolume *lInnerVolumeLogical)
{
    for (int k = 0; k <= mTotalNrPMTs - 1; k++)
    {
        mConverter.str("");
        mConverter << "_" << k;

        G4RotationMatrix *lRot = new G4RotationMatrix();
        lRot->rotateY(mPMT_theta[k]);
        lRot->rotateZ(mPMT_phi[k]);
        G4Transform3D lTransformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

        mPMTManager->placeIt(lTransformers, lInnerVolumeLogical, mConverter.str());
    }
}

void LOM16::placeGelpads(G4LogicalVolume *lInnerVolumeLogical)
{
    for (int k = 0; k <= mTotalNrPMTs - 1; k++)
    {
        mConverter.str("");
        mConverter << "GelPad_" << k;

        G4RotationMatrix *lRot = new G4RotationMatrix();
        lRot->rotateY(mPMT_theta[k]);
        lRot->rotateZ(mPMT_phi[k]);
        G4Transform3D lTransformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), mGelPad_logical[k], mConverter.str(), lInnerVolumeLogical, false, 0, mCheckOverlaps);
    }
}
