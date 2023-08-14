/** @file OMSimLOM18.cc
 *  @brief Construction of LOM18.
 *
 *  @author Javi Vara & Markus Dittmer
 *
 *  @todo  - 18/07/22 Solve collisions of CAD with PMTs
 *         - Clean up needed variables
 *         - Move "magic numbers" to header, add comment with their meaning
 *         - Gel pads have to be tilted
 *         - Add Harness
 *         - Add realistic materials for internal components (100% absorber as of yet) 
 * @ingroup common
 */

#include "OMSimLOM18.hh"
#include "CADMesh.hh" 
#include "OMSimLogger.hh"

#include <G4Cons.hh>
#include <G4IntersectionSolid.hh>


LOM18::~LOM18()
{
    delete mPMTManager;
    //delete mHarness;
}

LOM18::LOM18(InputDataManager* pData, G4bool pPlaceHarness) {
    mData = pData;
    mPMTManager = new OMSimPMTConstruction(mData);
    mPMTManager->SimulateHACoating();
    mPMTManager->SelectPMT("pmt_Hamamatsu_4inch");
    mPMTManager->construction();
    mPMToffset = mPMTManager->GetDistancePMTCenterToPMTtip();
    mMaxPMTRadius = mPMTManager->GetMaxPMTMaxRadius() + 2 * mm;

    mPlaceHarness = pPlaceHarness;
    if (mPlaceHarness){
        //mHarness = new mDOMHarness(this, mData);
        //integrateDetectorComponent(mHarness, G4ThreeVector(0,0,0), G4RotationMatrix(), "");
        log_error("LOM18 harness not implemented yet");
    }
    construction();
}


/**
 * Placement function which appends all components into an abcDetectorComponent to be constructed in DetectorConstruction
 */
void LOM18::construction()
{   
    //Create pressure vessel and inner volume
    G4Polycone* lGlassSolid = createLOM18OuterSolid();
    G4Polycone* lInnerVolumeSolid = createLOM18InnerSolid();

    //Set positions and rotations of PMTs and gelpads
    setPMTPositions();
   
    //Logicals
    G4LogicalVolume* lInnerVolumeLogical = new G4LogicalVolume(lInnerVolumeSolid, mData->getMaterial("Ri_Air"), "Inner volume logical"); //Inner volume of vessel (mothervolume of all internal components)  
    G4LogicalVolume* lGlassLogical = new G4LogicalVolume(lGlassSolid, mData->getMaterial("RiAbs_Glass_Vitrovex")," Glass_log"); //Vessel
    createGelpadLogicalVolumes(lInnerVolumeSolid);


    //Placements 
    placePMTs(lInnerVolumeLogical);
    placeGelpads(lInnerVolumeLogical);

    //if (true) placeCADSupportStructure(lInnerVolumeLogical);
    //if (gCADImport) placeCADPenetrator(lInnerVolumeLogical);

    //Place InnerVolume (Mother of every other component) into GlassVolume (Mother of all)
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lInnerVolumeLogical, "Gel_physical", lGlassLogical, false, 0, mCheckOverlaps); //Innervolume (mother volume for all components)   


    // ------------------ Add outer shape solid to MultiUnion in case you need substraction -------------------------------------------
    //Each Component needs to be appended to be places in abcDetectorComponent. Everything is placed in the InnerVolume which is placed in the glass which is the mother volume. This is the reason why not everything is appended on its own
    appendComponent(lGlassSolid, lGlassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel");
    appendEquatorBand();
    // ---------------- visualisation attributes --------------------------------------------------------------------------------
    lGlassLogical->SetVisAttributes(mGlassVis);
    lInnerVolumeLogical->SetVisAttributes(mInvisibleVis); //Material defined as Ri_Air
    for (int i = 0; i <= mTotalNrPMTs-1; i++) {
        mGelPad_logical[i]->SetVisAttributes(mGelVis); //mGelVis
    }
    
}





// ---------------- Component functions --------------------------------------------------------------------------------


/**
 * Creation of outer module volume based on pressure vessel technical drawing... stolen drom doumeki as of yet
 * @return SolidVolume of inner module volume
 */
