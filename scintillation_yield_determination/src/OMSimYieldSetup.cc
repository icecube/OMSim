
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