#include "OMSimPMTConstruction.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimDetectorConstruction.hh"
#include "OMSimSensitiveDetector.hh"
#include "OMSimTools.hh"
#include "CADMesh.hh"

#include <G4Cons.hh>
#include <G4Ellipsoid.hh>
#include <G4LogicalBorderSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Sphere.hh>
#include <G4Torus.hh>
#include <OMSimLogger.hh>
#include <cmath>
#include "G4SystemOfUnits.hh"

OMSimPMTConstruction::OMSimPMTConstruction() : OMSimDetectorComponent()
{
    m_internalReflections = !OMSimCommandArgsTable::getInstance().get<bool>("simple_PMT");
}

/**
 * @brief Constructs the PMT solid with all its components.
 */
void OMSimPMTConstruction::construction()
{
    log_trace("Starting construction of PMT");
    // definePhotocathodeProperties();
    m_photocathodeOpticalSurface = m_data->getOpticalSurface("Surf_Generic_Photocathode_20nm");
    m_components.clear();
    G4VSolid *solidPMT;
    G4VSolid *vacuumPhotocathodeSolid;
    G4VSolid *glassInside;

    std::tie(solidPMT, vacuumPhotocathodeSolid) = getBulbSolid("jOuterShape");
    std::tie(glassInside, vacuumPhotocathodeSolid) = getBulbSolid("jInnerShape");
    G4SubtractionSolid *vacuumBack = new G4SubtractionSolid("Vacuum Tube solid", glassInside, vacuumPhotocathodeSolid, 0, G4ThreeVector(0, 0, 0));

    G4LogicalVolume *logicalPMT;

    // The ? of the following two lines are ternary operators that replace if-else-statements
    logicalPMT = new G4LogicalVolume(solidPMT, m_data->getMaterial(m_internalReflections ? "RiAbs_Glass_Tube" : "Ri_Glass_Tube"), "PMT tube logical");
    m_photocathodeLV = new G4LogicalVolume(vacuumPhotocathodeSolid, m_data->getMaterial("Ri_Vacuum"), "PhotocathodeRegionVacuum");

    G4LogicalVolume *tubeVacuum = new G4LogicalVolume(glassInside, m_data->getMaterial("Ri_Vacuum"), "PMTvacuum");
    G4LogicalVolume *vacuumBackLogical = new G4LogicalVolume(vacuumBack, m_data->getMaterial("Ri_Vacuum"), "PMTvacuum");

    m_photocathodeRegionVacuumPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), m_photocathodeLV, "PhotocathodeRegionVacuum", tubeVacuum, false, 0, m_checkOverlaps);

    if (true)
    {
        m_vacuumBackPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), vacuumBackLogical, "VacuumTubeBack", tubeVacuum, false, 0, m_checkOverlaps);
        constructCADdynodeSystem(vacuumBackLogical);
    }
    else
    {
        constructCathodeBackshield(tubeVacuum);
    }
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), tubeVacuum, "VacuumTube", logicalPMT, false, 0, m_checkOverlaps);

    appendComponent(solidPMT, logicalPMT, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PMT");
    if (m_HACoatingBool)
        constructHAcoating();

    m_photocathodeLV->SetVisAttributes(m_photocathodeVis);
    if (m_internalReflections && OMSimCommandArgsTable::getInstance().get<bool>("visual"))
    {
        log_warning("PMT shape too complicated for visualiser, and tube can't be visualised. Use simple_PMT or check {} if you want to try with another visualiser!", Tools::visualisationURL);
        logicalPMT->SetVisAttributes(m_invisibleVis);
        tubeVacuum->SetVisAttributes(m_invisibleVis);
        vacuumBackLogical->SetVisAttributes(m_invisibleVis);
    }
    else
    {
        logicalPMT->SetVisAttributes(m_glassVis);
        tubeVacuum->SetVisAttributes(m_airVis);
        vacuumBackLogical->SetVisAttributes(m_airVis);
    }
    m_constructionFinished = true;
    log_trace("Construction of PMT finished");
}

