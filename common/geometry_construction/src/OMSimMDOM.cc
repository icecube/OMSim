/**
 *  @todo - Write documentation and parse current comments into Doxygen style
 */

#include "OMSimMDOM.hh"
#include "OMSimMDOMHarness.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimLogger.hh"
#include "OMSimTools.hh"

#include <G4Ellipsoid.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Polycone.hh>

mDOM::~mDOM()
{
    delete m_flashers;
    if (m_placeHarness)
    {
        delete m_harness;
    }
}
mDOM::mDOM(G4bool p_placeHarness)
    : OMSimOpticalModule(new OMSimPMTConstruction()),
      m_placeHarness(p_placeHarness),
      m_flashers(nullptr),
      m_harness(nullptr),
      m_refConeIdealInRad(0),
      m_PMToffset(0)
{
    log_info("Constructing mDOM");
    m_flashers = new mDOMFlasher();
    m_managerPMT->selectPMT("pmt_Hamamatsu_R15458_CT");
    m_managerPMT->construction();
    m_refConeIdealInRad = m_managerPMT->getMaxPMTRadius() + 2 * mm;
    m_PMToffset = m_managerPMT->getDistancePMTCenterToTip();

    if (m_placeHarness)
    {
        m_harness = new mDOMHarness(this);
        integrateDetectorComponent(m_harness, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "");
    }

    construction();
    log_trace("Finished constructing mDOM");
}

