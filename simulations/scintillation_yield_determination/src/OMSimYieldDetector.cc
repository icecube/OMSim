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
    m_worldSolid = new G4Orb("World", OMSimCommandArgsTable::getInstance().get<G4double>("world_radius") * m);
    m_worldLogical = new G4LogicalVolume(m_worldSolid, m_data->getMaterial("argWorld"), "World_log", 0, 0, 0);
    m_worldPhysical = new G4PVPlacement(0, G4ThreeVector(0., 0., 0.), m_worldLogical, "World_phys", 0, false, 0);
    G4VisAttributes *worldVis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.));
    m_worldLogical->SetVisAttributes(worldVis);
}

/**
 * @brief Constructs the selected detector from the command line argument.
 */
void OMSimYieldDetector::constructDetector()
{
    OMSimHitManager &hitManager = OMSimHitManager::getInstance();
    OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
    bool placeHarness = lArgs.get<bool>("place_harness");

    OMSimOpticalModule *OM = nullptr;

    switch (lArgs.get<G4int>("detector_type"))
    {

    case 0:
    {
        log_info("Constructing Okamoto Cs-137 Source setup for electron yield");
        G4double zRandom = 0;
        G4double yRandom = 0;
        G4double zRandomSource = 0;
        G4double yRandomSource = 0;
        G4double xSource = 0;
        G4double xSample = 0;
        G4double PMTrotY = 1*deg;
        G4double PMTrotX = 0*deg;
        if (lArgs.get<bool>("systematics"))
        {
            zRandom = G4RandGauss::shoot(0, 0.02) * cm;
            yRandom = G4RandGauss::shoot(0, 2) * mm;
            zRandomSource = G4RandGauss::shoot(0, 0.01) * cm;
            yRandomSource = G4RandGauss::shoot(0, 2) * mm;
            xSource = G4RandGauss::shoot(0, 2) * mm;
            xSample = G4RandGauss::shoot(0, 2) * mm;
            PMTrotY = G4RandGauss::shoot(1, 0.1) * deg;
            PMTrotX = G4RandGauss::shoot(0, 0.1) * deg;
        }

        OMSimPMTConstruction *pmtManager = new OMSimPMTConstruction();
        pmtManager->selectPMT("argPMT");
        pmtManager->construction();
        pmtManager->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix().rotateY(PMTrotY).rotateX(PMTrotX), m_worldLogical, "_0");
        hitManager.setNumberOfPMTs(1, 0);
        pmtManager->configureSensitiveVolume(this, "/PMT/0");

        OkamotoLargeSample *okamotoSample = new OkamotoLargeSample();

        G4double zSample = 4.46 * cm + pmtManager->getDistancePMTCenterToTip() + zRandom;
        G4double ySample = 21.77 * mm - (40 * mm - 25 * mm)+yRandom;

        okamotoSample->placeIt(G4ThreeVector(xSample, -ySample, zSample), G4RotationMatrix(), m_worldLogical, "");

        G4double zSource = zSample + okamotoSample->getSampleThickness() + 0.96 * cm + zRandomSource;
        Cs137Source *source = new Cs137Source();
        source->placeIt(G4ThreeVector(xSource, -ySample+yRandomSource, zSource), G4RotationMatrix(), m_worldLogical, "");
        m_source = source;

        break;
    }
     case 1:
    {
        log_info("Constructing Okamoto AM241 Source setup for alpha yield");
        G4double zRandom = 0;
        G4double yRandom = 0;
        G4double zRandomSource = 0;
        G4double yRandomSource = 0;
        G4double xSource = 0;
        G4double xSample = 0;
        G4double PMTrotY = 1*deg;
        G4double PMTrotX = 0*deg;
        if (lArgs.get<bool>("systematics"))
        {
            zRandom = G4RandGauss::shoot(0, 0.02) * cm;
            yRandom = G4RandGauss::shoot(0, 2) * mm;
            zRandomSource = G4RandGauss::shoot(0.1, 0.01) * mm;
            yRandomSource = G4RandGauss::shoot(0, 2) * mm;
            xSource = G4RandGauss::shoot(0, 2) * mm;
            xSample = G4RandGauss::shoot(0, 2) * mm;
            PMTrotY = G4RandGauss::shoot(1, 0.1) * deg;
            PMTrotX = G4RandGauss::shoot(0, 0.1) * deg;
        }

        OMSimPMTConstruction *pmtManager = new OMSimPMTConstruction();
        pmtManager->selectPMT("argPMT");
        pmtManager->construction();
        pmtManager->placeIt(G4ThreeVector(0, 0, 0), G4RotationMatrix().rotateY(PMTrotY).rotateX(PMTrotX), m_worldLogical, "_0");
        hitManager.setNumberOfPMTs(1, 0);
        pmtManager->configureSensitiveVolume(this, "/PMT/0");

        OkamotoSmallSample *okamotoSample = new OkamotoSmallSample();

        G4double zSample = 4.46 * cm + pmtManager->getDistancePMTCenterToTip() + zRandom;
        G4double ySample = 21.77 * mm - (40 * mm - 25 * mm)+yRandom;

        okamotoSample->placeIt(G4ThreeVector(xSample, -ySample, zSample), G4RotationMatrix(), m_worldLogical, "");

        G4double zSource = zSample + okamotoSample->getSampleThickness() + zRandomSource;
        Am241Source *source = new Am241Source();
        source->placeIt(G4ThreeVector(xSource, -ySample+yRandomSource, zSource), G4RotationMatrix(), m_worldLogical, "");
        m_source = source;

        break;
    }


     case 2:
    {
        log_info("Constructing Okamoto Cs-137 Source setup for electron yield");
        G4double z = 0.2*mm;
        G4double yRandom = 0;
        if (lArgs.get<bool>("systematics"))
        {
            yRandom = G4RandGauss::shoot(0, 1) * mm;
        }

        AMETEKSiliconDetector *detector = new AMETEKSiliconDetector();
        Am241Source *source = new Am241Source();
        source->placeIt(G4ThreeVector(0,yRandom,z), G4RotationMatrix(), m_worldLogical, "");
        detector->placeIt(G4ThreeVector(0,0,0), G4RotationMatrix(), m_worldLogical, "");

        OMSimSensitiveDetector* sensitiveDetector = new OMSimSensitiveDetector("/Si/0", DetectorType::VolumePhotonDetector);
        hitManager.setNumberOfPMTs(1, 0);
        registerSensitiveDetector(detector->getComponent("Detector").VLogical, sensitiveDetector);

        m_source = source;
        break;
    }
    }

}