void OMSimPMTConstruction::constructHAcoating()
{
    log_trace("Constructing HA coating");
    readGlobalParameters("jOuterShape");
    // G4double lVisualCorr = 0.0*mm;
    // if (gVisual) lVisualCorr = 0.01*mm;
    G4Tubs *coatingUncut = new G4Tubs("HACoatingUncut", 0, 0.5 * m_tubeWidth + 0.1 * mm, m_missingTubeLength - 0.1 * mm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid *coatingCut = new G4SubtractionSolid("Bulb tube solid", coatingUncut, m_components.at("PMT").VSolid, 0, G4ThreeVector(0, 0, m_missingTubeLength));
    G4LogicalVolume *coatingLogical = new G4LogicalVolume(coatingCut, m_data->getMaterial("NoOptic_Absorber"), "HACoating");
    if (OMSimCommandArgsTable::getInstance().get<bool>("visual"))
    {
        log_warning("Visualization of HA coating is not possible. Check {} if you want to see the HA coating with another visualiser!", Tools::visualisationURL);
        coatingLogical->SetVisAttributes(m_invisibleVis);
    }
    appendComponent(coatingCut, coatingLogical, G4ThreeVector(0, 0, -m_missingTubeLength), G4RotationMatrix(), "HACoating");
}

/**
 * Placement of the PMT and definition of LogicalBorderSurfaces in case internal reflections are needed.
 * @param p_position G4ThreeVector with position of the module (as in G4PVPlacement())
 * @param p_rotation G4RotationMatrix with rotation of the module (as in G4PVPlacement())
 * @param p_mother G4LogicalVolume where the module is going to be placed (as in G4PVPlacement())
 * @param p_nameExtension G4String name of the physical volume. You should not have two physicals with the same name
 */
void OMSimPMTConstruction::placeIt(G4ThreeVector p_position, G4RotationMatrix p_rotation, G4LogicalVolume *&p_mother, G4String p_nameExtension)
{
    OMSimDetectorComponent::placeIt(p_position, p_rotation, p_mother, p_nameExtension);

    new G4LogicalBorderSurface("PhotoGlassToVacuum", m_lastPhysicals["PMT"], m_photocathodeRegionVacuumPhysical, m_photocathodeOpticalSurface);
    new G4LogicalBorderSurface("PhotoVacuumToGlass", m_photocathodeRegionVacuumPhysical, m_lastPhysicals["PMT"], m_photocathodeOpticalSurface);

    if (m_internalReflections)
    {
        new G4LogicalBorderSurface("PMT_mirrorglass", m_vacuumBackPhysical, m_lastPhysicals["PMT"], m_data->getOpticalSurface("Surf_PMTSideMirror"));
        new G4LogicalBorderSurface("PMT_mirrorglass", m_lastPhysicals["PMT"], m_vacuumBackPhysical, m_data->getOpticalSurface("Surf_PMTSideMirror"));
    }
}

/**
 * @see PMT::placeIt
 * @param p_transform G4Transform3D with position & rotation of PMT
 */
void OMSimPMTConstruction::placeIt(G4Transform3D p_transform, G4LogicalVolume *&p_mother, G4String p_nameExtension)
{
    OMSimDetectorComponent::placeIt(p_transform, p_mother, p_nameExtension);
    new G4LogicalBorderSurface("PhotoGlassToVacuum", m_lastPhysicals["PMT"], m_photocathodeRegionVacuumPhysical, m_photocathodeOpticalSurface);
    new G4LogicalBorderSurface("PhotoVacuumToGlass", m_photocathodeRegionVacuumPhysical, m_lastPhysicals["PMT"], m_photocathodeOpticalSurface);
    if (m_internalReflections)
    {
        new G4LogicalBorderSurface("PMT_mirrorglass", m_vacuumBackPhysical, m_lastPhysicals["PMT"], m_data->getOpticalSurface("Surf_PMTSideMirror"));
        new G4LogicalBorderSurface("PMT_mirrorglass", m_lastPhysicals["PMT"], m_vacuumBackPhysical, m_data->getOpticalSurface("Surf_PMTSideMirror"));
    }
}

/**
 * The basic shape of the PMT is constructed twice, once for the external solid and once for the internal. A subtraction of these two shapes would yield the glass envelope of the PMT. The function calls either simpleBulbConstruction or fullBulbConstruction, depending on the data provided and simulation type. In case only the frontal curvate of the photocathode has to be well constructed, it calls simpleBulbConstruction. fullBulbConstruction constructs the neck of the PMT precisely, but it needs to have the fit data of the PMT type and is only needed if internal reflections are simulated.
 * @see simpleBulbConstruction
 * @see fullBulbConstruction
 */
std::tuple<G4VSolid *, G4VSolid *> OMSimPMTConstruction::getBulbSolid(G4String p_side)
{
    G4SubtractionSolid *vacuumPhotocathodeSolid;
    G4String bulbBackShape = m_data->getValue<G4String>(m_selectedPMT, "jBulbBackShape");

    if (bulbBackShape == "Simple")
        m_simpleBulb = true;

    if (m_simpleBulb || !m_internalReflections)
    {
        return simpleBulbConstruction(p_side);
    }
    else
    {
        return fullBulbConstruction(p_side);
    }
}

/**
 * Construction of the basic shape of the PMT.
 * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
 */
std::tuple<G4VSolid *, G4VSolid *> OMSimPMTConstruction::simpleBulbConstruction(G4String p_side)
{
    log_trace("Constructing simple PMT bulb geometry");

    G4VSolid *bulbSolid = frontalBulbConstruction(p_side);
    // Defining volume with boundaries of photocathode volume
    G4Tubs *largeTube = new G4Tubs("LargeTube", 0, m_ellipseXYaxis, 50 * cm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid *photocathodeSide = new G4SubtractionSolid("SubstractionPhotocathodeSide", bulbSolid, largeTube, 0, G4ThreeVector(0, 0, -50 * cm));

    G4Tubs *bulkSolid = new G4Tubs("Bulb bulk solid", 0.0, 0.5 * m_tubeWidth, m_missingTubeLength, 0, 2 * CLHEP::pi);
    bulbSolid = new G4UnionSolid("Bulb tube solid", bulbSolid, bulkSolid, 0, G4ThreeVector(0, 0, -m_missingTubeLength));
    return std::make_tuple(bulbSolid, photocathodeSide);
}

/**
 * Construction of the basic shape of the PMT for a full paramterised PMT. This is needed if internal reflections are simulated.
 * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
 */
std::tuple<G4VSolid *, G4VSolid *> OMSimPMTConstruction::fullBulbConstruction(G4String p_side)
{
    log_trace("Constructing full PMT bulb geometry");

    G4double lineFitSlope = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jLineFitSlope");
    G4double xEllipseConeTransition = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipseConeTransition_x");
    G4double yEllipseConeTransition = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipseConeTransition_y");
    G4double xConeTorusTransition = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jConeTorusTransition_x");
    G4double rTorusCircle = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jTorusCircleR");
    G4double torusCirclePosX = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jTorusCirclePos_x");
    G4double torusCirclePosY = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jTorusCirclePos_y");
    G4double torusTubeTransitionY = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jTorusTubeTransition_y");

    G4VSolid *bulbSolid = frontalBulbConstruction(p_side);
    // Defining volume with boundaries of photocathode volume
    G4Tubs *largeTube = new G4Tubs("LargeTube", 0, m_ellipseXYaxis, 50 * cm, 0, 2 * CLHEP::pi);
    G4SubtractionSolid *photocathodeSide = new G4SubtractionSolid("SubstractionPhotocathodeSide", bulbSolid, largeTube, 0, G4ThreeVector(0, 0, -50 * cm));

    // Rest of tube
    G4Tubs *bulkSolid = new G4Tubs("Bulb bulk solid", 0.0, 0.5 * m_tubeWidth, m_missingTubeLength, 0, 2 * CLHEP::pi);

    // Creating Cone
    G4double coneLengthX = xEllipseConeTransition - xConeTorusTransition;
    G4double coneHalfHeight = lineFitSlope * coneLengthX * 0.5;
    G4Cons *cone = new G4Cons("Solid substraction cone", xConeTorusTransition, xConeTorusTransition + m_tubeWidth, xEllipseConeTransition, xEllipseConeTransition + m_tubeWidth, coneHalfHeight, 0, 2 * CLHEP::pi);

    // Cone is substracted from frontal volume
    G4double coneEllipseY = yEllipseConeTransition - m_ellipsePosY - coneHalfHeight;
    G4SubtractionSolid *bulbSolidSubstraction = new G4SubtractionSolid("Substracted solid bulb", bulbSolid,
                                                                       cone, 0, G4ThreeVector(0, 0, coneEllipseY));

    // Creating Torus
    G4Torus *torus = new G4Torus("Solid substraction torus", 0.0, rTorusCircle, torusCirclePosX, 0, 2 * CLHEP::pi);
    G4double torusToEllipse = torusCirclePosY - m_ellipsePosY;
    G4Tubs *tubeEdge = new G4Tubs("Solid edge of torus", torusCirclePosX, xEllipseConeTransition + m_tubeWidth, torusCirclePosX * 0.5, 0, 2 * CLHEP::pi);

    G4UnionSolid *torusTubeEdge = new G4UnionSolid("Solid torus with cylindrical edges", torus, tubeEdge, 0, G4ThreeVector(0, 0, 0));

    // Create Tube for substracting cone and torus
    G4double substractionTubeLength = yEllipseConeTransition - torusTubeTransitionY;
    G4Tubs *substractionTube = new G4Tubs("substracion_tube", 0.0, xEllipseConeTransition, 0.5 * substractionTubeLength, 0, 2 * CLHEP::pi);

    G4double tubeEllipeY = yEllipseConeTransition - m_ellipsePosY - substractionTubeLength * 0.5;

    G4SubtractionSolid *bulbBack = new G4SubtractionSolid("Solid back of PMT", substractionTube, cone, 0, G4ThreeVector(0, 0, coneEllipseY - tubeEllipeY));
    bulbBack = new G4SubtractionSolid("Solid back of PMT", bulbBack, torusTubeEdge, 0, G4ThreeVector(0, 0, torusToEllipse - tubeEllipeY));

    bulbSolid = new G4UnionSolid("Bulb tube solid", bulbSolidSubstraction, bulbBack, 0, G4ThreeVector(0, 0, tubeEllipeY));
    bulbSolid = new G4UnionSolid("Bulb tube solid", bulbSolid, bulkSolid, 0, G4ThreeVector(0, 0, -m_missingTubeLength));

    return std::make_tuple(bulbSolid, photocathodeSide);
}

/**
 * Creates and positions a thin disk behind the photocathode volume in order to shield photons coming from behind the PMT. Only used when internal reflections are turned off.
 */
void OMSimPMTConstruction::constructCathodeBackshield(G4LogicalVolume *p_PMTinner)
{
    log_trace("Constructing cathode back shield");
    readGlobalParameters("jInnerShape");
    G4double shieldWidth = 0.5 * mm;
    G4double shieldZPos = shieldWidth / 2;
    G4double furthestZ = shieldWidth + shieldZPos;
    G4double shieldRad = m_ellipseXYaxis * std::sqrt(1 - std::pow(furthestZ, 2.) / std::pow(m_ellipseZaxis, 2.));
    G4Tubs *shieldSolid = new G4Tubs("Shield solid", 0, shieldRad - 0.05 * mm, shieldWidth / 2, 0, 2 * CLHEP::pi);
    G4LogicalVolume *shieldLogical = new G4LogicalVolume(shieldSolid, m_data->getMaterial("NoOptic_Absorber"), "Shield logical");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -shieldWidth / 2), shieldLogical, "Shield physical", p_PMTinner, false, 0, m_checkOverlaps);
    shieldLogical->SetVisAttributes(m_blackVis);
}

