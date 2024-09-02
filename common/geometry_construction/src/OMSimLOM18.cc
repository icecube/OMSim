/** @file OMSimLOM18.cc
 *  @todo  - 18/07/22 Solve collisions of CAD with PMTs
 *         - Clean up needed variables
 *         - Move "magic numbers" to header, add comment with their meaning
 *         - Gel pads have to be tilted
 *         - Add Harness
 *         - Add realistic materials for internal components (100% absorber as of yet) 
 *         - Write documentation and parse current comments into Doxygen style
 */
#include "OMSimLOM18.hh"
#include "CADMesh.hh" 
#include "OMSimLogger.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4Cons.hh>
#include <G4IntersectionSolid.hh>


LOM18::~LOM18()
{
    //delete m_harness;
}

LOM18::LOM18(G4bool p_placeHarness): OMSimOpticalModule(new OMSimPMTConstruction()), m_placeHarness(p_placeHarness)
 {
    log_info("Constructing LOM18");
    m_managerPMT->includeHAcoating();
    m_managerPMT->selectPMT("pmt_Hamamatsu_4inch");
    m_managerPMT->construction();
    m_PMToffset = m_managerPMT->getDistancePMTCenterToTip();
    m_maxPMTRadius = m_managerPMT->getMaxPMTRadius() + 2 * mm;

    m_placeHarness = p_placeHarness;
    if (m_placeHarness){
        //m_harness = new mDOMHarness(this, m_data);
        //integrateDetectorComponent(m_harness, G4ThreeVector(0,0,0), G4RotationMatrix(), "");
        log_error("LOM18 harness not implemented yet");
    }
    construction();
}


/**
 * Placement function which appends all components into an OMSimDetectorComponent to be constructed in DetectorConstruction
 */