void mDOM::construction()
{

    setPMTPositions();
    setLEDPositions();

    G4UnionSolid *glassSolid = pressureVessel(m_glassOutRad, "Glass");
    G4UnionSolid *gelSolid = pressureVessel(m_glassInRad, "Gel");

    G4SubtractionSolid *supStructureSolid;
    G4UnionSolid *supStructureTemporalSolid;

    std::tie(supStructureSolid, supStructureTemporalSolid) = supportStructure();

    // Reflector solid
    G4Cons *lRefConeBasicSolid = new G4Cons("RefConeBasic", m_refConeIdealInRad,
                                            m_refConeIdealInRad + m_reflectorConeSheetThickness / cos(mRefConeAngle),
                                            m_refConeIdealInRad + 2 * m_reflectorHalfZ * tan(mRefConeAngle), m_refConeIdealInRad + m_reflectorConeSheetThickness / cos(mRefConeAngle) + 2 * m_reflectorHalfZ * tan(mRefConeAngle), m_reflectorHalfZ, 0, 2 * CLHEP::pi);

    // Reflector cone for polar PMTs
    G4Ellipsoid *supStructureTopSolid = new G4Ellipsoid("FoamSphereTop solid", m_supStructureRad, m_supStructureRad, m_supStructureRad, -5 * mm, m_supStructureRad + 5 * mm);
    G4IntersectionSolid *reflectorPolarSolid = new G4IntersectionSolid("PolarRefCones", lRefConeBasicSolid, supStructureTopSolid, 0, G4ThreeVector(0, 0, -(m_glassInRad - m_gelThicknessFrontPMT - m_PMToffset + m_reflectorHalfZ)));

    // Reflector cone for equatorial PMTs
    G4SubtractionSolid *reflectorEqUpCutSolid = equatorialReflector(supStructureTemporalSolid, lRefConeBasicSolid, m_thetaEquatorial, "upper");
    G4SubtractionSolid *reflectorEqLowCutSolid = equatorialReflector(supStructureTemporalSolid, lRefConeBasicSolid, 180. * deg - m_thetaEquatorial, "lower");

    // LED flashers
    supStructureSolid = substractFlashers(supStructureSolid);

    // Logicals
    G4LogicalVolume *gelLogical = new G4LogicalVolume(gelSolid,
                                                      m_data->getMaterial("RiAbs_Gel_Shin-Etsu"),
                                                      "Gelcorpus logical");

    G4LogicalVolume *supportStructureLogical = new G4LogicalVolume(supStructureSolid,
                                                                   m_data->getMaterial("NoOptic_Absorber"),
                                                                   "TubeHolder logical");

    G4LogicalVolume *glassLogical = new G4LogicalVolume(glassSolid,
                                                        m_data->getMaterial("RiAbs_Glass_Vitrovex"),
                                                        "Glass_log");
    if (m_placeHarness)
    {
        gelLogical = new G4LogicalVolume(substractHarnessPlug(gelSolid),
                                         m_data->getMaterial("RiAbs_Gel_Shin-Etsu"),
                                         "Gelcorpus logical");
        supportStructureLogical = new G4LogicalVolume(substractHarnessPlug(supStructureSolid),
                                                      m_data->getMaterial("NoOptic_Absorber"),
                                                      "TubeHolder logical");
        glassLogical = new G4LogicalVolume(substractHarnessPlug(glassSolid),
                                           m_data->getMaterial("RiAbs_Glass_Vitrovex"),
                                           "Glass_log");
    }
    G4LogicalVolume *reflectorPolarLogical = new G4LogicalVolume(reflectorPolarSolid,
                                                                 m_data->getMaterial("NoOptic_Reflector"),
                                                                 "RefConeType1 logical");

    G4LogicalVolume *reflectorEqUpCutLogical = new G4LogicalVolume(reflectorEqUpCutSolid,
                                                                   m_data->getMaterial("NoOptic_Reflector"),
                                                                   "RefConeType2 ETEL logical");

    G4LogicalVolume *reflectorEqLowCutLogical = new G4LogicalVolume(reflectorEqLowCutSolid,
                                                                    m_data->getMaterial("NoOptic_Reflector"),
                                                                    "RefConeType3 ETEL logical");

    // Placements
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), supportStructureLogical, "SupportStructure_physical", gelLogical, false, 0, m_checkOverlaps);

    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), gelLogical, "Gel_physical", glassLogical, false, 0, m_checkOverlaps);

    G4Transform3D transformers;
    std::stringstream converter;
    for (int k = 0; k <= m_totalNumberPMTs - 1; k++)
    {
        converter.str("");
        converter << "_" << k;

        transformers = G4Transform3D(m_PMTRotations[k], m_positionsPMT[k]);
        m_managerPMT->placeIt(transformers, gelLogical, converter.str());

        // Placing reflective cones:
        converter.str("");
        converter << "RefCone_" << k << "_physical";
        transformers = G4Transform3D(m_PMTRotations[k], m_reflectorPositions[k]);
        if (k >= m_numberPolarPMTs && k <= m_numberPolarPMTs + m_numberEqPMTs - 1)
        {
            transformers = G4Transform3D(m_PMTRotPhi[k], G4ThreeVector(0, 0, 0));
            new G4PVPlacement(transformers, reflectorEqUpCutLogical, converter.str(), gelLogical, false, 0, m_checkOverlaps);
        }
        else if (k >= m_numberPolarPMTs + m_numberEqPMTs && k <= m_numberPolarPMTs + m_numberEqPMTs * 2 - 1)
        {
            transformers = G4Transform3D(m_PMTRotPhi[k], G4ThreeVector(0, 0, 0));
            new G4PVPlacement(transformers, reflectorEqLowCutLogical, converter.str(), gelLogical, false, 0, m_checkOverlaps);
        }
        else
        {
            new G4PVPlacement(transformers, reflectorPolarLogical, converter.str(), gelLogical, false, 0, m_checkOverlaps);
        }
    }

    // Place LED
    for (int k = 0; k <= m_NrTotalLED - 1; k++)
    {
        converter.str("");
        converter << "LEDhole_physical_" << k;
        m_flashers->placeIt(m_LEDTransformers[k], gelLogical, converter.str());
    }

    // ------------------ Add outer shape solid to MultiUnion in case you need substraction -------------------------------------------

    appendComponent(glassSolid, glassLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "PressureVessel_" + std::to_string(m_index));
    // ------------------- optical border surfaces --------------------------------------------------------------------------------
    new G4LogicalSkinSurface("RefCone_skin", reflectorPolarLogical,
                             m_data->getOpticalSurface("Surf_V95Gel"));
    new G4LogicalSkinSurface("RefCone_skin", reflectorEqUpCutLogical,
                             m_data->getOpticalSurface("Surf_V95Gel"));
    new G4LogicalSkinSurface("RefCone_skin", reflectorEqLowCutLogical,
                             m_data->getOpticalSurface("Surf_V95Gel"));

    //     // ---------------- visualisation attributes --------------------------------------------------------------------------------
    glassLogical->SetVisAttributes(m_glassVis);
    gelLogical->SetVisAttributes(m_gelVis);

    if (!OMSimCommandArgsTable::getInstance().get<bool>("simple_PMT") && OMSimCommandArgsTable::getInstance().get<bool>("visual"))
    {
        log_warning("PMT shape too complicated for visualiser, and support structure can't be visualised. Use simple_PMT or check {} if you want to try with another visualiser!", Tools::visualisationURL);
        supportStructureLogical->SetVisAttributes(m_invisibleVis);
    }
    else
    {
        supportStructureLogical->SetVisAttributes(m_absorberVis);
    }
    reflectorPolarLogical->SetVisAttributes(m_aluVis);
    reflectorEqUpCutLogical->SetVisAttributes(m_aluVis);
    reflectorEqLowCutLogical->SetVisAttributes(m_aluVis);
}

