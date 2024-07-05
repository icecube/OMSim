#include "OMSimYieldDetector.hh"
#include "OMSimYieldSetup.hh"
#include "OMSimMDOM.hh"
#include "OMSimPDOM.hh"
#include "OMSimLOM16.hh"
#include "OMSimLOM18.hh"
#include "OMSimDEGG.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimSensitiveDetector.hh"

/**
 * @brief Constructs the world volume (sphere).
 */
void OMSimYieldDetector::constructWorld()
{
    mWorldSolid = new G4Orb("World", OMSimCommandArgsTable::getInstance().get<G4double>("world_radius") * m);
    mWorldLogical = new G4LogicalVolume(mWorldSolid, mData->getMaterial("argWorld"), "World_log", 0, 0, 0);
    mWorldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), mWorldLogical, "World_phys", 0, false, 0);
    G4VisAttributes *World_vis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.));
    mWorldLogical->SetVisAttributes(World_vis);
}

/**
 * @brief Constructs the selected detector from the command line argument.
 */
void OMSimYieldDetector::constructDetector()
{
    OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
    bool lPlaceHarness = lArgs.get<bool>("place_harness");

    OMSimOpticalModule *lOpticalModule = nullptr;

    switch (lArgs.get<G4int>("detector_type"))
    {

    case 0:
    {
        log_info("Constructing Okamoto Cs-137 Source setup for electron yield");
        G4double lZrandom = 0;
        G4double lYrandom = 0;
        G4double lZrandomSource = 0;
        G4double lYrandomSource = 0;
        G4double lXSource = 0;
        G4double lXSample = 0;
        G4double lPMTrotY = 1*deg;
        G4double lPMTrotX = 0*deg;
        if (lArgs.get<bool>("systematics"))
        {
            lZrandom = G4RandGauss::shoot(0, 0.02) * cm;
            lYrandom = G4RandGauss::shoot(0, 2) * mm;
            lZrandomSource = G4RandGauss::shoot(0, 0.01) * cm;
            lYrandomSource = G4RandGauss::shoot(0, 2) * mm;
            lXSource = G4RandGauss::shoot(0, 2) * mm;
            lXSample = G4RandGauss::shoot(0, 2) * mm;
            lPMTrotY = G4RandGauss::shoot(1, 0.1) * deg;
            lPMTrotX = G4RandGauss::shoot(0, 0.1) * deg;
        }

        OMSimPMTConstruction *lPMTManager = new OMSimPMTConstruction(mData);
        lPMTManager->selectPMT("argPMT");
        lPMTManager->construction();
        lPMTManager->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix().rotateY(lPMTrotY).rotateX(lPMTrotX), mWorldLogical, "_0");
        lHitManager.setNumberOfPMTs(1, 0);
        lPMTManager->configureSensitiveVolume(this, "/PMT/0");

        OkamotoLargeSample *lOkamotoSample = new OkamotoLargeSample(mData);

        G4double lzSample = 4.46 * cm + lPMTManager->getDistancePMTCenterToTip() + lZrandom;
        G4double lySample = 21.77 * mm - (40 * mm - 25 * mm)+lYrandom;

        lOkamotoSample->placeIt(G4ThreeVector(lXSample, -lySample, lzSample), G4RotationMatrix(), mWorldLogical, "");

        G4double lzSource = lzSample + lOkamotoSample->getSampleThickness() + 0.96 * cm + lZrandomSource;
        Cs137Source *lSource = new Cs137Source(mData);
        lSource->placeIt(G4ThreeVector(lXSource, -lySample+lYrandomSource, lzSource), G4RotationMatrix(), mWorldLogical, "");
        mSource = lSource;

        break;
    }
    case 1:
    {
        log_info("Constructing single PMT");
        OMSimPMTConstruction *lPMTManager = new OMSimPMTConstruction(mData);
        lPMTManager->selectPMT("argPMT");
        lPMTManager->construction();
        lPMTManager->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "_0");
        lHitManager.setNumberOfPMTs(1, 0);
        lPMTManager->configureSensitiveVolume(this, "/PMT/0");
        break;
    }
    case 2:
    {
        lOpticalModule = new mDOM(mData, lPlaceHarness);
        break;
    }
    case 3:
    {

        lOpticalModule = new pDOM(mData, lPlaceHarness);
        break;
    }
    case 4:
    {

        lOpticalModule = new LOM16(mData, lPlaceHarness);
        break;
    }
    case 5:
    {

        lOpticalModule = new LOM18(mData, lPlaceHarness);
        break;
    }
    case 6:
    {
        lOpticalModule = new DEGG(mData, lPlaceHarness);
        break;
    }
    }

    if (lOpticalModule)
    {
        lOpticalModule->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix(), mWorldLogical, "");
        lOpticalModule->configureSensitiveVolume(this);
        mOpticalModule = lOpticalModule;
    }
}