void LOM18::construction()
{   
    //Create pressure vessel and inner volume
    G4Polycone* glassSolid = createLOM18OuterSolid();
    G4Polycone* innerVolumeSolid = createLOM18InnerSolid();

    //Set positions and rotations of PMTs and gelpads
    setPMTPositions();
   
    //Logicals
    G4LogicalVolume* innerVolumeLogical = new G4LogicalVolume(innerVolumeSolid, m_data->getMaterial("Ri_Air"), "Inner volume logical"); //Inner volume of vessel (mothervolume of all internal components)  
    G4LogicalVolume* glassLogical = new G4LogicalVolume(glassSolid, m_data->getMaterial("RiAbs_Glass_Vitrovex")," Glass_log"); //Vessel
    createGelpadLogicalVolumes(innerVolumeSolid);


    //Placements 
    placePMTs(innerVolumeLogical);
    placeGelpads(innerVolumeLogical);

    //if (true) placeCADSupportStructure(innerVolumeLogical);
    //if (gCADImport) placeCADPenetrator(innerVolumeLogical);

    //Place InnerVolume (Mother of every other component) into GlassVolume (Mother of all)
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), innerVolumeLogical, "Gel_physical", glassLogical, false, 0, m_checkOverlaps); //Innervolume (mother volume for all components)   


    // ------------------ Add outer shape solid to MultiUnion in case you need substraction -------------------------------------------
    //Each Component needs to be appended to be places in OMSimDetectorComponent. Everything is placed in the InnerVolume which is placed in the glass which is the mother volume. This is the reason why not everything is appended on its own
    appendComponent(glassSolid, glassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel_" + std::to_string(m_index));
    appendEquatorBand();
    // ---------------- visualisation attributes --------------------------------------------------------------------------------
    glassLogical->SetVisAttributes(m_glassVis);
    innerVolumeLogical->SetVisAttributes(m_invisibleVis); //Material defined as Ri_Air
    for (int i = 0; i <= m_totalNumberPMTs-1; i++) {
        m_gelPadLogical[i]->SetVisAttributes(m_gelVis); //m_gelVis
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
    G4double tapeWidth = 45.0 * mm;
    G4double thicknessTape = 1.0*mm;

    G4int nsegments = 3;
    G4double rInner[3], rOuter[3], zPlane[3];
    zPlane[0] = -tapeWidth * 0.5;
    zPlane[1] = 0;
    zPlane[2] = tapeWidth * 0.5;
    rInner[0] = 0;
    rInner[1] = 0;
    rInner[2] = 0;
    rOuter[0] = m_GlassEquatorWidth + thicknessTape;
    rOuter[1] = m_GlassEquatorWidth + thicknessTape;
    rOuter[2] = m_GlassEquatorWidth + thicknessTape;

    G4Polycone* polycone = new  G4Polycone("blacktape", 0, 2*M_PI, nsegments, zPlane, rInner, rOuter);
    G4VSolid* solidouter = createLOM18OuterSolid();
    G4SubtractionSolid* equatorBandSolid = new G4SubtractionSolid("BlackTapeLOM18", polycone, solidouter, 0, G4ThreeVector(0,0,0));
    G4LogicalVolume* equatorBandLogical = new G4LogicalVolume(equatorBandSolid, m_data->getMaterial("NoOptic_Absorber"),"Equatorband_log"); 
    equatorBandLogical->SetVisAttributes(m_absorberVis);
    
    appendComponent(equatorBandSolid, equatorBandLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTape");
}



/**
 * Imports inner components of module from CAD file and places them as an absorber (non optical compenents only since the tesselation is strong!)
 */
void LOM18::placeCADSupportStructure(G4LogicalVolume* p_innerVolume)
{
    //select file
    std::stringstream CADfile;
    CADfile.str(""); 
    CADfile << "EverythingButPMTsGelpadsVesselPenetrator.obj";
    //CADfile << "Internal.obj";
    
    log_debug("Using the following CAD file for support structure: {}", CADfile.str());

    //load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/LOM18/" + CADfile.str() );

    //Offset
    G4ThreeVector CADoffset = G4ThreeVector(0, 0, 0); //measured from CAD file since origin =!= Module origin
    mesh->SetOffset(CADoffset);

    //rotate
    G4RotationMatrix* rot = new G4RotationMatrix();
    //rot->rotateY(m_thetaPMT[k]);
    rot->rotateZ(45*deg);
    
    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    { 
        G4LogicalVolume* supportStructureLogical  = new G4LogicalVolume( solid , m_data->getMaterial("NoOptic_Absorber") , "logical" , 0, 0, 0); //should be Surf_AluminiumGround
        supportStructureLogical->SetVisAttributes(m_aluVis);
        new G4PVPlacement( rot , G4ThreeVector(0, 0, 0) , supportStructureLogical, "Support structure" , p_innerVolume, false, 0, m_checkOverlaps);
    }
}

/**
 * Imports inner Penetrator of module from CAD file and places them as an absorber (non optical compenents only since the tesselation is strong!)
 */
void LOM18::placeCADPenetrator(G4LogicalVolume* innerVolumeLogical)
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
    G4RotationMatrix* rot = new G4RotationMatrix();
    //rot->rotateY(m_thetaPMT[k]);
    rot->rotateZ(45*deg);
    
    // Place all of the meshes it can find in the file as solids individually.
    for (auto solid : mesh->GetSolids())
    { 
        G4LogicalVolume* supportStructureLogical  = new G4LogicalVolume( solid , m_data->getMaterial("NoOptic_Absorber") , "logical" , 0, 0, 0);
        supportStructureLogical->SetVisAttributes(m_aluVis);
        new G4PVPlacement( rot , G4ThreeVector(0, 0, 0) , supportStructureLogical, "Penetrator" , innerVolumeLogical, false, 0, m_checkOverlaps);
    }
}



/**
 * Saves postition and angles of PMTs into arrays to be used in other functions.
 */
void LOM18::setPMTPositions()
{
    //PMT offsets must be measured from CAD files
    G4double zOffsetCenterPMTAxisOrigin = 180.64*mm; //measure the z-offset from the Equator to PMT tip
    G4double zOffsetEquatorialPMTAxisOrigin = 87.76*mm; //measure the z-offset from the Equator to PMT tip

    G4double lengthCenterPMTAxisOriginToVesselInnerWall = 166.03*mm; //elongate the symetry axis of the PMT and measure from the central vessel z axis to inner vessel wall
    G4double lengthEquatorialPMTAxisOriginToVesselInnerWall = 160.35*mm; //elongate the symetry axis of the PMT and measure from the central vessel z axis to inner vessel wall

    //helper variables
    G4double zOffsetTipPMT;
    G4double thetaPMT, phiPMT;
    G4double xPMT, yPMT, zPMT, xyzPMT=0;

    for (int i = 0; i <= m_totalNumberPMTs-1; i++) {
        //upper polar
        if (i>=0 && i<=m_numberPolarPMTs-1){ 
            //rotation
            thetaPMT = 0*deg;
            phiPMT = 0*deg;

            //PMT position
            zOffsetTipPMT = m_GlassPoleLength - m_GlassThickPole - m_GelThicknessFrontPolarPMT;
            zPMT = zOffsetTipPMT - m_PMToffset;
            xyzPMT = 0*mm;
        }

        //upper center
        if (i>=m_numberPolarPMTs && i<=m_numberPolarPMTs + m_NrCenterPMTs -1){ 
            //rotation
            thetaPMT = m_thetaCenter;
            phiPMT = (i - m_numberPolarPMTs + 0.5) * 360. * deg / m_NrCenterPMTs; //i*90.0*deg; for 4 

            //PMT position
            zOffsetTipPMT = zOffsetCenterPMTAxisOrigin;
            zPMT = zOffsetTipPMT - m_PMToffset*cos(90*deg-thetaPMT);
            xyzPMT = lengthCenterPMTAxisOriginToVesselInnerWall  - m_PMToffset - m_gelThicknessFrontCenterPMT;            
        }
        
        //upper equatorial
        if (i>=m_numberPolarPMTs + m_NrCenterPMTs && i<=m_numberPolarPMTs + m_NrCenterPMTs + m_NrEquatorialPMTs -1){ 
            //rotation
            thetaPMT = m_thetaEquatorial; 
            phiPMT = m_EqPMTPhiPhase + (i - m_numberPolarPMTs - m_NrCenterPMTs + 0.5) * 360. * deg / m_NrEquatorialPMTs; //i*90.0*deg; for 4 

            //PMT position
            zOffsetTipPMT = zOffsetEquatorialPMTAxisOrigin;
            zPMT = zOffsetTipPMT - m_PMToffset*cos(90*deg-thetaPMT);
            xyzPMT = lengthEquatorialPMTAxisOriginToVesselInnerWall  - m_PMToffset - m_gelThicknessFrontEquatorialPMT;
        }
        
        // Other module half

        //lower polar
        if (i>= m_numberPMTsPerHalf && i<= m_numberPMTsPerHalf + m_numberPolarPMTs -1){ 
            //rotation
            thetaPMT = 180*deg;
            phiPMT = 0*deg;

            //PMT position
            zOffsetTipPMT = m_GlassPoleLength - m_GlassThickPole - m_GelThicknessFrontPolarPMT;
            zPMT = -zOffsetTipPMT + m_PMToffset;
            xyzPMT = 0*mm;
        }

        //lower center
        if (i>= m_numberPMTsPerHalf + m_numberPolarPMTs && i<= m_numberPMTsPerHalf + m_numberPolarPMTs + m_NrCenterPMTs -1){ 
            //rotation
            thetaPMT = 180*deg -  m_thetaCenter;
            phiPMT = (i - m_numberPolarPMTs - m_NrCenterPMTs + 0.5) * 360. * deg / m_NrCenterPMTs; //i*90.0*deg; for 4 

            //PMT position
            zOffsetTipPMT = zOffsetCenterPMTAxisOrigin;
            zPMT = -zOffsetTipPMT + m_PMToffset*cos(180*deg-thetaPMT);
            xyzPMT = lengthCenterPMTAxisOriginToVesselInnerWall  - m_PMToffset - m_gelThicknessFrontCenterPMT;
        }

        //lower equatorial
        if (i>=m_totalNumberPMTs-m_NrEquatorialPMTs && i<=m_totalNumberPMTs-1){ 
            //rotation
            thetaPMT = 180*deg + m_thetaEquatorial;
            phiPMT = m_EqPMTPhiPhase + (i - m_numberPolarPMTs - m_NrCenterPMTs + 0.5) * 360. * deg / m_NrEquatorialPMTs; //i*90.0*deg; for 4 

            //PMT position
            zOffsetTipPMT = zOffsetEquatorialPMTAxisOrigin;
            zPMT = -zOffsetTipPMT + m_PMToffset*cos(180*deg-thetaPMT);
            xyzPMT = lengthEquatorialPMTAxisOriginToVesselInnerWall  - m_PMToffset - m_gelThicknessFrontEquatorialPMT;
        }

        //PMT positions
        xPMT = xyzPMT * sin(thetaPMT) * cos(phiPMT);
        yPMT = xyzPMT * sin(thetaPMT) * sin(phiPMT);
        m_positionsPMT.push_back( G4ThreeVector(xPMT,yPMT,zPMT) );

        //PMT angles
        m_thetaPMT.push_back(thetaPMT);
        m_phiPMT.push_back(phiPMT);
    }
}    

/**
 * Creation of LogicalVolume of gel pads
 * @param p_gelSolid Solid inner module volume. IntersectionSolid with PMT cones is filled with gel
 */
void LOM18::createGelpadLogicalVolumes(G4Polycone* p_gelSolid) 
{
    //getting the PMT solid
    G4VSolid* solidPMT = m_managerPMT->getPMTSolid();

    //Definition of helper volumes
    G4Cons* solidBasicCone;
    G4IntersectionSolid* temporalGelPad;
    G4LogicalVolume* logicalGelPad;
    G4SubtractionSolid* solidGelpad;

    //Helper variables
    G4double bufferGelpad = 50*mm; //needs to overlap everywhere to be cut later. Without this, the gelpad won't reach the vessel inner wall at all points and have a flat surface. Value can be anything as long as it is long enough
    G4ThreeVector shiftGelpad;
    G4Transform3D* transformer;

    //create logical volume for each gelpad    
    for(int k=0 ; k<=m_totalNumberPMTs-1 ; k++){
            m_conv.str("");
            m_converter2.str("");
            m_conv << "GelPad_" << k << "_solid";
            m_converter2 << "Gelpad_final" << k << "_logical";

            // polar gel pads
            if(k<=m_numberPolarPMTs-1 or (m_numberPMTsPerHalf<=k && k<=m_numberPMTsPerHalf)){ 
                solidBasicCone = new G4Cons("GelPadBasic", 0,m_maxPMTRadius , 0, m_maxPMTRadius  + 2*bufferGelpad*tan(m_polarPadOpeningAngle), bufferGelpad, 0, 2*CLHEP::pi); //creates a very long cone which has two flat sides at an irrelevant position
                
                //rotation and position of gelpad
                G4RotationMatrix* rot = new G4RotationMatrix();
                rot->rotateY(m_thetaPMT[k]);
                rot->rotateZ(m_phiPMT[k]); 
                G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

                //To shift BasicCone onto photocathode edge
                shiftGelpad = G4ThreeVector(0,0,bufferGelpad*cos(m_thetaPMT[k]) );
                transformer = new G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]+shiftGelpad));

                //creating volumes ... basic cone, subtract PMT, logical volume of gelpad
                temporalGelPad = new G4IntersectionSolid(m_conv.str(), p_gelSolid, solidBasicCone, *transformer); //Places beginning of cone at the edge of photocathode. Only intersection with inner volume counts.
                solidGelpad = new G4SubtractionSolid(m_conv.str(), temporalGelPad, solidPMT, transformers);
                
                logicalGelPad = new G4LogicalVolume(solidGelpad, m_data->getMaterial("RiAbs_Gel_Shin-Etsu"), m_converter2.str());
            }
            // center gel pads
            else if( (m_numberPolarPMTs<=k && k<=m_numberPolarPMTs+m_NrCenterPMTs-1) or (k>=m_numberPMTsPerHalf+m_numberPolarPMTs && k<= m_numberPMTsPerHalf+m_numberPolarPMTs+m_NrCenterPMTs-1) ){ 
                solidBasicCone = new G4Cons("GelPadBasic", 0,m_maxPMTRadius , 0, m_maxPMTRadius  + 2*bufferGelpad*tan(m_centerPadOpeningAngle), bufferGelpad, 0, 2*CLHEP::pi); //creates a very long cone which has two flat sides at an irrelevant position
                
                //rotation and position of gelpad
                G4RotationMatrix* rot = new G4RotationMatrix();
                rot->rotateY(m_thetaPMT[k]);
                rot->rotateZ(m_phiPMT[k]);
                G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

                //To shift BasicCone onto photocathode edge
                shiftGelpad = G4ThreeVector(bufferGelpad*sin(m_thetaPMT[k]) * cos(m_phiPMT[k]),bufferGelpad*sin(m_thetaPMT[k]) * sin(m_phiPMT[k]),bufferGelpad*cos(m_thetaPMT[k]) );
                transformer = new G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]+shiftGelpad));

                //creating volumes ... basic cone, subtract PMT, logical volume of gelpad
                temporalGelPad = new G4IntersectionSolid(m_conv.str(), p_gelSolid, solidBasicCone, *transformer); //Places beginning of cone at the edge of photocathode. Only intersection with inner volume counts.
                solidGelpad = new G4SubtractionSolid(m_conv.str(), temporalGelPad, solidPMT, transformers);
                logicalGelPad = new G4LogicalVolume(solidGelpad, m_data->getMaterial("RiAbs_Gel_Shin-Etsu"), m_converter2.str());
            }
            // equatorial gel pads
            else if(k<=m_numberPolarPMTs+m_NrCenterPMTs+m_NrEquatorialPMTs -1 or k>=m_totalNumberPMTs-m_NrEquatorialPMTs){ 
                solidBasicCone = new G4Cons("GelPadBasic", 0,m_maxPMTRadius , 0, m_maxPMTRadius  + 2*bufferGelpad*tan(m_equatorialPadOpeningAngle), bufferGelpad, 0, 2*CLHEP::pi); //creates a very long cone which has two flat sides at an irrelevant position
                
                //rotation and position of gelpad
                G4RotationMatrix* rot = new G4RotationMatrix();
                rot->rotateY(m_thetaPMT[k]);
                rot->rotateZ(m_phiPMT[k]);
                G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

                //To shift BasicCone onto photocathode edge
                shiftGelpad = G4ThreeVector(bufferGelpad*sin(m_thetaPMT[k]) * cos(m_phiPMT[k]),bufferGelpad*sin(m_thetaPMT[k]) * sin(m_phiPMT[k]),bufferGelpad*cos(m_thetaPMT[k]) );
                transformer = new G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]+shiftGelpad));

                //creating volumes ... basic cone, subtract PMT, logical volume of gelpad
                temporalGelPad = new G4IntersectionSolid(m_conv.str(), p_gelSolid, solidBasicCone, *transformer); //Places beginning of cone at the edge of photocathode. Only intersection with inner volume counts.
                solidGelpad = new G4SubtractionSolid(m_conv.str(), temporalGelPad, solidPMT, transformers);
                logicalGelPad = new G4LogicalVolume(solidGelpad, m_data->getMaterial("RiAbs_Gel_Shin-Etsu"), m_converter2.str());
            }

            
    //save logicalvolume of gelpads in array
    m_gelPadLogical.push_back( logicalGelPad ); 
    }
}

