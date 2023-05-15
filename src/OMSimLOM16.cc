/** @file OMSimLOM16.cc
 *  @brief Construction of LOM16.
 *
 *  @author Javi Vara & Markus Dittmer
 *  @date 18th July 2022
 *
 *  @version revision 1, Geant4 10.7
 *
 *  @todo  - 18/07/22 Solve collisions of CAD with PMTs
           -

    Changelog:
        18 July 2022    Martin Unland
                        -Internal CAD components are now placed in world. Gel and glass were substracted to avoid collisions.
                        -Some nomenclature changes
 */

#include "OMSimLOM16.hh"
#include "abcDetectorComponent.hh"
#include "CADMesh.hh" 
#include <dirent.h>
#include <stdexcept>
#include <cstdlib>

#include "G4Cons.hh"
#include "G4Ellipsoid.hh"
#include "G4EllipticalTube.hh"
#include "G4Sphere.hh"
#include "G4IntersectionSolid.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalVolume.hh"
#include "G4Orb.hh"
#include "G4Polycone.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include <G4UnitsTable.hh>
#include "G4VisAttributes.hh"
#include "G4Torus.hh"
#include "G4EllipticalCone.hh"

#include "OMSimLogger.hh"




LOM16::LOM16(OMSimInputData* pData, G4bool pPlaceHarness) {
    mData = pData;
    mPMTManager = new OMSimPMTConstruction(mData);
    mPMTManager->SimulateHACoating();
    mPMTManager->SelectPMT("pmt_Hamamatsu_4inch");
    mPMTManager->Construction();
    GetSharedData();

    mPlaceHarness = pPlaceHarness;
    if (mPlaceHarness) {
        //mHarness = new mDOMHarness(this, mData);
        //IntegrateDetectorComponent(mHarness, G4ThreeVector(0,0,0), G4RotationMatrix(), "");
        error("LOM16 harness not implemented yet");
    }

    Construction();
}

//Parameters from json files
void LOM16::GetSharedData() {
    //Shared Module Parameters
    mGlassOutRad = mData->GetValue(mDataKey, "jGlassOutRad"); // outer radius of galss cylinder (pressure vessel)
    mNrPolarPMTs = mData->GetValue(mDataKey, "jNrPolarPMTs");
    mNrEqPMTs = mData->GetValue(mDataKey, "jNrEqPMTs");
    mTotalNrPMTs = (mNrPolarPMTs + mNrEqPMTs) * 2;
    mGelPadDZ = mData->GetValue(mDataKey, "jGelPadDZ");// semiaxis (along pmt axis) of gelpads ... simply needs to be larger then 5mm (+ some more for tilted pads)...could be 100
}