/**
 * Construction & placement of the dynode system entrance for internal reflections. Currently only geometry for Hamamatsu R15458.
 * @param p_mother LogicalVolume of the mother, where the dynode system entrance is placed (vacuum volume)
 */
void OMSimPMTConstruction::constructCADdynodeSystem(G4LogicalVolume *p_mother)
{
    log_trace("Constructing CAD dynode system");
    auto supportStructureMesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/PMT/streifen.obj");
    auto frontalPanelMesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/PMT/frontalPlateonly.obj");
    auto dynodeMesh = CADMesh::TessellatedMesh::FromOBJ("../common/data/CADmeshes/PMT/dynodes.obj");

    G4double dynodeOffset = m_data->getValueWithUnit(m_selectedPMT, "jDynodeCADOffsetFromTip");
    G4double dynodeZ0 = m_data->getValueWithUnit(m_selectedPMT, "jDynodeCADZ0");
    G4double scale = m_data->getValueWithUnit(m_selectedPMT, "jDynodeCADscale");
    G4ThreeVector lCADoffset = G4ThreeVector(0, 0, getDistancePMTCenterToTip() - dynodeZ0 - dynodeOffset);
    supportStructureMesh->SetOffset(lCADoffset);
    frontalPanelMesh->SetOffset(lCADoffset);
    dynodeMesh->SetOffset(lCADoffset);
    supportStructureMesh->SetScale(scale);
    frontalPanelMesh->SetScale(scale);
    dynodeMesh->SetScale(scale);

    G4RotationMatrix *rot = new G4RotationMatrix();
    rot->rotateZ(90 * deg);

    G4LogicalVolume *supportStructure = new G4LogicalVolume(supportStructureMesh->GetSolid(), m_data->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
    G4LogicalVolume *frontalPlate = new G4LogicalVolume(frontalPanelMesh->GetSolid(), m_data->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
    G4LogicalVolume *dynodes = new G4LogicalVolume(dynodeMesh->GetSolid(), m_data->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);

    new G4LogicalSkinSurface("SkinFrontalPlate", frontalPlate, m_data->getOpticalSurface("Surf_PMTFrontPlate"));
    new G4LogicalSkinSurface("SkinSupportStructure", supportStructure, m_data->getOpticalSurface("Surf_AluminiumGround"));
    new G4LogicalSkinSurface("SkinDynodes", dynodes, m_data->getOpticalSurface("Surf_Dynode"));

    // lAbsorbers->SetVisAttributes(m_absorberVis);
    // new G4PVPlacement( rot , G4ThreeVector(0, 0, 0) , lAbsorbers, "DynodeSystemAbsorbers" , p_mother, false, 0, m_checkOverlaps);
    frontalPlate->SetVisAttributes(m_blueVis);
    new G4PVPlacement(rot, G4ThreeVector(0, 0, 0), frontalPlate, "frontalPlate", p_mother, false, 0, m_checkOverlaps);

    supportStructure->SetVisAttributes(m_boardVis);
    new G4PVPlacement(rot, G4ThreeVector(0, 0, 0), supportStructure, "DynodeSupportStructure", p_mother, false, 0, m_checkOverlaps);

    dynodes->SetVisAttributes(m_redVis);
    new G4PVPlacement(rot, G4ThreeVector(0, 0, 0), dynodes, "Dynodes", p_mother, false, 0, m_checkOverlaps);

    readGlobalParameters("jInnerShape");
    G4Tubs *shieldSolid = new G4Tubs("Shield solid", 0, 0.5 * m_tubeWidth - 0.05 * mm, 0.05 * mm / 2, 0, 2 * CLHEP::pi);
    G4LogicalVolume *shieldLogical = new G4LogicalVolume(shieldSolid, m_data->getMaterial("NoOptic_Absorber"), "logical", 0, 0, 0);
    shieldLogical->SetVisAttributes(m_absorberVis);
    new G4PVPlacement(rot, G4ThreeVector(0, 0, -0.6 * 2 * m_missingTubeLength), shieldLogical, "BackShield", p_mother, false, 0, m_checkOverlaps);
}

/**
 * @brief Reads the parameter table and assigns the value and dimension of member variables.
 */
void OMSimPMTConstruction::readGlobalParameters(G4String p_side)
{
    m_outRad = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jOutRad");
    m_ellipseXYaxis = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipseXYaxis");
    m_ellipseZaxis = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipseZaxis");
    m_spherePosY = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jSpherePos_y");
    m_ellipsePosY = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipsePos_y");
    m_sphereEllipseTransition_r = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jSphereEllipseTransition_r");
    m_totalLenght = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jTotalLenght");
    m_tubeWidth = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jTubeWidth");
    G4double lFrontToEllipse_y = m_outRad + m_spherePosY - m_ellipsePosY;
    m_missingTubeLength = (m_totalLenght - lFrontToEllipse_y) * 0.5 * mm;

}

G4VSolid *OMSimPMTConstruction::frontalBulbConstruction(G4String p_side)
{
    G4String frontalShape = m_data->getValue<G4String>(m_selectedPMT, "jFrontalShape");
    readGlobalParameters(p_side);
    if (frontalShape == "SphereEllipse")
        return sphereEllipsePhotocathode(p_side);
    else if (frontalShape == "TwoEllipses")
        return doubleEllipsePhotocathode(p_side);
    else if (frontalShape == "SingleEllipse")
        return ellipsePhotocathode(p_side);
    else
    {
        log_critical("Type of PMT frontal shape {} type not known!", frontalShape);
        throw std::runtime_error("Type of PMT frontal shape type not known!");
    }
}


/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with sphereEllipsePhotocathode were fitted with a sphere and an ellipse.
 * @return bulbSolid the frontal solid of the PMT
 */
G4VSolid *OMSimPMTConstruction::sphereEllipsePhotocathode(G4String p_side)
{
    log_trace("Constructing photocathode with one ellipsoid and a sphere");
    G4double sphereAngle = asin(m_sphereEllipseTransition_r / m_outRad);
    // PMT frontal glass envelope as union of sphere and ellipse
    G4Ellipsoid *bulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", m_ellipseXYaxis, m_ellipseXYaxis, m_ellipseZaxis);
    G4Sphere *bulbSphere = new G4Sphere("Solid Bulb Ellipsoid", 0.0, m_outRad, 0, 2 * CLHEP::pi, 0, sphereAngle);
    G4UnionSolid *bulbSolid = new G4UnionSolid("Solid Bulb", bulbEllipsoid, bulbSphere, 0, G4ThreeVector(0, 0, m_spherePosY - m_ellipsePosY));
    if (p_side=="jOuterShape")
    {
        m_centreToTipDistance = m_outRad + m_spherePosY - m_ellipsePosY;
    }
    
    return bulbSolid;
}

/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with ellipsePhotocathode were fitted with an ellipse.
 * @return bulbSolid the frontal solid of the PMT
 */
G4VSolid *OMSimPMTConstruction::ellipsePhotocathode(G4String p_side)
{
    log_trace("Constructing photocathode with one ellipsoid");
    G4Ellipsoid *bulbSolid = new G4Ellipsoid("Solid Bulb Ellipsoid", m_ellipseXYaxis, m_ellipseXYaxis, m_ellipseZaxis);

    if (p_side=="jOuterShape")
    {
        m_centreToTipDistance = m_ellipseZaxis;
    }
    
    return bulbSolid;
}


/**
 * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with doubleEllipsePhotocathode were fitted with two ellipses.
 * @return bulbSolid the frontal solid of the PMT
 */
G4VSolid *OMSimPMTConstruction::doubleEllipsePhotocathode(G4String p_side)
{
    log_trace("Constructing photocathode with two ellipses");
    G4double ellipseXYAxis2 = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipseXYaxis_2");
    G4double ellipseZAxis2 = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipseZaxis_2");
    G4double ellipseYpos2 = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipsePos_y_2");

    G4double ellipseEllipseTransitionY = m_ellipsePosY;
    if(m_data->checkIfKeyInTree(m_selectedPMT, p_side + ".jEllipseEllipseTransition_y"))
    {
        ellipseEllipseTransitionY = m_data->getValueWithUnit(m_selectedPMT, p_side + ".jEllipseEllipseTransition_y");
    }
     

    G4Ellipsoid *bulbEllipsoid = new G4Ellipsoid("Solid Bulb Ellipsoid", m_ellipseXYaxis, m_ellipseXYaxis, m_ellipseZaxis);
    G4Ellipsoid *bulbEllipsoid2 = new G4Ellipsoid("Solid Bulb Ellipsoid 2", ellipseXYAxis2, ellipseXYAxis2, ellipseZAxis2);

    G4double excess = ellipseEllipseTransitionY-ellipseYpos2;

    G4Tubs *substractionTube = new G4Tubs("substracion_tube_large_ellipsoid", 0.0, ellipseXYAxis2 * 3, ellipseZAxis2, 0, 2 * CLHEP::pi);
    G4SubtractionSolid *substractedLargeEllipsoid = new G4SubtractionSolid("Substracted Bulb Ellipsoid 2", bulbEllipsoid2,
                                                                           substractionTube, 0, G4ThreeVector(0, 0, excess-ellipseZAxis2));

    G4UnionSolid *bulbSolid = new G4UnionSolid("Solid Bulb", bulbEllipsoid, substractedLargeEllipsoid, 0, G4ThreeVector(0, 0, -m_ellipsePosY + ellipseYpos2));
    
    if (p_side=="jOuterShape")
    {
        m_centreToTipDistance = ellipseYpos2 +ellipseZAxis2 - m_ellipsePosY;
    }
    
    return bulbSolid;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Main class methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 * @return G4double the distance between the 0.0 position of the PMT solid volume and the plane normal to the PMT frontal tip.
 */
G4double OMSimPMTConstruction::getDistancePMTCenterToTip()
{
    log_trace("Distance from PMT center to tip {}", m_centreToTipDistance);
    return m_centreToTipDistance;
}

/**
 * @return G4double the maximal radius of the frontal part of the PMT.
 */
G4double OMSimPMTConstruction::getMaxPMTRadius()
{
    readGlobalParameters("jOuterShape");
    return m_ellipseXYaxis;
}

/**
 * @returns the solid of the constructed PMT.
 */
G4VSolid *OMSimPMTConstruction::getPMTSolid()
{
    return m_components.at("PMT").VSolid;
}

/**
 * @return the bulb glass logical volume (PMT mother).
 */
G4LogicalVolume *OMSimPMTConstruction::getLogicalVolume()
{
    return m_components.at("PMT").VLogical;
}

/**
 * Select PMT model to use and assigns mPMT class.
 * @param p_selectedPMT string with the name of the PMT model
 */
void OMSimPMTConstruction::selectPMT(G4String p_selectedPMT)
{
    if (p_selectedPMT.substr(0, 6) == "argPMT")
    {
        const G4String listPMT[] = {"pmt_Hamamatsu_R15458_CT", "pmt_Hamamatsu_R7081", "pmt_Hamamatsu_4inch", "pmt_Hamamatsu_R5912_20_100"};
        p_selectedPMT = listPMT[OMSimCommandArgsTable::getInstance().get<G4int>("pmt_model")];
    }

    m_selectedPMT = p_selectedPMT;

    // Check if requested PMT is in the table of PMTs
    if (m_data->checkIfTreeNameInTable(p_selectedPMT))
    { // if found
        log_info("PMT type {} selected", p_selectedPMT);
    }
    else
    {
        log_critical("Selected PMT type not in PMT dictionary, please check that requested PMT exists in data folder.");
    }
}

void OMSimPMTConstruction::includeHAcoating()
{
    m_HACoatingBool = true;
    if (m_constructionFinished)
    {
        log_warning("You should call this function before Construction(), otherwise we have to construct everything twice...");
        construction();
    }
}

G4double OMSimPMTConstruction::getPMTGlassWeight()
{
    log_trace("Getting PMT bulb weight");
    try
    {
        return m_data->getValue<double>(m_selectedPMT, "jBulbWeight");
    }
    catch (const std::exception &e)
    {
        log_warning("PMT table has no bulb weight defined (key jBulbWeight does not exist). Simulation of PMT scintillation cannot be performed!");
        return 0 * kg;
    }
}



/**
 * @brief Creates and configures an instance of OMSimPMTResponse based on PMT model selected.
 */
OMSimPMTResponse *OMSimPMTConstruction::getPMTResponseInstance()
{
    std::unique_ptr<OMSimPMTResponse> responsePMT = std::make_unique<OMSimPMTResponse>();
    std::string fileQE = OMSimCommandArgsTable::getInstance().get<std::string>("QE_file");
    if (fileQE == "default")
    {
        if (m_data->checkIfKeyInTree(m_selectedPMT, "jDefaultQEFileName"))
            fileQE = m_data->getValue<std::string>(m_selectedPMT, "jDefaultQEFileName");
    }
    if (fileQE != "default")
        responsePMT->makeQEInterpolator(fileQE);

    if (m_data->checkIfKeyInTree(m_selectedPMT, "jAbsorbedFractionFileName"))
        responsePMT->makeQEweightInterpolator(m_data->getValue<std::string>(m_selectedPMT, "jAbsorbedFractionFileName"));
    if (m_data->checkIfKeyInTree(m_selectedPMT, "jCEweightsFileName"))
        responsePMT->makeCEweightInterpolator(m_data->getValue<std::string>(m_selectedPMT, "jCEweightsFileName"));

    if (m_data->checkIfKeyInTree(m_selectedPMT, "jScannedWavelengths"))
    {
        std::vector<G4double> scannedWavelengths;
        m_data->parseKeyContentToVector(scannedWavelengths, m_selectedPMT, "jScannedWavelengths", 1.*nm, false);
        responsePMT->setScannedWavelengths(scannedWavelengths);
        if ((m_data->checkIfKeyInTree(m_selectedPMT, "jScanDataPath")))
            responsePMT->makeScansInterpolators(m_data->getValue<std::string>(m_selectedPMT, "jScanDataPath"));
    }

    return responsePMT.release();
}

void OMSimPMTConstruction::configureSensitiveVolume(OMSimDetectorConstruction *p_detectorConstruction, G4String p_name)
{
    log_debug("Configuring PMTs of {} as sensitive detector", p_name);
    DetectorType detectorType = OMSimCommandArgsTable::getInstance().get<bool>("simple_PMT") ? DetectorType::PerfectPMT : DetectorType::PMT;
    OMSimSensitiveDetector *sensitiveDetector = new OMSimSensitiveDetector(p_name, detectorType);
    sensitiveDetector->setPMTResponse(getPMTResponseInstance());
    p_detectorConstruction->registerSensitiveDetector(m_photocathodeLV, sensitiveDetector);
}