G4Polycone* LOM18::createLOM18OuterSolid()
{
	// parameters
	G4double r1, centerR1, centerZ1, thetaDelta1,
			 r2, centerR2, centerZ2, thetaDelta2,
			 r3, centerR3, centerZ3, thetaDelta3, r4;
	G4int numZPlanes1, numZPlanes2, numZPlanes3, numZPlanes;
	
	r1 = 154*mm;
	centerR1 = 0;
	centerZ1 = 116*mm;
	thetaDelta1 = 50.23*deg;

	r2 = 140*mm;
	centerR2 = 10.76*mm;
	centerZ2 = 124.96*mm;
	thetaDelta2 = 33*deg;

	r3 = 1300*mm;
	centerR3 = -1141.13*mm;
	centerZ3 = -11.95*mm;
	thetaDelta3 = 5.28*deg;

	r4 = 159*mm;
	numZPlanes1 = 5;
	numZPlanes2 = 5;
	numZPlanes3 = 5;

	numZPlanes = (numZPlanes1 + numZPlanes2 + numZPlanes3 - 2)*2 + 1;

	G4double zPlane[numZPlanes], rOuter[numZPlanes], rInner[numZPlanes];

	// fill value
	zPlane[0] = centerZ1 + r1;
	rOuter[0] = 1e-100*mm; // in order to avoid error (tentative)
	rInner[0] = 0;
	for(G4int i=1; i<numZPlanes1; i++) {
		G4double theta = thetaDelta1*i/(numZPlanes1 - 1);
		zPlane[i] = centerZ1 + r1*cos(theta);
		rOuter[i] = centerR1 + r1*sin(theta);
		rInner[i] = 0;
	}
	for(G4int i=1; i<numZPlanes2; i++) {
		G4double theta = thetaDelta1 + thetaDelta2*i/(numZPlanes2 - 1);
		zPlane[numZPlanes1 + i - 1] = centerZ2 + r2*cos(theta);
		rOuter[numZPlanes1 + i - 1] = centerR2 + r2*sin(theta);
		rInner[numZPlanes1 + i - 1] = 0;	
	}
	for(G4int i=1; i<numZPlanes3; i++) {
		G4double theta = thetaDelta1 + thetaDelta2 + thetaDelta3*i/(numZPlanes3 - 1);
		zPlane[numZPlanes1 + numZPlanes2 + i - 2] = centerZ3 + r3*cos(theta);
		rOuter[numZPlanes1 + numZPlanes2 + i - 2] = centerR3 + r3*sin(theta);
		rInner[numZPlanes1 + numZPlanes2 + i - 2] = 0;
	}
	zPlane[numZPlanes/2] = 0;
	rOuter[numZPlanes/2] = r4;
	rInner[numZPlanes/2] = 0;
	// opposite side
	for(G4int i=0; i<numZPlanes/2; i++) {
		zPlane[numZPlanes - i - 1] = -zPlane[i];
		rOuter[numZPlanes - i - 1] = rOuter[i];
		rInner[numZPlanes - i - 1] = 0;
	}
	
	// create solid
	G4Polycone *solid = new G4Polycone("solid", 0, 2*M_PI, numZPlanes,zPlane, rInner, rOuter);
	return solid;
}

/**
 * Creation of inner module volume based on pressure vessel technical drawing... stolen drom doumeki as of yet
 * @return SolidVolume of inner module volume
 */
