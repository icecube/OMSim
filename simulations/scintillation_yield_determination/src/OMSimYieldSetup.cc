
#include "OMSimYieldSetup.hh"
#include <G4Box.hh>
#include <G4ExtrudedSolid.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Tubs.hh>

Cs137Source::Cs137Source()
    : OMSimDetectorComponent()
{
    construction();
}

void Cs137Source::construction()
{
    G4Tubs *solidCs =
        new G4Tubs("Cs solid", 0, 3 * mm, 2 * mm, 0, 2 * CLHEP::pi);
    G4Tubs *solidCover =
        new G4Tubs("Cover solid", 0, 3.5 * mm, 2.5 * mm, 0, 2 * CLHEP::pi);
    G4LogicalVolume *glassCs = new G4LogicalVolume(
        solidCs, m_data->getMaterial("Ri_Glass_Tube"), "Cs_logical");
    G4LogicalVolume *logicalCover = new G4LogicalVolume(
        solidCover, m_data->getMaterial("NoOptic_Stahl"), "Cover_logical");
    new G4LogicalSkinSurface(
        "RefCone_skin", logicalCover,
        m_data->getOpticalSurface("Surf_StainlessSteelGround"));

    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), glassCs, "Cs137",
                      logicalCover, false, 0, m_checkOverlaps);

    appendComponent(solidCover, logicalCover, G4ThreeVector(0, 0, 0),
                    G4RotationMatrix(), "SourceCover");
}

//////////////////////////////////////////////////////////////////////////////////////////

OkamotoLargeSample::OkamotoLargeSample()
    : OMSimDetectorComponent(), m_meanThickness(0)
{
    construction();
}

void OkamotoLargeSample::construction()
{
    const G4double AA = 10.06 * mm;
    const G4double BB = 10.04 * mm;
    const G4double CC = 10.04 * mm;
    const G4double DD = 10.05 * mm;
    m_meanThickness = (AA + BB + CC + DD) / 4;

    const G4double AB = 48.95 * mm;
    const G4double CD = 50.70 * mm;
    G4double lY0 = (AB + CD) / 2;
    lY0 /= 2;

    const G4double lAC = 50.71 * mm;
    const G4double lBD = 49.88 * mm;
    G4double lX0 = (lAC + lBD) / 2;
    lX0 /= 2;

    std::vector<G4TwoVector> lPolygon{{-lX0, -lY0},
                                      {lAC - lX0, -lY0},
                                      {lBD - lX0, AB - lY0},
                                      {lAC - lBD - lX0, CD - lY0}};

    std::vector<G4ExtrudedSolid::ZSection> lZsections{
        {0 / 2, {0, 0}, 1}, {m_meanThickness, {0, 0}, 1}};

    G4ExtrudedSolid *lGlassSampleSolid =
        new G4ExtrudedSolid("OkamotoSampleLarge_solid", lPolygon, lZsections);
    G4LogicalVolume *lGlassSampleLogical = new G4LogicalVolume(
        lGlassSampleSolid, m_data->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"),
        "OkamotoSampleLarge_logical");
    appendComponent(lGlassSampleSolid, lGlassSampleLogical,
                    G4ThreeVector(0, 0, 0), G4RotationMatrix(),
                    "OkamotoSampleLarge");
}

G4double OkamotoLargeSample::getSampleThickness()
{
    return m_meanThickness;
}

//////////////////////////////////////////////////////////////////////////////////////////

Am241Source::Am241Source()
    : OMSimDetectorComponent()
{
    construction();
}