G4UnionSolid *mDOM::pressureVessel(const G4double p_outRad, G4String p_sufix)
{
    G4Ellipsoid *topSolid = new G4Ellipsoid("SphereTop solid" + p_sufix, p_outRad, p_outRad, p_outRad, -5 * mm, p_outRad + 5 * mm);
    G4Ellipsoid *bottomSolid = new G4Ellipsoid("SphereBottom solid" + p_sufix, p_outRad, p_outRad, p_outRad, -(p_outRad + 5 * mm), 5 * mm);

    G4double zCorners[] = {m_cylinderHeight * 1.001, m_cylinderHeight, 0, -m_cylinderHeight, -m_cylinderHeight * 1.001};
    G4double rCorners[] = {0, p_outRad, p_outRad + m_cylinderHeight * sin(m_cylinderAngle), p_outRad, 0};
    G4Polycone *cylinderSolid = new G4Polycone("Cylinder solid" + p_sufix, 0, 2 * CLHEP::pi, 5, rCorners, zCorners);

    G4UnionSolid *temporalUnion = new G4UnionSolid("temp" + p_sufix, cylinderSolid, topSolid, 0, G4ThreeVector(0, 0, m_cylinderHeight));
    G4UnionSolid *unionSolid = new G4UnionSolid("OM body" + p_sufix, temporalUnion, bottomSolid, 0, G4ThreeVector(0, 0, -m_cylinderHeight));
    return unionSolid;
}