G4Polycone* LOM18::createLOM18InnerSolid()
{
	// parameters
	G4double r1, centerR1, centerZ1, thetaDelta1,
			 r2, centerR2, centerZ2, thetaDelta2,
			 r3, centerR3, centerZ3, thetaDelta3, r4;
	G4int numZPlanes1, numZPlanes2, numZPlanes3, numZPlanes;
	
	r1 = 138*mm;
	centerR1 = 0;
	centerZ1 = 119.5*mm;
	thetaDelta1 = 52.03*deg;

	r2 = 125*mm;
	centerR2 = 10.249*mm;
	centerZ2 = 127.5*mm;
	thetaDelta2 = 32.48*deg;

	r3 = 1710*mm;
	centerR3 = -1567.44*mm;
	centerZ3 = -24.52*mm;
	thetaDelta3 = 4.0025*deg;

	r4 = 142.5*mm;
	numZPlanes1 = 5;
	numZPlanes2 = 5;
	numZPlanes3 = 5;

	numZPlanes = (numZPlanes1 + numZPlanes2 + numZPlanes3 - 2)*2 + 1;

	G4double zPlane[numZPlanes], rOuter[numZPlanes], rInner[numZPlanes];

	// fill value
	zPlane[0] = centerZ1 + r1;
	rOuter[0] = 1e-100*mm; // in order to avoid error (tentative)
	rInner[0] = 0;
	for(G4int i=1; i<numZPlanes1; i++) {
		G4double theta = thetaDelta1*i/(numZPlanes1 - 1);
		zPlane[i] = centerZ1 + r1*cos(theta);
		rOuter[i] = centerR1 + r1*sin(theta);
		rInner[i] = 0;
	}
	for(G4int i=1; i<numZPlanes2; i++) {
		G4double theta = thetaDelta1 + thetaDelta2*i/(numZPlanes2 - 1);
		zPlane[numZPlanes1 + i - 1] = centerZ2 + r2*cos(theta);
		rOuter[numZPlanes1 + i - 1] = centerR2 + r2*sin(theta);
		rInner[numZPlanes1 + i - 1] = 0;	
	}
	for(G4int i=1; i<numZPlanes3; i++) {
		G4double theta = thetaDelta1 + thetaDelta2 + thetaDelta3*i/(numZPlanes3 - 1);
		zPlane[numZPlanes1 + numZPlanes2 + i - 2] = centerZ3 + r3*cos(theta);
		rOuter[numZPlanes1 + numZPlanes2 + i - 2] = centerR3 + r3*sin(theta);
		rInner[numZPlanes1 + numZPlanes2 + i - 2] = 0;
	}
	zPlane[numZPlanes/2] = 0;
	rOuter[numZPlanes/2] = r4;
	rInner[numZPlanes/2] = 0;
	// opposite side
	for(G4int i=0; i<numZPlanes/2; i++) {
		zPlane[numZPlanes - i - 1] = -zPlane[i];
		rOuter[numZPlanes - i - 1] = rOuter[i];
		rInner[numZPlanes - i - 1] = 0;
	}
	
	// create solid
	G4Polycone *solid = new G4Polycone("solid", 0, 2*M_PI, numZPlanes,zPlane, rInner, rOuter);
	return solid;
}