/**
 * Placement of PMTs
 * @param p_innerVolumeLogical LogicalVolume of inner module volume in which the PMTs are placed.
 */
void LOM18::placePMTs(G4LogicalVolume* p_innerVolumeLogical)
{
    for(int k=0 ; k<=m_totalNumberPMTs-1 ; k++){
        m_conv.str("");
        m_conv << "_"<< k;

        G4RotationMatrix* rot = new G4RotationMatrix();
        rot->rotateY(m_thetaPMT[k]);
        rot->rotateZ(m_phiPMT[k]);
        G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

        m_managerPMT->placeIt(transformers, p_innerVolumeLogical, m_conv.str());
    }

}

/**
 * Placement of gel pads
 * @param p_innerVolumeLogical LogicalVolume of inner module volume in which the gel pads are placed.
 */
void LOM18::placeGelpads(G4LogicalVolume* p_innerVolumeLogical)
{
    for(int k=0 ; k<=m_totalNumberPMTs-1 ; k++){
        m_conv.str("");
        m_conv << "GelPad_" << k;

        G4RotationMatrix* rot = new G4RotationMatrix();
        rot->rotateY(m_thetaPMT[k]);
        rot->rotateZ(m_phiPMT[k]);
        G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

        new G4PVPlacement(0, G4ThreeVector(0,0,0), m_gelPadLogical[k], m_conv.str(), p_innerVolumeLogical, false, 0, m_checkOverlaps);
    }
}