std::tuple<G4SubtractionSolid *, G4UnionSolid *> mDOM::supportStructure()
{
    G4VSolid *solidPMT = m_managerPMT->getPMTSolid();

    //  PMT support structure primitives & cutting "nests" for PMTs later

    G4Ellipsoid *supStructureTopSolid = new G4Ellipsoid("FoamSphereTop solid", m_supStructureRad, m_supStructureRad, m_supStructureRad, -5 * mm, m_supStructureRad + 5 * mm);
    G4Ellipsoid *supStructureBottomSolid = new G4Ellipsoid("FoamSphereBottom solid", m_supStructureRad, m_supStructureRad, m_supStructureRad, -(m_supStructureRad + 5 * mm), 5 * mm);
    G4Tubs *supStructureCylinderSolid = new G4Tubs("FoamCylinder solid", 0, m_glassOutRad - m_glassThick - m_gelThickness, m_cylinderHeight, 0, 2 * CLHEP::pi);
    G4UnionSolid *supStructureTemporalUnion = new G4UnionSolid("Foam TempUnion solid", supStructureCylinderSolid, supStructureTopSolid, 0, G4ThreeVector(0, 0, (m_cylinderHeight)));
    G4UnionSolid *supStructureTemporalSolid = new G4UnionSolid("Foam solid", supStructureTemporalUnion, supStructureBottomSolid, 0, G4ThreeVector(0, 0, -(m_cylinderHeight)));

    // Reflectors nests for support structure substraction
    G4double reflectorOuterNegRad = m_refConeIdealInRad + m_reflectorConeToHolder / cos(mRefConeAngle);
    G4double reflectorOuterPosRad = m_refConeIdealInRad + m_reflectorConeToHolder / cos(mRefConeAngle) + 2 * 1.5 * m_reflectorHalfZ * tan(mRefConeAngle);
    G4Cons *reflectorNestConeSolid = new G4Cons("RefConeNestCone", 0, reflectorOuterNegRad, 0, reflectorOuterPosRad, 1.5 * m_reflectorHalfZ, 0, 2 * CLHEP::pi);
    G4UnionSolid *reflectorNestSolid = new G4UnionSolid("RefConeNest", solidPMT, reflectorNestConeSolid, 0, G4ThreeVector(0, 0, 1.5 * m_reflectorHalfZ));

    // Support structure substraction
    G4SubtractionSolid *supStructureSolid;
    G4Transform3D transformers;
    for (int k = 0; k <= m_totalNumberPMTs - 1; k++)
    {
        transformers = G4Transform3D(m_PMTRotations[k], m_positionsPMT[k]);
        if (k == 0)
            supStructureSolid = new G4SubtractionSolid("TubeHolder solid", supStructureTemporalSolid, reflectorNestSolid, transformers);
        else
            supStructureSolid = new G4SubtractionSolid("TubeHolder solid", supStructureSolid, reflectorNestSolid, transformers);
    }
    return std::make_tuple(supStructureSolid, supStructureTemporalSolid);
}

G4SubtractionSolid *mDOM::equatorialReflector(G4VSolid *p_supportStructure, G4Cons *p_reflector, G4double p_angle, G4String p_sufix)
{
    G4double reflectorR = m_glassInRad - m_gelThicknessFrontPMT - m_PMToffset + m_reflectorHalfZ + m_EqPMTrOffset;
    G4double rho = reflectorR * sin(p_angle);
    G4double x = rho * cos(0 * deg);
    G4double y = rho * sin(0 * deg);
    G4int sign;
    if (p_angle == m_thetaEquatorial)
        sign = 1; // if upper reflector
    else
        sign = -1;
    G4double z = reflectorR * cos(p_angle) + sign * (m_cylinderHeight - m_EqPMTzOffset);

    G4RotationMatrix *rot = new G4RotationMatrix();
    rot->rotateY(p_angle);
    G4Transform3D transformers = G4Transform3D(*rot, G4ThreeVector(x, y, z));
    G4IntersectionSolid *uncutReflector = new G4IntersectionSolid("RefConeType2_" + p_sufix, p_supportStructure, p_reflector, transformers);

    // Start cutting process
    G4Cons *reflectorCutterSolid = new G4Cons("RefConeCutter" + p_sufix,
                                              0, m_refConeIdealInRad + m_reflectorConeSheetThickness * std::sqrt(2.) + 2 * mm,
                                              0, m_refConeIdealInRad + m_reflectorConeSheetThickness * std::sqrt(2.) + 2 * m_reflectorHalfZ + 2 * mm,
                                              m_reflectorHalfZ, 0, 2 * CLHEP::pi);

    G4double xCut = rho * cos(360. * deg / m_numberEqPMTs);
    G4double yCut = rho * sin(360. * deg / m_numberEqPMTs);

    rot = new G4RotationMatrix();
    rot->rotateY(p_angle);
    rot->rotateZ(360. * deg / m_numberEqPMTs);
    transformers = G4Transform3D(*rot, G4ThreeVector(xCut, yCut, z));
    G4SubtractionSolid *reflectorCutSolid = new G4SubtractionSolid("RefConeEqUpCut" + p_sufix,
                                                                   uncutReflector,
                                                                   reflectorCutterSolid,
                                                                   transformers);

    rot = new G4RotationMatrix();
    rot->rotateY(p_angle);
    rot->rotateZ(-360. * deg / m_numberEqPMTs);
    transformers = G4Transform3D(*rot, G4ThreeVector(xCut, -yCut, z));
    reflectorCutSolid = new G4SubtractionSolid("RefConeEqLoCut" + p_sufix, reflectorCutSolid, reflectorCutterSolid, transformers);

    return reflectorCutSolid;
}

