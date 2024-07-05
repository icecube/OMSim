
#include "OMSimYieldSetup.hh"
#include <G4Box.hh>
#include <G4ExtrudedSolid.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4Tubs.hh>

Cs137Source::Cs137Source(InputDataManager *pData)
    : abcDetectorComponent(pData)
{
    construction();
}

void Cs137Source::construction()
{
    G4Tubs *lCsSolid =
        new G4Tubs("Cs solid", 0, 3 * mm, 2 * mm, 0, 2 * CLHEP::pi);
    G4Tubs *lCoverSolid =
        new G4Tubs("Cover solid", 0, 3.5 * mm, 2.5 * mm, 0, 2 * CLHEP::pi);
    G4LogicalVolume *lCsglass = new G4LogicalVolume(
        lCsSolid, mData->getMaterial("Ri_Glass_Tube"), "Cs_logical");
    G4LogicalVolume *lCoverLogical = new G4LogicalVolume(
        lCoverSolid, mData->getMaterial("NoOptic_Stahl"), "Cover_logical");
    new G4LogicalSkinSurface(
        "RefCone_skin", lCoverLogical,
        mData->getOpticalSurface("Refl_StainlessSteelGround"));

    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), lCsglass, "Cs137",
                      lCoverLogical, false, 0, mCheckOverlaps);

    appendComponent(lCoverSolid, lCoverLogical, G4ThreeVector(0, 0, 0),
                    G4RotationMatrix(), "SourceCover");
}

//////////////////////////////////////////////////////////////////////////////////////////

OkamotoLargeSample::OkamotoLargeSample(InputDataManager *pData)
    : abcDetectorComponent(pData), mMeanThickness(0)
{
    construction();
}

void OkamotoLargeSample::construction()
{
    const G4double lAA = 10.06 * mm;
    const G4double lBB = 10.04 * mm;
    const G4double lCC = 10.04 * mm;
    const G4double lDD = 10.05 * mm;
    mMeanThickness = (lAA + lBB + lCC + lDD) / 4;

    const G4double lAB = 48.95 * mm;
    const G4double lCD = 50.70 * mm;
    G4double lY0 = (lAB + lCD) / 2;
    lY0 /= 2;

    const G4double lAC = 50.71 * mm;
    const G4double lBD = 49.88 * mm;
    G4double lX0 = (lAC + lBD) / 2;
    lX0 /= 2;

    std::vector<G4TwoVector> lPolygon{{-lX0, -lY0},
                                      {lAC - lX0, -lY0},
                                      {lBD - lX0, lAB - lY0},
                                      {lAC - lBD - lX0, lCD - lY0}};

    std::vector<G4ExtrudedSolid::ZSection> lZsections{
        {0 / 2, {0, 0}, 1}, {mMeanThickness, {0, 0}, 1}};

    G4ExtrudedSolid *lGlassSampleSolid =
        new G4ExtrudedSolid("OkamotoSampleLarge_solid", lPolygon, lZsections);
    G4LogicalVolume *lGlassSampleLogical = new G4LogicalVolume(
        lGlassSampleSolid, mData->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"),
        "OkamotoSampleLarge_logical");
    appendComponent(lGlassSampleSolid, lGlassSampleLogical,
                    G4ThreeVector(0, 0, 0), G4RotationMatrix(),
                    "OkamotoSampleLarge");
}

G4double OkamotoLargeSample::getSampleThickness()
{
    return mMeanThickness;
}

//////////////////////////////////////////////////////////////////////////////////////////

Am241Source::Am241Source(InputDataManager *pData)
    : abcDetectorComponent(pData)
{
    construction();
}