void Am241Source::construction()
{

    G4double SourceRing_thick = 3. / 2 * mm, SourceRing_inner = 25.4 / 2 * mm, SourceRing_outer = 30.0 / 2 * mm;
    G4Tubs *SourceRing_solidvol = new G4Tubs("SourceRing_solid", SourceRing_inner, SourceRing_outer, SourceRing_thick, 0., 2 * CLHEP::pi);
    G4Tubs *lEmitterSolid = new G4Tubs("Am241Emitter_solid", 0, SourceRing_inner-0.001*mm, 0.001*mm, 0., 2 * CLHEP::pi);

    G4LogicalVolume *SourceRing_logicvol = new G4LogicalVolume(SourceRing_solidvol, m_data->getMaterial("NoOptic_Stahl"), "SourceRing_logic");
    G4LogicalVolume *lEmitterLogical = new G4LogicalVolume(lEmitterSolid, m_data->getMaterial("Ri_Vacuum"), "Am241Emitter_logic");

    G4Tubs *SourceBack_solidvol = new G4Tubs("SourceBack_solid", 0 * cm, SourceRing_outer, SourceRing_thick, 0. * deg, 360. * deg);
    G4LogicalVolume *SourceBack_logicvol = new G4LogicalVolume(SourceBack_solidvol, m_data->getMaterial("NoOptic_Stahl"), "SourceBack_logic");
    
    // new G4LogicalSkinSurface(
    //     "RefCone_skin", SourceBack_logicvol,
    //     m_data->getOpticalSurface("Surf_StainlessSteelGround"));
    // new G4LogicalSkinSurface(
    //     "RefCone_skin", SourceRing_logicvol,
    //     m_data->getOpticalSurface("Surf_StainlessSteelGround"));

    appendComponent(SourceBack_solidvol, SourceBack_logicvol, G4ThreeVector(0, 0, 3./2*mm*3+0.001*mm),
                    G4RotationMatrix(), "SourceBack");

    appendComponent(SourceRing_solidvol, SourceRing_logicvol, G4ThreeVector(0, 0, 3./2*mm),
                    G4RotationMatrix(), "SourceRing");

    appendComponent(lEmitterSolid, lEmitterLogical, G4ThreeVector(0, 0, 2*3./2*mm),
                    G4RotationMatrix(), "Emitter");
}

//////////////////////////////////////////////////////////////////////////////////////////

OkamotoSmallSample::OkamotoSmallSample()
    : OMSimDetectorComponent(), m_meanThickness(0)
{
    construction();
}

void OkamotoSmallSample::construction()
{
    const G4double AA = 2.1 * mm;
    const G4double BB = 2.1 * mm;
    const G4double CC = 2.1 * mm;
    const G4double DD = 2.0 * mm;
    m_meanThickness = (AA + BB + CC + DD) / 4;

    const G4double AB = 31.35 * mm;
    const G4double CD = 31.2 * mm;
    G4double lY0 = (AB + CD) / 2;
    lY0 /= 2;

    const G4double lAC = 31.9 * mm;
    const G4double lBD = 32.0 * mm;
    G4double lX0 = (lAC + lBD) / 2;
    lX0 /= 2;

    std::vector<G4TwoVector> lPolygon{{-lX0, -lY0},
                                      {lAC - lX0, -lY0},
                                      {lBD - lX0, AB - lY0},
                                      {lAC - lBD - lX0, CD - lY0}};

    std::vector<G4ExtrudedSolid::ZSection> lZsections{
        {0 / 2, {0, 0}, 1}, {m_meanThickness, {0, 0}, 1}};

    G4ExtrudedSolid *lGlassSampleSolid =
        new G4ExtrudedSolid("OkamotoSampleSmall_solid", lPolygon, lZsections);
    G4LogicalVolume *lGlassSampleLogical = new G4LogicalVolume(
        lGlassSampleSolid, m_data->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"),
        "OkamotoSampleSmall_logical");
    appendComponent(lGlassSampleSolid, lGlassSampleLogical,
                    G4ThreeVector(0, 0, 0), G4RotationMatrix(),
                    "OkamotoSampleSmall");
}

G4double OkamotoSmallSample::getSampleThickness()
{
    return m_meanThickness;
}


//////////////////////////////////////////////////////////////////////////////////////////

AMETEKSiliconDetector::AMETEKSiliconDetector()
    : OMSimDetectorComponent()
{
    construction();
}

void AMETEKSiliconDetector::construction()
{
    G4Tubs *RingSolid = new G4Tubs("RingSolid", 0.5*19.5*mm, 31.6*0.5*mm, 0.5*1.1*mm, 0. * deg, 360. * deg);
    G4Tubs *DetectorSolid = new G4Tubs("DetectorSolid", 0 * cm, 0.5*19.5*mm, 1*mm*0.5, 0. * deg, 360. * deg);
    G4LogicalVolume *RingLog = new G4LogicalVolume(RingSolid, m_data->getMaterial("NoOptic_Stahl"), "RingLog");
    G4LogicalVolume *DetectorLog = new G4LogicalVolume(DetectorSolid, m_data->getMaterial("RiAbs_Absorber"), "DetectorLog");


    appendComponent(RingSolid, RingLog, G4ThreeVector(0, 0, -0.5*1.1*mm),
                    G4RotationMatrix(), "RingDetector");

    appendComponent(DetectorSolid, DetectorLog, G4ThreeVector(0, 0, -0.5*1.1*mm-1*mm-0.0001*mm),
                    G4RotationMatrix(), "Detector");

}