G4SubtractionSolid *mDOM::substractHarnessPlug(G4VSolid *p_solid)
{
    Component plug = m_harness->getComponent("Plug");
    G4Transform3D plugTransform = G4Transform3D(plug.Rotation, plug.Position);
    G4SubtractionSolid *solidSubstracted = new G4SubtractionSolid(p_solid->GetName() + "_plugSubstracted", p_solid, plug.VSolid, plugTransform);
    return solidSubstracted;
}

G4SubtractionSolid *mDOM::substractFlashers(G4VSolid *p_supStructureSolid)
{

    G4SubtractionSolid *supStructureSolid;

    // cut in holding structure to place afterwards LEDs inside
    // NOTE: This should not be necessary (substraction with lAirSolid should be enough), but somehow a visual glitch is visible otherwise
    G4Cons *cutTubeHolderInnerSolid = new G4Cons("cut inner solid", 0, 2.65 * mm, 0, 4.288 * mm, 4.4 * mm, 0, 2 * CLHEP::pi);                                          // inner cone
    G4Tubs *cutTubeHolderOuterSolid = new G4Tubs("cut outer solid", 0, 7.25 * mm, 2 * 0.667 * mm, 0, 2 * CLHEP::pi);                                                   // outer cone
    G4UnionSolid *cutTubeHolderSolid = new G4UnionSolid("cut solid", cutTubeHolderInnerSolid, cutTubeHolderOuterSolid, 0, G4ThreeVector(0, 0, 4.4 * mm + 0.667 * mm)); // union

    // subtraction holding structure
    for (int k = 0; k <= m_NrTotalLED - 1; k++)
    {
        if (k == 0)
        {
            supStructureSolid = new G4SubtractionSolid("TubeHolderSub solid", p_supStructureSolid, cutTubeHolderSolid, m_LEDTransformers[k]);
        }
        else
        {
            supStructureSolid = new G4SubtractionSolid("TubeHolderSub solid", supStructureSolid, cutTubeHolderSolid, m_LEDTransformers[k]);
        }
    }

    return supStructureSolid;
}

