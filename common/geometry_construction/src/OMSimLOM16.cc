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
    // delete m_harness;
}

LOM16::LOM16(G4bool pPlaceHarness): OMSimOpticalModule(new OMSimPMTConstruction()), m_placeHarness(pPlaceHarness)
{
    log_info("Constructing LOM16");
    m_managerPMT->includeHAcoating();
    m_managerPMT->selectPMT(m_PMTModel);
    m_managerPMT->construction();
    m_PMToffset = m_managerPMT->getDistancePMTCenterToTip();
    m_maxPMTRadius = m_managerPMT->getMaxPMTRadius() + 2 * mm;
    if (m_placeHarness)
    {
        // m_harness = new mDOMHarness(this, m_data);
        // integrateDetectorComponent(m_harness, G4ThreeVector(0,0,0), G4RotationMatrix(), "");
        log_error("LOM16 harness not implemented yet");
    }

    construction();
}


void LOM16::construction()
{
    G4VSolid *glassSolid = pressureVessel(m_glassOutRad, "Glass");
    G4VSolid *airSolid = pressureVessel(m_glassInRad, "Gel"); // Fill entire vessel with gel as logical volume (not placed) for intersectionsolids with gelpads

    // Set positions and rotations of PMTs and gelpads
    setPMTAndGelpadPositions();

    // CAD internal components

    placeCADSupportStructure();
    glassSolid = substractToVolume(glassSolid, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Glass");
    airSolid = substractToVolume(airSolid, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Gel");
    
    // Logicals
    G4LogicalVolume *glassLogical = new G4LogicalVolume(glassSolid, m_data->getMaterial("RiAbs_Glass_Vitrovex"), " Glass_log"); // Vessel
    G4LogicalVolume *p_innerVolume = new G4LogicalVolume(airSolid, m_data->getMaterial("Ri_Air"), "Inner volume logical"); // Inner volume of vessel (mothervolume of all internal components)
    createGelpadLogicalVolumes(airSolid);                                                                                       // logicalvolumes of all gelpads saved globally to be placed below

    // Placements
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), p_innerVolume, "Gel_physical", glassLogical, false, 0, m_checkOverlaps);

    placePMTs(p_innerVolume);
    placeGelpads(p_innerVolume);

    appendComponent(glassSolid, glassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel_" + std::to_string(m_index));
    appendEquatorBand();

    // ---------------- visualisation attributes --------------------------------------------------------------------------------
    glassLogical->SetVisAttributes(m_glassVis);
    p_innerVolume->SetVisAttributes(m_invisibleVis); // Material defined as Ri_Air
    for (int i = 0; i <= m_totalNumberPMTs - 1; i++)
    {
        m_gelPadLogical[i]->SetVisAttributes(m_gelVis); // m_gelVis
    }
}

// ---------------- Component functions --------------------------------------------------------------------------------

G4UnionSolid *LOM16::pressureVessel(const G4double pOutRad, G4String pSuffix)
{
    G4Ellipsoid *topSolid = new G4Ellipsoid("SphereTop solid" + pSuffix, pOutRad, pOutRad, pOutRad, -5 * mm, pOutRad + 5 * mm);
    G4Ellipsoid *bottomSolid = new G4Ellipsoid("SphereBottom solid" + pSuffix, pOutRad, pOutRad, pOutRad, -(pOutRad + 5 * mm), 5 * mm);

    G4double zCorners[] = {m_cylinderHeight * 1.001, m_cylinderHeight, 0, -m_cylinderHeight, -m_cylinderHeight * 1.001};
    G4double rCorners[] = {0, pOutRad, pOutRad + m_cylinderHeight * sin(m_cylinderAngle), pOutRad, 0};
    G4Polycone *cylinderSolid = new G4Polycone("Cylinder solid" + pSuffix, 0, 2 * CLHEP::pi, 5, rCorners, zCorners);

    G4UnionSolid *temporalUnion = new G4UnionSolid("temp" + pSuffix, cylinderSolid, topSolid, 0, G4ThreeVector(0, 0, m_cylinderHeight));
    G4UnionSolid *unionSolid = new G4UnionSolid("OM body" + pSuffix, temporalUnion, bottomSolid, 0, G4ThreeVector(0, 0, -m_cylinderHeight));
    return unionSolid;
}

void LOM16::appendEquatorBand()
{

    G4Box *cuttingBox = new G4Box("Cutter", m_glassOutRad + 10 * mm, m_glassOutRad + 10 * mm, m_equatorialBandWidth / 2.);
    G4UnionSolid *outerSolid = pressureVessel(m_glassOutRad + m_equatorialBandThickness, "BandOuter");

    G4IntersectionSolid *intersectionSolid = new G4IntersectionSolid("BandThicknessBody", cuttingBox, outerSolid, 0, G4ThreeVector(0, 0, 0));
    G4SubtractionSolid *equatorBandSolid = new G4SubtractionSolid("Equatorband_solid", intersectionSolid,
                                                                   m_components.at("PressureVessel_" + std::to_string(m_index)).VSolid, 0, G4ThreeVector(0, 0, 0));

    G4LogicalVolume *equatorBandLogical = new G4LogicalVolume(equatorBandSolid, m_data->getMaterial("NoOptic_Absorber"), "Equatorband_log");
    equatorBandLogical->SetVisAttributes(m_absorberVis);
    appendComponent(equatorBandSolid, equatorBandLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "TeraTape");
}

// ---------------- Module specific funtions below --------------------------------------------------------------------------------

// Places internal components based on a CAD file. OBJ filetype, modified by python script, tesselated via mesh.cc / Documentation here: https://github.com/christopherpoole/CADMesh
// To do:
// Recreate CAD obj files (no SetScale & more variety to choose from)
// Each component has its own label in visualizer -> just one ... another function for penetrator, dummy main boards, ...
void LOM16::placeCADSupportStructure()
{

    G4String filePath = "../common/data/CADmeshes/LOM16/LOM_Internal_WithPen_WithCali.obj";
    G4String mssg = "Using the following CAD file for LOM16 internal structure: " + filePath;
    log_info(mssg);

    // load mesh
    auto mesh = CADMesh::TessellatedMesh::FromOBJ(filePath);
    G4ThreeVector lCADoffset = G4ThreeVector(m_xInternalCAD, m_yInternalCAD, m_zInternalCAD);
    mesh->SetOffset(lCADoffset);

    // mesh->SetScale(10); //did a mistake...this LOM_Internal file needs cm -> mm -> x10
    // Place all of the meshes it can find in the file as solids individually.
    for (auto iSolid : mesh->GetSolids())
    {
        G4LogicalVolume *supportStructureLogical = new G4LogicalVolume(iSolid, m_data->getMaterial("NoOptic_Absorber"), "SupportStructureCAD_Logical");
        supportStructureLogical->SetVisAttributes(m_absorberVis);
        appendComponent(iSolid, supportStructureLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "SupportStructureCAD");
    }
}

// ToDo:
// sin(90 +- ...) -> cos(...)
void LOM16::setPMTAndGelpadPositions()
{
    const G4double totalLength = m_data->getValueWithUnit(m_PMTModel, "jOuterShape.jTotalLenght");
    const G4double cetreModuleToBottomPMTPolar = 70.9 * mm;
    const G4double centreModuleToBottomPMTEquatorial = 25.4 * mm;

    const G4double zCentreBottomPMT = totalLength - m_PMToffset; // Distance from bottom base of PMT to center of the Geant4 solid
    G4double thetaPMT, phiPMT, xPMT, yPMT, zPMT, xGelPad, yGelPad, zGelPad, rhoPMT, rhoGelpad;
    // calculate PMT and pads positions in the usual 4 for loop way
    for (int i = 0; i <= m_totalNumberPMTs - 1; i++)
    {

        // upper polar
        if (i >= 0 && i <= m_numberPolarPMTs - 1)
        {
            thetaPMT = m_thetaPolar;
            phiPMT = m_polarEquatorialPMTphiPhase + i * 90.0 * deg;

            rhoPMT = (147.7406 - 4.9) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            rhoGelpad = rhoPMT + 2 * m_gelPadDZ;

            zPMT = cetreModuleToBottomPMTPolar + zCentreBottomPMT * sin(90 * deg - thetaPMT);
            zGelPad = zPMT + 2 * m_gelPadDZ * sin(90 * deg - thetaPMT);
        }

        // upper equatorial
        if (i >= m_numberPolarPMTs && i <= m_numberPolarPMTs + m_numberEqPMTs - 1)
        {
            thetaPMT = m_thetaEquatorial;
            phiPMT = i * 90.0 * deg;

            rhoPMT = (120.1640 - 5) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            rhoGelpad = rhoPMT + 2 * m_gelPadDZ;

            zPMT = centreModuleToBottomPMTEquatorial + zCentreBottomPMT * sin(90 * deg - thetaPMT);
        }

        // lower equatorial
        if (i >= m_numberPolarPMTs + m_numberEqPMTs && i <= m_numberPolarPMTs + m_numberEqPMTs + m_numberEqPMTs - 1)
        {
            thetaPMT = 180 * deg - m_thetaEquatorial; // 118*deg; // 118.5*deg;
            phiPMT = i * 90.0 * deg;

            rhoPMT = (120.1640 - 5) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            rhoGelpad = rhoPMT + 2 * m_gelPadDZ;

            zPMT = (-centreModuleToBottomPMTEquatorial) - zCentreBottomPMT * sin(90 * deg - (180 * deg - thetaPMT)); // rodo to cos ...
        }

        // lower polar
        if (i >= m_totalNumberPMTs - m_numberPolarPMTs && i <= m_totalNumberPMTs - 1)
        {
            thetaPMT = 180 * deg - m_thetaPolar; // 144*deg; //152*deg;
            phiPMT = m_polarEquatorialPMTphiPhase + ((i)*90.0) * deg;

            rhoPMT = (147.7406 - 4.9) * mm; // For Position of PMT to Vessel wall (147.7406). 4.9mm is distance from PMT photocathode to vessel inner surface
            rhoGelpad = rhoPMT + 2 * m_gelPadDZ;

            zPMT = (-cetreModuleToBottomPMTPolar) - zCentreBottomPMT * sin(90 * deg - (180 * deg - thetaPMT));
            zGelPad = zPMT - 2 * m_gelPadDZ * sin(90 * deg - (180 * deg - thetaPMT));
        }

        // PMTs
        xPMT = (rhoPMT * mm) * sin(thetaPMT) * cos(phiPMT);
        yPMT = (rhoPMT * mm) * sin(thetaPMT) * sin(phiPMT);

        // Gelpads
        xGelPad = rhoGelpad * sin(thetaPMT) * cos(phiPMT);
        yGelPad = rhoGelpad * sin(thetaPMT) * sin(phiPMT);

        // save positions in arrays
        m_positionsPMT.push_back(G4ThreeVector(xPMT, yPMT, zPMT));
        m_positionsGelpad.push_back(G4ThreeVector(xGelPad, yGelPad, zGelPad));

        // save angles in arrays
        m_thetaPMT.push_back(thetaPMT);
        m_phiPMT.push_back(phiPMT);
    }
}

// Todo
// 4*m_gelPadDZ -> 2 2*m_gelPadDZ -> 1 ...  not needed if m_gelPadDZ is long enough
// tra and transformers declaration uniformely.
// rename some stuff for clarity
void LOM16::createGelpadLogicalVolumes(G4VSolid *p_gelSolid)
{
    // getting the PMT solid
    G4VSolid *solidPMT = m_managerPMT->getPMTSolid();

    // Definition of helper volumes
    G4Cons *gelPadBasicSolid;
    G4EllipticalCone *gelPadBasicSolidEquatorial;
    G4IntersectionSolid *cutCone;
    G4LogicalVolume *gelPadLogical;
    G4SubtractionSolid *cutConeFinal;

    // Definition of semiaxes in (elliptical section) cone for titled gel pads
    G4double dx = std::cos(m_equatorialTiltAngle) / (2 * (1 + std::sin(m_equatorialTiltAngle) * std::tan(m_equatorialPadOpeningAngle))) * m_maxPMTRadius * 2; // semiaxis y at -ztop
    G4double ztop = 2 * m_gelPadDZ;
    G4double dy = m_maxPMTRadius;                                // semiaxis x at -ztop
    G4double Dy = dy + 2 * ztop * std::tan(m_equatorialPadOpeningAngle); // semiaxis x at +ztop
    G4double Dx = dx * Dy / dy;                                 //     Dx/dx=Dy/dy always ;  //semiaxis y at -ztop
    G4double xsemiaxis = (Dx - dx) / (2 * ztop);                // Best way it can be defined
    G4double ysemiaxis = (Dy - dy) / (2 * ztop);                // Best way it can be defined
    G4double zmax = (Dx + dx) / (2 * xsemiaxis);                // Best way it can be defined

    // For the placement of tilted  equatorial pads
    G4double dz = -dx * std::sin(m_equatorialTiltAngle);
    G4double dY3 = m_maxPMTRadius - (ztop * std::sin(m_equatorialTiltAngle) + dx * std::cos(m_equatorialTiltAngle));

    // create logical volume for each gelpad
    for (int k = 0; k <= m_totalNumberPMTs - 1; k++)
    {
        G4Transform3D *tra;
        G4Transform3D *tra2;

        m_converter.str("");
        m_converter2.str("");
        m_converter << "GelPad_" << k << "_solid";
        m_converter2 << "Gelpad_final" << k << "_logical";

        // polar gel pads
        if (k <= m_numberPolarPMTs - 1 or k >= m_totalNumberPMTs - m_numberPolarPMTs)
        {
            gelPadBasicSolid = new G4Cons("GelPadBasic", 0, m_maxPMTRadius, 0, m_maxPMTRadius + 4 * m_gelPadDZ * tan(m_polarPadOpeningAngle), 2 * m_gelPadDZ, 0, 2 * CLHEP::pi);

            // rotation and position of gelpad
            G4RotationMatrix *rotation = new G4RotationMatrix();
            rotation->rotateY(m_thetaPMT[k]);
            rotation->rotateZ(m_phiPMT[k]);

            tra = new G4Transform3D(*rotation, G4ThreeVector(m_positionsGelpad[k]));
            G4Transform3D transformers = G4Transform3D(*rotation, G4ThreeVector(m_positionsPMT[k]));

            // creating volumes ... basic cone, subtract PMT, logical volume of gelpad
            cutCone = new G4IntersectionSolid(m_converter.str(), p_gelSolid, gelPadBasicSolid, *tra);
            cutConeFinal = new G4SubtractionSolid(m_converter.str(), cutCone, solidPMT, transformers);
            gelPadLogical = new G4LogicalVolume(cutConeFinal, m_data->getMaterial("RiAbs_Gel_Shin-Etsu"), m_converter2.str());
        };

        // upper equatorial
        if (k >= m_numberPolarPMTs && k <= m_numberPolarPMTs + m_numberEqPMTs - 1)
        {
            // the tilted pad is a cone with elliptical section. Moreover, there is an union with a disc called "Tube". This union does not affect the geometry, but it is used to shift the center of the geant solid so it can be placed in the same way as the PMT. In addition to that, Cut_tube is used to do a better substraction of the residual part that lies below the photocatode.
            gelPadBasicSolidEquatorial = new G4EllipticalCone("cone", xsemiaxis, ysemiaxis, zmax, ztop);

            // rotation and position of tilted gelpad
            G4RotationMatrix *rotation = new G4RotationMatrix();
            rotation->rotateY((180 * deg + m_equatorialTiltAngle));
            tra2 = new G4Transform3D(*rotation, G4ThreeVector(-dY3, 0, ztop * std::cos(m_equatorialTiltAngle) + dz));

            // place tilted cone
            G4Tubs *Tube = new G4Tubs("tube", 0, m_maxPMTRadius, 0.1 * mm, 0., 2 * CLHEP::pi);
            G4UnionSolid *Tilted_cone = new G4UnionSolid("union", Tube, gelPadBasicSolidEquatorial, *tra2);

            // For subtraction of gelpad reaching below the photocathode
            rotation = new G4RotationMatrix();
            tra = new G4Transform3D(*rotation, G4ThreeVector(0, 0, -30 * mm)); // 30 -> thickness/width of subtraction box ... just has to be large eough to cut overlapping gelpads under photocathode

            // rotation and position of PMT
            G4RotationMatrix *rot = new G4RotationMatrix();
            rot->rotateY(m_thetaPMT[k]);
            rot->rotateZ(m_phiPMT[k]);
            G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

            // creating volumes ... basic cone, tilted cone, subtract PMT, logical volume of gelpad
            G4Tubs *Cut_tube = new G4Tubs("tub", 0, 2 * m_maxPMTRadius, 30 * mm, 0, 2 * CLHEP::pi);
            G4SubtractionSolid *Tilted_final = new G4SubtractionSolid("cut", Tilted_cone, Cut_tube, *tra);
            cutCone = new G4IntersectionSolid(m_converter.str(), p_gelSolid, Tilted_final, transformers);
            cutConeFinal = new G4SubtractionSolid(m_converter.str(), cutCone, solidPMT, transformers);
            gelPadLogical = new G4LogicalVolume(cutConeFinal, m_data->getMaterial("RiAbs_Gel_Shin-Etsu"), m_converter2.str());
        };

        // lower equatorial
        if (k >= m_numberPolarPMTs + m_numberEqPMTs && k <= m_totalNumberPMTs - m_numberPolarPMTs - 1)
        {
            gelPadBasicSolidEquatorial = new G4EllipticalCone("cone", xsemiaxis, ysemiaxis, zmax, ztop);

            // rotation and position of tilted gelpad
            G4RotationMatrix *rotation = new G4RotationMatrix();
            rotation->rotateY((180 * deg - m_equatorialTiltAngle));
            tra2 = new G4Transform3D(*rotation, G4ThreeVector(dY3, 0, ztop * std::cos(m_equatorialTiltAngle) + dz));

            // place tilted cone
            G4Tubs *Tube = new G4Tubs("tube", 0, m_maxPMTRadius, 0.1 * mm, 0., 2 * CLHEP::pi);
            G4UnionSolid *Tilted_cone = new G4UnionSolid("union", Tube, gelPadBasicSolidEquatorial, *tra2);

            // For subtraction of gelpad reaching below the photocathode
            rotation = new G4RotationMatrix();
            tra = new G4Transform3D(*rotation, G4ThreeVector(0, 0, -30 * mm));

            // rotation and position of PMT
            G4RotationMatrix *rot = new G4RotationMatrix();
            rot->rotateY(m_thetaPMT[k]);
            rot->rotateZ(m_phiPMT[k]);
            G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

            // creating volumes ... basic cone, tilted cone, subtract PMT, logical volume of gelpad
            G4Tubs *Cut_tube = new G4Tubs("tub", 0, 2 * m_maxPMTRadius, 30 * mm, 0, 2 * CLHEP::pi);
            G4SubtractionSolid *Tilted_final = new G4SubtractionSolid("cut", Tilted_cone, Cut_tube, *tra);
            cutCone = new G4IntersectionSolid(m_converter.str(), p_gelSolid, Tilted_final, transformers);
            cutConeFinal = new G4SubtractionSolid(m_converter.str(), cutCone, solidPMT, transformers);
            gelPadLogical = new G4LogicalVolume(cutConeFinal, m_data->getMaterial("RiAbs_Gel_Shin-Etsu"), m_converter2.str());
        };

        // save logicalvolume of gelpads in array
        m_gelPadLogical.push_back(gelPadLogical); //
    }
}

void LOM16::placePMTs(G4LogicalVolume *p_innerVolume)
{
    for (int k = 0; k <= m_totalNumberPMTs - 1; k++)
    {
        m_converter.str("");
        m_converter << "_" << k;

        G4RotationMatrix *rot = new G4RotationMatrix();
        rot->rotateY(m_thetaPMT[k]);
        rot->rotateZ(m_phiPMT[k]);
        G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

        m_managerPMT->placeIt(transformers, p_innerVolume, m_converter.str());
    }
}

void LOM16::placeGelpads(G4LogicalVolume *p_innerVolume)
{
    for (int k = 0; k <= m_totalNumberPMTs - 1; k++)
    {
        m_converter.str("");
        m_converter << "GelPad_" << k;

        G4RotationMatrix *rot = new G4RotationMatrix();
        rot->rotateY(m_thetaPMT[k]);
        rot->rotateZ(m_phiPMT[k]);
        G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(m_positionsPMT[k]));

        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), m_gelPadLogical[k], m_converter.str(), p_innerVolume, false, 0, m_checkOverlaps);
    }
}