void Am241Source::construction()
{

    G4double SourceRing_thick = 3. / 2 * mm, SourceRing_inner = 25.4 / 2 * mm, SourceRing_outer = 30.0 / 2 * mm;
    G4Tubs *SourceRing_solidvol = new G4Tubs("SourceRing_solid", SourceRing_inner, SourceRing_outer, SourceRing_thick, 0., 2 * CLHEP::pi);
    G4Tubs *lEmitterSolid = new G4Tubs("Am241Emitter_solid", 0, SourceRing_inner-0.001*mm, 0.001*mm, 0., 2 * CLHEP::pi);

    G4LogicalVolume *SourceRing_logicvol = new G4LogicalVolume(SourceRing_solidvol, mData->getMaterial("NoOptic_Stahl"), "SourceRing_logic");
    G4LogicalVolume *lEmitterLogical = new G4LogicalVolume(lEmitterSolid, mData->getMaterial("Ri_Vacuum"), "Am241Emitter_logic");

    G4Tubs *SourceBack_solidvol = new G4Tubs("SourceBack_solid", 0 * cm, SourceRing_outer, SourceRing_thick, 0. * deg, 360. * deg);
    G4LogicalVolume *SourceBack_logicvol = new G4LogicalVolume(SourceBack_solidvol, mData->getMaterial("NoOptic_Stahl"), "SourceBack_logic");
    
    // new G4LogicalSkinSurface(
    //     "RefCone_skin", SourceBack_logicvol,
    //     mData->getOpticalSurface("Refl_StainlessSteelGround"));
    // new G4LogicalSkinSurface(
    //     "RefCone_skin", SourceRing_logicvol,
    //     mData->getOpticalSurface("Refl_StainlessSteelGround"));

    appendComponent(SourceBack_solidvol, SourceBack_logicvol, G4ThreeVector(0, 0, 3./2*mm*3+0.001*mm),
                    G4RotationMatrix(), "SourceBack");

    appendComponent(SourceRing_solidvol, SourceRing_logicvol, G4ThreeVector(0, 0, 3./2*mm),
                    G4RotationMatrix(), "SourceRing");

    appendComponent(lEmitterSolid, lEmitterLogical, G4ThreeVector(0, 0, 2*3./2*mm),
                    G4RotationMatrix(), "Emitter");
}

//////////////////////////////////////////////////////////////////////////////////////////

OkamotoSmallSample::OkamotoSmallSample(InputDataManager *pData)
    : abcDetectorComponent(pData), mMeanThickness(0)
{
    construction();
}

void OkamotoSmallSample::construction()
{
    const G4double lAA = 2.1 * mm;
    const G4double lBB = 2.1 * mm;
    const G4double lCC = 2.1 * mm;
    const G4double lDD = 2.0 * mm;
    mMeanThickness = (lAA + lBB + lCC + lDD) / 4;

    const G4double lAB = 31.35 * mm;
    const G4double lCD = 31.2 * mm;
    G4double lY0 = (lAB + lCD) / 2;
    lY0 /= 2;

    const G4double lAC = 31.9 * mm;
    const G4double lBD = 32.0 * mm;
    G4double lX0 = (lAC + lBD) / 2;
    lX0 /= 2;

    std::vector<G4TwoVector> lPolygon{{-lX0, -lY0},
                                      {lAC - lX0, -lY0},
                                      {lBD - lX0, lAB - lY0},
                                      {lAC - lBD - lX0, lCD - lY0}};

    std::vector<G4ExtrudedSolid::ZSection> lZsections{
        {0 / 2, {0, 0}, 1}, {mMeanThickness, {0, 0}, 1}};

    G4ExtrudedSolid *lGlassSampleSolid =
        new G4ExtrudedSolid("OkamotoSampleSmall_solid", lPolygon, lZsections);
    G4LogicalVolume *lGlassSampleLogical = new G4LogicalVolume(
        lGlassSampleSolid, mData->getMaterial("RiAbs_Glass_Okamoto_DOUMEKI"),
        "OkamotoSampleSmall_logical");
    appendComponent(lGlassSampleSolid, lGlassSampleLogical,
                    G4ThreeVector(0, 0, 0), G4RotationMatrix(),
                    "OkamotoSampleSmall");
}

G4double OkamotoSmallSample::getSampleThickness()
{
    return mMeanThickness;
}


//////////////////////////////////////////////////////////////////////////////////////////

AMETEKSiliconDetector::AMETEKSiliconDetector(InputDataManager *pData)
    : abcDetectorComponent(pData)
{
    construction();
}

void AMETEKSiliconDetector::construction()
{
    G4Tubs *RingSolid = new G4Tubs("RingSolid", 0.5*19.5*mm, 31.6*0.5*mm, 0.5*1.1*mm, 0. * deg, 360. * deg);
    G4Tubs *DetectorSolid = new G4Tubs("DetectorSolid", 0 * cm, 0.5*19.5*mm, 1*mm*0.5, 0. * deg, 360. * deg);
    G4LogicalVolume *RingLog = new G4LogicalVolume(RingSolid, mData->getMaterial("NoOptic_Stahl"), "RingLog");
    G4LogicalVolume *DetectorLog = new G4LogicalVolume(DetectorSolid, mData->getMaterial("RiAbs_Absorber"), "DetectorLog");


    appendComponent(RingSolid, RingLog, G4ThreeVector(0, 0, -0.5*1.1*mm),
                    G4RotationMatrix(), "RingDetector");

    appendComponent(DetectorSolid, DetectorLog, G4ThreeVector(0, 0, -0.5*1.1*mm-1*mm-0.0001*mm),
                    G4RotationMatrix(), "Detector");

}