void mDOM::setPMTPositions()
{
    G4double rPMT;       // radius for PMT positioning
    G4double rReflector; // radius for RefCone positioning
    G4double zOffsetPMT;
    G4RotationMatrix rot;
    G4RotationMatrix rot2;
    G4double thetaPMT;
    G4double phiPMT;

    G4double rBasePMT = m_glassInRad - m_gelThicknessFrontPMT - m_PMToffset;
    G4double reflectorConeRBase = m_glassInRad - m_gelThicknessFrontPMT - m_PMToffset + m_reflectorHalfZ;

    std::vector<G4double> thetaPMTlist = {m_thetaPolar, m_thetaEquatorial, 180. * deg - m_thetaEquatorial, 180. * deg - m_thetaPolar};
    std::vector<G4double> zOffsetPMTlist = {m_cylinderHeight, m_cylinderHeight - m_EqPMTzOffset, -m_cylinderHeight + m_EqPMTzOffset, -m_cylinderHeight};
    std::vector<int> countPMTlist = {m_numberPolarPMTs, m_numberEqPMTs, m_numberEqPMTs, m_numberPolarPMTs};
    std::vector<G4double> phaseShiftList = {0., 0.5, 0.5, 0.};

    for (int j = 0; j < 4; j++)
    {
        thetaPMT = thetaPMTlist[j];
        zOffsetPMT = zOffsetPMTlist[j];
        rPMT = rBasePMT;
        rReflector = reflectorConeRBase;
        if (j >= 1 && j <= 2)
        {
            rPMT += m_EqPMTrOffset;
            rReflector += m_EqPMTrOffset;
        }

        for (int i = 0; i < countPMTlist[j]; i++)
        {
            rot = G4RotationMatrix();
            rot2 = G4RotationMatrix();
            // lPMTphi = (i + 0.5 * (j % 2)) * 360. * deg / lPMTCountList[j];
            phiPMT = (i + phaseShiftList[j]) * 360. * deg / countPMTlist[j];

            G4double lPMTrho = rPMT * sin(thetaPMT);
            m_positionsPMT.push_back(G4ThreeVector(lPMTrho * cos(phiPMT), lPMTrho * sin(phiPMT), rPMT * cos(thetaPMT) + zOffsetPMT));

            rot.rotateY(thetaPMT);
            rot.rotateZ(phiPMT);
            rot2.rotateZ(phiPMT);
            m_PMTRotations.push_back(rot);
            m_PMTRotPhi.push_back(rot2);

            G4double lRefConeRho = rReflector * sin(thetaPMT);
            m_reflectorPositions.push_back(G4ThreeVector(lRefConeRho * cos(phiPMT), lRefConeRho * sin(phiPMT), rReflector * cos(thetaPMT) + zOffsetPMT));
        }
    }
}

void mDOM::setLEDPositions()
{
    G4int numberPolarLED = 1;
    G4int numberEquatorialLED = 4;
    m_NrTotalLED = (numberPolarLED + numberEquatorialLED) * 2; // 10 LEDs in total
    G4double rLED = 152.8 * mm;
    G4double zOffsetLED;
    G4double thetaLED;
    G4double phiLED;
    G4ThreeVector positionLED;
    G4RotationMatrix *rotationLED;
    G4Transform3D transformers;

    m_LEDAngFromSphere.resize(m_NrTotalLED);

    std::vector<G4double> thetaLEDlist = {m_thetaPolLED, m_thetaEqLED, 180. * deg - m_thetaEqLED, 180. * deg - m_thetaPolLED};
    std::vector<G4double> zOffsetLEDlist = {m_cylinderHeight, m_cylinderHeight, -m_cylinderHeight, -m_cylinderHeight};
    std::vector<int> countLEDlist = {numberPolarLED, numberEquatorialLED, numberEquatorialLED, numberPolarLED};
    std::vector<G4double> phaseShiftList = {-0.25, 0., 0., 0.};

    int flasherCounter = 0;
    for (int j = 0; j < 4; j++)
    {
        thetaLED = thetaLEDlist[j];
        zOffsetLED = zOffsetLEDlist[j];

        for (int i = 0; i < countLEDlist[j]; i++)
        {
            phiLED = m_polarEquatorialPMTphiPhase + (i + phaseShiftList[j]) * 360. * deg / countLEDlist[j];

            G4double lLEDrho = rLED * sin(thetaLED);
            positionLED = G4ThreeVector(lLEDrho * cos(phiLED), lLEDrho * sin(phiLED), rLED * cos(thetaLED) + zOffsetLED);

            (m_LEDAngFromSphere.at(flasherCounter)).resize(3);
            (m_LEDAngFromSphere.at(flasherCounter)).at(0) = lLEDrho;
            (m_LEDAngFromSphere.at(flasherCounter)).at(1) = thetaLED;
            (m_LEDAngFromSphere.at(flasherCounter)).at(2) = phiLED;

            rotationLED = new G4RotationMatrix();
            rotationLED->rotateY(thetaLED);
            rotationLED->rotateZ(phiLED);

            transformers = G4Transform3D(*rotationLED, positionLED);
            m_LEDTransformers.push_back(transformers);
            flasherCounter++;
        }
    }
}