//Placement function
void LOM16::Construction()
{   mComponents.clear();
    //Create pressure vessel and inner volume
    
    G4double lGlassThick = mData->GetValue(mDataKey, "jGlassThick");  // maximum Glass thickness
    G4double lGlassInRad = mGlassOutRad - lGlassThick;
    G4VSolid* lGlassSolid = PressureVessel(mGlassOutRad, "Glass");
    G4VSolid* lAirSolid = PressureVessel(lGlassInRad, "Gel"); // Fill entire vessel with gel as logical volume (not placed) for intersectionsolids with gelpads

    //Set positions and rotations of PMTs and gelpads
    SetPMTAndGelpadPositions();

    //CAD internal components
    if (false) {
        PlaceCADSupportStructure();
        lGlassSolid = SubstractToVolume(lGlassSolid, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Glass");
        lAirSolid = SubstractToVolume(lAirSolid, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Gel");
    }
    //Logicals
    G4LogicalVolume* lGlassLogical = new G4LogicalVolume(lGlassSolid, mData->GetMaterial("RiAbs_Glass_Vitrovex"), " Glass_log"); //Vessel
    G4LogicalVolume* lInnerVolumeLogical = new G4LogicalVolume(lAirSolid, mData->GetMaterial("Ri_Air"), "Inner volume logical"); //Inner volume of vessel (mothervolume of all internal components)  
    CreateGelpadLogicalVolumes(lAirSolid); //logicalvolumes of all gelpads saved globally to be placed below
    

    //Placements 
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lInnerVolumeLogical, "Gel_physical", lGlassLogical, false, 0, mCheckOverlaps);
    
    PlacePMTs(lInnerVolumeLogical);
    PlaceGelpads(lInnerVolumeLogical);

    AppendComponent(lGlassSolid, lGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel");
    AppendEquatorBand();
    
    // ---------------- visualisation attributes --------------------------------------------------------------------------------
    lGlassLogical->SetVisAttributes(mGlassVis);
    lInnerVolumeLogical->SetVisAttributes(mInvisibleVis); //Material defined as Ri_Air
    for (int i = 0; i <= mTotalNrPMTs - 1; i++) {
        mGelPad_logical[i]->SetVisAttributes(mGelVis); //mGelVis
    }
    
}


// ---------------- Component functions --------------------------------------------------------------------------------

G4UnionSolid* LOM16::PressureVessel(const G4double pOutRad, G4String pSuffix)
{
    G4double lCylHigh = mData->GetValue(mDataKey, "jCylHigh");         // height of cylindrical part of glass half-vessel
    G4double lCylinderAngle = mData->GetValue(mDataKey, "jCylinderAngle");  // Deviation angle of cylindrical part of the pressure vessel

    G4Ellipsoid* lTopSolid = new G4Ellipsoid("SphereTop solid" + pSuffix, pOutRad, pOutRad, pOutRad, -5 * mm, pOutRad + 5 * mm);
    G4Ellipsoid* lBottomSolid = new G4Ellipsoid("SphereBottom solid" + pSuffix, pOutRad, pOutRad, pOutRad, -(pOutRad + 5 * mm), 5 * mm);

    G4double zCorners[] = { lCylHigh * 1.001, lCylHigh, 0, -lCylHigh, -lCylHigh * 1.001 };
    G4double rCorners[] = { 0, pOutRad, pOutRad + lCylHigh * sin(lCylinderAngle), pOutRad, 0 };
    G4Polycone* lCylinderSolid = new G4Polycone("Cylinder solid" + pSuffix, 0, 2 * CLHEP::pi, 5, rCorners, zCorners);

    G4UnionSolid* lTempUnion = new G4UnionSolid("temp" + pSuffix, lCylinderSolid, lTopSolid, 0, G4ThreeVector(0, 0, lCylHigh));
    G4UnionSolid* lUnionSolid = new G4UnionSolid("OM body" + pSuffix, lTempUnion, lBottomSolid, 0, G4ThreeVector(0, 0, -lCylHigh));
    return lUnionSolid;
}


void LOM16::AppendEquatorBand()
{
    G4double lWidth = 45 * mm; //Total width (both halves)
    G4double lThickness = 1 * mm; //Thickness since its a 3D object

    G4Box* lCuttingBox = new G4Box("Cutter", mGlassOutRad + 10 * mm, mGlassOutRad + 10 * mm, lWidth / 2.);
    G4UnionSolid* lOuter = PressureVessel(mGlassOutRad + lThickness, "BandOuter");

    G4IntersectionSolid* lIntersectionSolid = new G4IntersectionSolid("BandThicknessBody", lCuttingBox, lOuter, 0, G4ThreeVector(0, 0, 0));
    G4SubtractionSolid* lEquatorBandSolid = new G4SubtractionSolid("Equatorband_solid", lIntersectionSolid,
                                                                   mComponents.at("PressureVessel").VSolid, 0, G4ThreeVector(0, 0, 0));

    G4LogicalVolume* lEquatorbandLogical = new G4LogicalVolume(lEquatorBandSolid, mData->GetMaterial("NoOptic_Absorber"), "Equatorband_log");
    lEquatorbandLogical->SetVisAttributes(mAbsorberVis);
    AppendComponent(lEquatorBandSolid, lEquatorbandLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTape");
    
}


// ---------------- Module specific funtions below --------------------------------------------------------------------------------



//Places internal components based on a CAD file. OBJ filetype, modified by python script, tesselated via mesh.cc / Documentation here: https://github.com/christopherpoole/CADMesh
//To do: 
//Recreate CAD obj files (no SetScale & more variety to choose from)
//Each component has its own label in visualizer -> just one ... another function for penetrator, dummy main boards, ...
void LOM16::PlaceCADSupportStructure()
{
    G4String lFilePath = mData->GetString(mDataKey, "jInternalCADFile");
    G4String mssg = "Using the following CAD file for LOM16 internal structure: " + lFilePath;
    info(mssg);

    //load mesh
    auto lMesh = CADMesh::TessellatedMesh::FromOBJ(lFilePath);
    G4ThreeVector lCADoffset = G4ThreeVector(mData->GetValue(mDataKey, "jInternalCAD_x"),
        mData->GetValue(mDataKey, "jInternalCAD_y"),
        mData->GetValue(mDataKey, "jInternalCAD_z")); //measured from CAD file since origin =!= Module origin
    lMesh->SetOffset(lCADoffset);
    // lMesh->SetScale(10); //did a mistake...this LOM_Internal file needs cm -> mm -> x10
     // Place all of the meshes it can find in the file as solids individually.
    for (auto iSolid : lMesh->GetSolids())
    {
        G4LogicalVolume* lSupportStructureLogical = new G4LogicalVolume(iSolid, mData->GetMaterial("NoOptic_Absorber"), "SupportStructureCAD_Logical");
        lSupportStructureLogical->SetVisAttributes(mAluVis);
        AppendComponent(iSolid, lSupportStructureLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SupportStructureCAD");
    }
}


//ToDo:
//sin(90 +- ...) -> cos(...)
void LOM16::SetPMTAndGelpadPositions()
{
    G4double lTotalLenght = mData->GetValue("pmt_Hamamatsu_4inch", "jOuterShape.jTotalLenght");
    G4double lOutRad = mData->GetValue("pmt_Hamamatsu_4inch", "jOuterShape.jOutRad");
    G4double lSpherePos_y = mData->GetValue("pmt_Hamamatsu_4inch", "jOuterShape.jSpherePos_y");
    G4double lEllipsePos_y = mData->GetValue("pmt_Hamamatsu_4inch", "jOuterShape.jEllipsePos_y");
    G4double lThetaPolar = mData->GetValue(mDataKey, "jThetaPolar"); //theta angle polar pmts
    G4double lThetaEquatorial = mData->GetValue(mDataKey, "jThetaEquatorial"); //theta angle equatorial pmts
    G4double lPolEqPMTPhiPhase = mData->GetValue(mDataKey, "jPolEqPMTPhiPhase"); //rotation of equatorial PMTs in respect to polar PMTs


    G4double Z_center_module_tobottomPMT_polar = 70.9 * mm; //measured z-offset from vessel origin from CAD file
    G4double Z_center_module_tobottomPMT_equatorial = 25.4 * mm; //measured z-offset from vessel origin from CAD file
    mGelPadDZ = 30 * mm;

    //calculate distances
    const G4double lFrontToEllipse_y = lOutRad + lSpherePos_y - lEllipsePos_y; //Center of PMT (PMT solid in Geant4) to tip
    const G4double lZ_centertobottomPMT = lTotalLenght - lFrontToEllipse_y; //Distance from bottom base of PMT to center of the Geant4 solid
    G4double lPMT_theta, lPMT_phi, lPMT_x, lPMT_y, PMT_z, lGelPad_x, lGelPad_y, lGelPad_z, lPMT_rho, lGelPad_rho;
    //calculate PMT and pads positions in the usual 4 for loop way 
    for (int i = 0; i <= mTotalNrPMTs - 1; i++) {

        //upper polar
        if (i >= 0 && i <= mNrPolarPMTs - 1) {
            lPMT_theta = lThetaPolar;
            lPMT_phi = lPolEqPMTPhiPhase + i * 90.0 * deg;

            lPMT_rho = (147.7406 - 4.9) * mm;  // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = Z_center_module_tobottomPMT_polar + lZ_centertobottomPMT * sin(90 * deg - lPMT_theta);
            lGelPad_z = PMT_z + 2 * mGelPadDZ * sin(90 * deg - lPMT_theta);
        }

        //upper equatorial
        if (i >= mNrPolarPMTs && i <= mNrPolarPMTs + mNrEqPMTs - 1) {
            lPMT_theta = lThetaEquatorial;
            lPMT_phi = i * 90.0 * deg;

            lPMT_rho = (120.1640 - 5) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = Z_center_module_tobottomPMT_equatorial + lZ_centertobottomPMT * sin(90 * deg - lPMT_theta);
        }

        //lower equatorial
        if (i >= mNrPolarPMTs + mNrEqPMTs && i <= mNrPolarPMTs + mNrEqPMTs + mNrEqPMTs - 1) {
            lPMT_theta = 180 * deg - lThetaEquatorial; //118*deg; // 118.5*deg;
            lPMT_phi = i * 90.0 * deg;

            lPMT_rho = (120.1640 - 5) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = (-Z_center_module_tobottomPMT_equatorial) - lZ_centertobottomPMT * sin(90 * deg - (180 * deg - lPMT_theta)); //rodo to cos ...
        }

        //lower polar
        if (i >= mTotalNrPMTs - mNrPolarPMTs && i <= mTotalNrPMTs - 1) {
            lPMT_theta = 180 * deg - lThetaPolar; //144*deg; //152*deg;
            lPMT_phi = lPolEqPMTPhiPhase + ((i) * 90.0) * deg;

            lPMT_rho = (147.7406 - 4.9) * mm;  // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            lGelPad_rho = lPMT_rho + 2 * mGelPadDZ;

            PMT_z = (-Z_center_module_tobottomPMT_polar) - lZ_centertobottomPMT * sin(90 * deg - (180 * deg - lPMT_theta));
            lGelPad_z = PMT_z - 2 * mGelPadDZ * sin(90 * deg - (180 * deg - lPMT_theta));
        }

        //PMTs
        lPMT_x = (lPMT_rho * mm) * sin(lPMT_theta) * cos(lPMT_phi);
        lPMT_y = (lPMT_rho * mm) * sin(lPMT_theta) * sin(lPMT_phi);

        //Gelpads
        lGelPad_x = lGelPad_rho * sin(lPMT_theta) * cos(lPMT_phi);
        lGelPad_y = lGelPad_rho * sin(lPMT_theta) * sin(lPMT_phi);

        //save positions in arrays
        mPMTPositions.push_back(G4ThreeVector(lPMT_x, lPMT_y, PMT_z));
        mGelpadPositions.push_back(G4ThreeVector(lGelPad_x, lGelPad_y, lGelPad_z));

        //save angles in arrays
        mPMT_theta.push_back(lPMT_theta);
        mPMT_phi.push_back(lPMT_phi);
    }
}

//Todo
//4*mGelPadDZ -> 2 2*mGelPadDZ -> 1 ...  not needed if mGelPadDZ is long enough
//tra and transformers declaration uniformely.
//rename some stuff for clarity
void LOM16::CreateGelpadLogicalVolumes(G4VSolid* lGelSolid)
{
    G4double lEqTiltAngle = mData->GetValue(mDataKey, "jEqTiltAngle"); //tilt angle of gel pad axis in respect to PMT axis
    G4double lPolPadOpeningAngle = mData->GetValue(mDataKey, "jPolPadOpeningAngle");
    G4double lEqPadOpeningAngle = mData->GetValue(mDataKey, "jEqPadOpeningAngle");
    G4double lMaxPMTRadius = mPMTManager->GetMaxPMTMaxRadius() + 2 * mm;

    //getting the PMT solid
    G4VSolid* lPMTsolid = mPMTManager->GetPMTSolid();

    //Definition of helper volumes
    G4Cons* lGelPadBasicSolid;
    G4EllipticalCone* lGelPadBasicSolid_eq;
    G4IntersectionSolid* Cut_cone;
    G4LogicalVolume* GelPad_logical;
    G4SubtractionSolid* Cut_cone_final;

    // Definition of semiaxes in (elliptical section) cone for titled gel pads
    G4double dx = std::cos(lEqTiltAngle) / (2 * (1 + std::sin(lEqTiltAngle) * std::tan(lEqPadOpeningAngle))) * lMaxPMTRadius * 2;  //semiaxis y at -ztop
    G4double ztop = 2 * mGelPadDZ;
    G4double dy = lMaxPMTRadius;  //semiaxis x at -ztop
    G4double Dy = dy + 2 * ztop * std::tan(lEqPadOpeningAngle);  //semiaxis x at +ztop
    G4double Dx = dx * Dy / dy;  //     Dx/dx=Dy/dy always ;  //semiaxis y at -ztop
    G4double xsemiaxis = (Dx - dx) / (2 * ztop);  // Best way it can be defined
    G4double ysemiaxis = (Dy - dy) / (2 * ztop); // Best way it can be defined
    G4double zmax = (Dx + dx) / (2 * xsemiaxis); // Best way it can be defined

    // For the placement of tilted  equatorial pads
    G4double dz = -dx * std::sin(lEqTiltAngle);
    G4double dY3 = lMaxPMTRadius - (ztop * std::sin(lEqTiltAngle) + dx * std::cos(lEqTiltAngle));

    //create logical volume for each gelpad    
    for (int k = 0; k <= mTotalNrPMTs - 1; k++) {
        G4Transform3D* tra;
        G4Transform3D* tra2;

        mConverter.str("");
        mConverter2.str("");
        mConverter << "GelPad_" << k << "_solid";
        mConverter2 << "Gelpad_final" << k << "_logical";

        // polar gel pads
        if (k <= mNrPolarPMTs - 1 or k >= mTotalNrPMTs - mNrPolarPMTs) {
            lGelPadBasicSolid = new G4Cons("GelPadBasic", 0, lMaxPMTRadius, 0, lMaxPMTRadius + 4 * mGelPadDZ * tan(lPolPadOpeningAngle), 2 * mGelPadDZ, 0, 2 * CLHEP::pi);

            //rotation and position of gelpad
            G4RotationMatrix* rotation = new G4RotationMatrix();
            rotation->rotateY(mPMT_theta[k]);
            rotation->rotateZ(mPMT_phi[k]);

            tra = new G4Transform3D(*rotation, G4ThreeVector(mGelpadPositions[k]));
            G4Transform3D transformers = G4Transform3D(*rotation, G4ThreeVector(mPMTPositions[k]));

            //creating volumes ... basic cone, subtract PMT, logical volume of gelpad
            Cut_cone = new G4IntersectionSolid(mConverter.str(), lGelSolid, lGelPadBasicSolid, *tra);
            Cut_cone_final = new G4SubtractionSolid(mConverter.str(), Cut_cone, lPMTsolid, transformers);
            GelPad_logical = new G4LogicalVolume(Cut_cone_final, mData->GetMaterial("RiAbs_Gel_Shin-Etsu"), mConverter2.str());
        };

        //upper equatorial
        if (k >= mNrPolarPMTs && k <= mNrPolarPMTs + mNrEqPMTs - 1) {
            //the tilted pad is a cone with elliptical section. Moreover, there is an union with a disc called "Tube". This union does not affect the geometry, but it is used to shift the center of the geant solid so it can be placed in the same way as the PMT. In addition to that, Cut_tube is used to do a better substraction of the residual part that lies below the photocatode. 
            lGelPadBasicSolid_eq = new G4EllipticalCone("cone", xsemiaxis, ysemiaxis, zmax, ztop);

            //rotation and position of tilted gelpad
            G4RotationMatrix* rotation = new G4RotationMatrix();
            rotation->rotateY((180 * deg + lEqTiltAngle));
            tra2 = new G4Transform3D(*rotation, G4ThreeVector(-dY3, 0, ztop * std::cos(lEqTiltAngle) + dz));

            //place tilted cone
            G4Tubs* Tube = new G4Tubs("tube", 0, lMaxPMTRadius, 0.1 * mm, 0., 2 * CLHEP::pi);
            G4UnionSolid* Tilted_cone = new G4UnionSolid("union", Tube, lGelPadBasicSolid_eq, *tra2);

            //For subtraction of gelpad reaching below the photocathode
            rotation = new G4RotationMatrix();
            tra = new G4Transform3D(*rotation, G4ThreeVector(0, 0, -30 * mm)); //30 -> thickness/width of subtraction box ... just has to be large eough to cut overlapping gelpads under photocathode

            //rotation and position of PMT
            G4RotationMatrix* rot = new G4RotationMatrix();
            rot->rotateY(mPMT_theta[k]);
            rot->rotateZ(mPMT_phi[k]);
            G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(mPMTPositions[k]));

            //creating volumes ... basic cone, tilted cone, subtract PMT, logical volume of gelpad
            G4Tubs* Cut_tube = new G4Tubs("tub", 0, 2 * lMaxPMTRadius, 30 * mm, 0, 2 * CLHEP::pi);
            G4SubtractionSolid* Tilted_final = new G4SubtractionSolid("cut", Tilted_cone, Cut_tube, *tra);
            Cut_cone = new G4IntersectionSolid(mConverter.str(), lGelSolid, Tilted_final, transformers);
            Cut_cone_final = new G4SubtractionSolid(mConverter.str(), Cut_cone, lPMTsolid, transformers);
            GelPad_logical = new G4LogicalVolume(Cut_cone_final, mData->GetMaterial("RiAbs_Gel_Shin-Etsu"), mConverter2.str());
        };

        //lower equatorial
        if (k >= mNrPolarPMTs + mNrEqPMTs && k <= mTotalNrPMTs - mNrPolarPMTs - 1) {
            lGelPadBasicSolid_eq = new G4EllipticalCone("cone", xsemiaxis, ysemiaxis, zmax, ztop);

            //rotation and position of tilted gelpad
            G4RotationMatrix* rotation = new G4RotationMatrix();
            rotation->rotateY((180 * deg - lEqTiltAngle));
            tra2 = new G4Transform3D(*rotation, G4ThreeVector(dY3, 0, ztop * std::cos(lEqTiltAngle) + dz));

            //place tilted cone
            G4Tubs* Tube = new G4Tubs("tube", 0, lMaxPMTRadius, 0.1 * mm, 0., 2 * CLHEP::pi);
            G4UnionSolid* Tilted_cone = new G4UnionSolid("union", Tube, lGelPadBasicSolid_eq, *tra2);

            //For subtraction of gelpad reaching below the photocathode
            rotation = new G4RotationMatrix();
            tra = new G4Transform3D(*rotation, G4ThreeVector(0, 0, -30 * mm));

            //rotation and position of PMT
            G4RotationMatrix* rot = new G4RotationMatrix();
            rot->rotateY(mPMT_theta[k]);
            rot->rotateZ(mPMT_phi[k]);
            G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(mPMTPositions[k]));

            //creating volumes ... basic cone, tilted cone, subtract PMT, logical volume of gelpad
            G4Tubs* Cut_tube = new G4Tubs("tub", 0, 2 * lMaxPMTRadius, 30 * mm, 0, 2 * CLHEP::pi);
            G4SubtractionSolid* Tilted_final = new G4SubtractionSolid("cut", Tilted_cone, Cut_tube, *tra);
            Cut_cone = new G4IntersectionSolid(mConverter.str(), lGelSolid, Tilted_final, transformers);
            Cut_cone_final = new G4SubtractionSolid(mConverter.str(), Cut_cone, lPMTsolid, transformers);
            GelPad_logical = new G4LogicalVolume(Cut_cone_final, mData->GetMaterial("RiAbs_Gel_Shin-Etsu"), mConverter2.str());
        };

        //save logicalvolume of gelpads in array
        mGelPad_logical.push_back(GelPad_logical); //
    }
}


void LOM16::PlacePMTs(G4LogicalVolume* lInnerVolumeLogical)
{
    for (int k = 0; k <= mTotalNrPMTs - 1; k++) {
        mConverter.str("");
        mConverter <<  "_" << k;

        G4RotationMatrix* lRot = new G4RotationMatrix();
        lRot->rotateY(mPMT_theta[k]);
        lRot->rotateZ(mPMT_phi[k]);
        G4Transform3D lTransformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

        mPMTManager->PlaceIt(lTransformers, lInnerVolumeLogical, mConverter.str());
    }

}

void LOM16::PlaceGelpads(G4LogicalVolume* lInnerVolumeLogical)
{
    for (int k = 0; k <= mTotalNrPMTs - 1; k++) {
        mConverter.str("");
        mConverter << "GelPad_" << k;

        G4RotationMatrix* lRot = new G4RotationMatrix();
        lRot->rotateY(mPMT_theta[k]);
        lRot->rotateZ(mPMT_phi[k]);
        G4Transform3D lTransformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), mGelPad_logical[k], mConverter.str(), lInnerVolumeLogical, false, 0, mCheckOverlaps);
    }
}