void LOM18::appendEquatorBand()
{
    G4double tape_width = 45.0 * mm;
    G4double thicknessTape = 1.0*mm;

    G4int nsegments = 3;
    G4double rInner[3], rOuter[3], zPlane[3];
    zPlane[0] = -tape_width * 0.5;
    zPlane[1] = 0;
    zPlane[2] = tape_width * 0.5;
    rInner[0] = 0;
    rInner[1] = 0;
    rInner[2] = 0;
    rOuter[0] = mGlassEquatorWidth + thicknessTape;
    rOuter[1] = mGlassEquatorWidth + thicknessTape;
    rOuter[2] = mGlassEquatorWidth + thicknessTape;

    G4Polycone* polycone = new  G4Polycone("blacktape", 0, 2*M_PI, nsegments, zPlane, rInner, rOuter);
    G4VSolid* solidouter = createLOM18OuterSolid();
    G4SubtractionSolid* lEquatorbandSolid = new G4SubtractionSolid("BlackTapeLOM18", polycone, solidouter, 0, G4ThreeVector(0,0,0));
    G4LogicalVolume* lEquatorbandLogical = new G4LogicalVolume(lEquatorbandSolid, mData->getMaterial("NoOptic_Absorber"),"Equatorband_log"); 
    lEquatorbandLogical->SetVisAttributes(mAbsorberVis);
    
    appendComponent(lEquatorbandSolid, lEquatorbandLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTape");
}



/**
 * Imports inner components of module from CAD file and places them as an absorber (non optical compenents only since the tesselation is strong!)
 */
void LOM18::placeCADSupportStructure(G4LogicalVolume* lInnerVolumeLogical)
{
    //select file
    std::stringstream CADfile;
    CADfile.str(""); 
    CADfile << "EverythingButPMTsGelpadsVesselPenetrator.obj";
    //CADfile << "Internal.obj";
    
    G4cout <<  "using the following CAD file for support structure: "  << CADfile.str()  << G4endl;

    //load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/LOM18/" + CADfile.str() );

    //Offset
    G4ThreeVector CADoffset = G4ThreeVector(0, 0, 0); //measured from CAD file since origin =!= Module origin
    mesh->SetOffset(CADoffset);

    //rotate
    lRot = new G4RotationMatrix();
    //lRot->rotateY(mPMT_theta[k]);
    lRot->rotateZ(45*deg);
    
    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    { 
        mSupportStructureLogical  = new G4LogicalVolume( solid , mData->getMaterial("NoOptic_Absorber") , "logical" , 0, 0, 0); //should be Refl_AluminiumGround
        mSupportStructureLogical->SetVisAttributes(mAluVis);
        new G4PVPlacement( lRot , G4ThreeVector(0, 0, 0) , mSupportStructureLogical, "Support structure" , lInnerVolumeLogical, false, 0, mCheckOverlaps);
    }
}

/**
 * Imports inner Penetrator of module from CAD file and places them as an absorber (non optical compenents only since the tesselation is strong!)
 */
void LOM18::placeCADPenetrator(G4LogicalVolume* lInnerVolumeLogical)
{
    //select file
    std::stringstream CADfile;
    CADfile.str(""); 
    CADfile << "Penetrator.obj";
    //CADfile << "Internal.obj";
    
    G4cout <<  "using the following CAD file for penetrator: "  << CADfile.str()  << G4endl;

    //load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/LOM18/" + CADfile.str() );

    //Offset
    G4ThreeVector CADoffset = G4ThreeVector(0, 0, 0); //measured from CAD file since origin =!= Module origin
    mesh->SetOffset(CADoffset);

    //rotate
    lRot = new G4RotationMatrix();
    //lRot->rotateY(mPMT_theta[k]);
    lRot->rotateZ(45*deg);
    
    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    { 
        mSupportStructureLogical  = new G4LogicalVolume( solid , mData->getMaterial("NoOptic_Absorber") , "logical" , 0, 0, 0);
        mSupportStructureLogical->SetVisAttributes(mAluVis);
        new G4PVPlacement( lRot , G4ThreeVector(0, 0, 0) , mSupportStructureLogical, "Penetrator" , lInnerVolumeLogical, false, 0, mCheckOverlaps);
    }
}



/**
 * Saves postition and angles of PMTs into arrays to be used in other functions.
 */
void LOM18::setPMTPositions()
{
    //PMT offsets must be measured from CAD files
    G4double lZOffsetCenterPMTAxisOrigin = 180.64*mm; //measure the z-offset from the Equator to PMT tip
    G4double lZOffsetEquatorialPMTAxisOrigin = 87.76*mm; //measure the z-offset from the Equator to PMT tip

    G4double lXYZLengthCenterPMTAxisOriginToVesselInnerWall = 166.03*mm; //elongate the symetry axis of the PMT and measure from the central vessel z axis to inner vessel wall
    G4double lXYZLengthEquatorialPMTAxisOriginToVesselInnerWall = 160.35*mm; //elongate the symetry axis of the PMT and measure from the central vessel z axis to inner vessel wall

    //helper variables
    G4double lPMTTipZOffset;
    G4double lPMT_theta, lPMT_phi;
    G4double lPMT_x, lPMT_y, lPMT_z, lPMT_xyz=0;

    for (int i = 0; i <= mTotalNrPMTs-1; i++) {
        //upper polar
        if (i>=0 && i<=mNrPolarPMTs-1){ 
            //rotation
            lPMT_theta = 0*deg;
            lPMT_phi = 0*deg;

            //PMT position
            lPMTTipZOffset = mGlassPoleLength - mGlassThickPole - mGelThicknessFrontPolarPMT;
            lPMT_z = lPMTTipZOffset - mPMToffset;
            lPMT_xyz = 0*mm;
        }

        //upper center
        if (i>=mNrPolarPMTs && i<=mNrPolarPMTs + mNrCenterPMTs -1){ 
            //rotation
            lPMT_theta = mThetaCenter;
            lPMT_phi = (i - mNrPolarPMTs + 0.5) * 360. * deg / mNrCenterPMTs; //i*90.0*deg; for 4 

            //PMT position
            lPMTTipZOffset = lZOffsetCenterPMTAxisOrigin;
            lPMT_z = lPMTTipZOffset - mPMToffset*cos(90*deg-lPMT_theta);
            lPMT_xyz = lXYZLengthCenterPMTAxisOriginToVesselInnerWall  - mPMToffset - mGelThicknessFrontCenterPMT;            
        }
        
        //upper equatorial
        if (i>=mNrPolarPMTs + mNrCenterPMTs && i<=mNrPolarPMTs + mNrCenterPMTs + mNrEquatorialPMTs -1){ 
            //rotation
            lPMT_theta = mThetaEquatorial; 
            lPMT_phi = mEqPMTPhiPhase + (i - mNrPolarPMTs - mNrCenterPMTs + 0.5) * 360. * deg / mNrEquatorialPMTs; //i*90.0*deg; for 4 

            //PMT position
            lPMTTipZOffset = lZOffsetEquatorialPMTAxisOrigin;
            lPMT_z = lPMTTipZOffset - mPMToffset*cos(90*deg-lPMT_theta);
            lPMT_xyz = lXYZLengthEquatorialPMTAxisOriginToVesselInnerWall  - mPMToffset - mGelThicknessFrontEqPMT;
        }
        
        // Other module half

        //lower polar
        if (i>= mNrPMTsPerHalf && i<= mNrPMTsPerHalf + mNrPolarPMTs -1){ 
            //rotation
            lPMT_theta = 180*deg;
            lPMT_phi = 0*deg;

            //PMT position
            lPMTTipZOffset = mGlassPoleLength - mGlassThickPole - mGelThicknessFrontPolarPMT;
            lPMT_z = -lPMTTipZOffset + mPMToffset;
            lPMT_xyz = 0*mm;
        }

        //lower center
        if (i>= mNrPMTsPerHalf + mNrPolarPMTs && i<= mNrPMTsPerHalf + mNrPolarPMTs + mNrCenterPMTs -1){ 
            //rotation
            lPMT_theta = 180*deg -  mThetaCenter;
            lPMT_phi = (i - mNrPolarPMTs - mNrCenterPMTs + 0.5) * 360. * deg / mNrCenterPMTs; //i*90.0*deg; for 4 

            //PMT position
            lPMTTipZOffset = lZOffsetCenterPMTAxisOrigin;
            lPMT_z = -lPMTTipZOffset + mPMToffset*cos(180*deg-lPMT_theta);
            lPMT_xyz = lXYZLengthCenterPMTAxisOriginToVesselInnerWall  - mPMToffset - mGelThicknessFrontCenterPMT;
        }

        //lower equatorial
        if (i>=mTotalNrPMTs-mNrEquatorialPMTs && i<=mTotalNrPMTs-1){ 
            //rotation
            lPMT_theta = 180*deg + mThetaEquatorial;
            lPMT_phi = mEqPMTPhiPhase + (i - mNrPolarPMTs - mNrCenterPMTs + 0.5) * 360. * deg / mNrEquatorialPMTs; //i*90.0*deg; for 4 

            //PMT position
            lPMTTipZOffset = lZOffsetEquatorialPMTAxisOrigin;
            lPMT_z = -lPMTTipZOffset + mPMToffset*cos(180*deg-lPMT_theta);
            lPMT_xyz = lXYZLengthEquatorialPMTAxisOriginToVesselInnerWall  - mPMToffset - mGelThicknessFrontEqPMT;
        }

        //PMT positions
        lPMT_x = lPMT_xyz * sin(lPMT_theta) * cos(lPMT_phi);
        lPMT_y = lPMT_xyz * sin(lPMT_theta) * sin(lPMT_phi);
        mPMTPositions.push_back( G4ThreeVector(lPMT_x,lPMT_y,lPMT_z) );

        //PMT angles
        mPMT_theta.push_back(lPMT_theta);
        mPMT_phi.push_back(lPMT_phi);
    }
}    

/**
 * Creation of LogicalVolume of gel pads
 * @param lGelSolid Solid inner module volume. IntersectionSolid with PMT cones is filled with gel
 */
void LOM18::createGelpadLogicalVolumes(G4Polycone* lGelSolid) 
{
    //getting the PMT solid
    G4VSolid* lPMTsolid = mPMTManager->GetPMTSolid();

    //Definition of helper volumes
    G4Cons* lBasicConeSolid;
    G4IntersectionSolid* lPrelimaryGelpad;
    G4LogicalVolume* lGelPad_logical;
    G4SubtractionSolid* lGelpad;

    //Helper variables
    G4double lGelpadBuffer = 50*mm; //needs to overlap everywhere to be cut later. Without this, the gelpad won't reach the vessel inner wall at all points and have a flat surface. Value can be anything as long as it is long enough
    G4ThreeVector GelpadShift;
    G4Transform3D* tra;

    //create logical volume for each gelpad    
    for(int k=0 ; k<=mTotalNrPMTs-1 ; k++){
            mConv.str("");
            converter2.str("");
            mConv << "GelPad_" << k << "_solid";
            converter2 << "Gelpad_final" << k << "_logical";

            // polar gel pads
            if(k<=mNrPolarPMTs-1 or (mNrPMTsPerHalf<=k && k<=mNrPMTsPerHalf)){ 
                lBasicConeSolid = new G4Cons("GelPadBasic", 0,mMaxPMTRadius , 0, mMaxPMTRadius  + 2*lGelpadBuffer*tan(mPolarPadOpeningAngle), lGelpadBuffer, 0, 2*CLHEP::pi); //creates a very long cone which has two flat sides at an irrelevant position
                
                //rotation and position of gelpad
                lRot = new G4RotationMatrix();
                lRot->rotateY(mPMT_theta[k]);
                lRot->rotateZ(mPMT_phi[k]); 
                G4Transform3D transformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

                //To shift BasicCone onto photocathode edge
                GelpadShift = G4ThreeVector(0,0,lGelpadBuffer*cos(mPMT_theta[k]) );
                tra = new G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]+GelpadShift));

                //creating volumes ... basic cone, subtract PMT, logical volume of gelpad
                lPrelimaryGelpad = new G4IntersectionSolid(mConv.str(), lGelSolid, lBasicConeSolid, *tra); //Places beginning of cone at the edge of photocathode. Only intersection with inner volume counts.
                lGelpad = new G4SubtractionSolid(mConv.str(), lPrelimaryGelpad, lPMTsolid, transformers);
                
                lGelPad_logical = new G4LogicalVolume(lGelpad, mData->getMaterial("RiAbs_Gel_Shin-Etsu"), converter2.str());
            }
            // center gel pads
            else if( (mNrPolarPMTs<=k && k<=mNrPolarPMTs+mNrCenterPMTs-1) or (k>=mNrPMTsPerHalf+mNrPolarPMTs && k<= mNrPMTsPerHalf+mNrPolarPMTs+mNrCenterPMTs-1) ){ 
                lBasicConeSolid = new G4Cons("GelPadBasic", 0,mMaxPMTRadius , 0, mMaxPMTRadius  + 2*lGelpadBuffer*tan(mCenterPadOpeningAngle), lGelpadBuffer, 0, 2*CLHEP::pi); //creates a very long cone which has two flat sides at an irrelevant position
                
                //rotation and position of gelpad
                lRot = new G4RotationMatrix();
                lRot->rotateY(mPMT_theta[k]);
                lRot->rotateZ(mPMT_phi[k]);
                G4Transform3D transformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

                //To shift BasicCone onto photocathode edge
                GelpadShift = G4ThreeVector(lGelpadBuffer*sin(mPMT_theta[k]) * cos(mPMT_phi[k]),lGelpadBuffer*sin(mPMT_theta[k]) * sin(mPMT_phi[k]),lGelpadBuffer*cos(mPMT_theta[k]) );
                tra = new G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]+GelpadShift));

                //creating volumes ... basic cone, subtract PMT, logical volume of gelpad
                lPrelimaryGelpad = new G4IntersectionSolid(mConv.str(), lGelSolid, lBasicConeSolid, *tra); //Places beginning of cone at the edge of photocathode. Only intersection with inner volume counts.
                lGelpad = new G4SubtractionSolid(mConv.str(), lPrelimaryGelpad, lPMTsolid, transformers);
                lGelPad_logical = new G4LogicalVolume(lGelpad, mData->getMaterial("RiAbs_Gel_Shin-Etsu"), converter2.str());
            }
            // equatorial gel pads
            else if(k<=mNrPolarPMTs+mNrCenterPMTs+mNrEquatorialPMTs -1 or k>=mTotalNrPMTs-mNrEquatorialPMTs){ 
                lBasicConeSolid = new G4Cons("GelPadBasic", 0,mMaxPMTRadius , 0, mMaxPMTRadius  + 2*lGelpadBuffer*tan(mEqPadOpeningAngle), lGelpadBuffer, 0, 2*CLHEP::pi); //creates a very long cone which has two flat sides at an irrelevant position
                
                //rotation and position of gelpad
                lRot = new G4RotationMatrix();
                lRot->rotateY(mPMT_theta[k]);
                lRot->rotateZ(mPMT_phi[k]);
                G4Transform3D transformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

                //To shift BasicCone onto photocathode edge
                GelpadShift = G4ThreeVector(lGelpadBuffer*sin(mPMT_theta[k]) * cos(mPMT_phi[k]),lGelpadBuffer*sin(mPMT_theta[k]) * sin(mPMT_phi[k]),lGelpadBuffer*cos(mPMT_theta[k]) );
                tra = new G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]+GelpadShift));

                //creating volumes ... basic cone, subtract PMT, logical volume of gelpad
                lPrelimaryGelpad = new G4IntersectionSolid(mConv.str(), lGelSolid, lBasicConeSolid, *tra); //Places beginning of cone at the edge of photocathode. Only intersection with inner volume counts.
                lGelpad = new G4SubtractionSolid(mConv.str(), lPrelimaryGelpad, lPMTsolid, transformers);
                lGelPad_logical = new G4LogicalVolume(lGelpad, mData->getMaterial("RiAbs_Gel_Shin-Etsu"), converter2.str());
            }

            
    //save logicalvolume of gelpads in array
    mGelPad_logical.push_back( lGelPad_logical ); 
    }
}

/**
 * Placement of PMTs
 * @param lInnerVolumeLogical LogicalVolume of inner module volume in which the PMTs are placed.
 */
void LOM18::placePMTs(G4LogicalVolume* lInnerVolumeLogical)
{
    for(int k=0 ; k<=mTotalNrPMTs-1 ; k++){
        mConv.str("");
        mConv << "_"<< k;

        lRot = new G4RotationMatrix();
        lRot->rotateY(mPMT_theta[k]);
        lRot->rotateZ(mPMT_phi[k]);
        lTransformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

        mPMTManager->placeIt(lTransformers, lInnerVolumeLogical, mConv.str());
    }

}

/**
 * Placement of gel pads
 * @param lInnerVolumeLogical LogicalVolume of inner module volume in which the gel pads are placed.
 */
void LOM18::placeGelpads(G4LogicalVolume* lInnerVolumeLogical)
{
    for(int k=0 ; k<=mTotalNrPMTs-1 ; k++){
        mConv.str("");
        mConv << "GelPad_" << k;

        lRot = new G4RotationMatrix();
        lRot->rotateY(mPMT_theta[k]);
        lRot->rotateZ(mPMT_phi[k]);
        lTransformers = G4Transform3D(*lRot, G4ThreeVector(mPMTPositions[k]));

        new G4PVPlacement(0, G4ThreeVector(0,0,0), mGelPad_logical[k], mConv.str(), lInnerVolumeLogical, false, 0, mCheckOverlaps);
    }
}
