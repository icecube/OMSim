/** @file OMSimMDOM.hh
 *  @brief Construction of mDOM.
 *  @ingroup common
 */
#pragma once
#include "OMSimMDOMFlasher.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

class mDOMHarness;

class mDOM : public OMSimOpticalModule
{

public:
    mDOM(G4bool p_placeHarness = true);
    ~mDOM();
    void construction();
    double getPressureVesselWeight() { return 13.0 * kg; };
    int getNumberOfPMTs() { return m_totalNumberPMTs; };
    G4String getName()
    {
        std::stringstream ss;
        ss << "/mDOM/" << m_index;
        return ss.str();
    }

    G4int m_NrTotalLED;
    std::vector<G4Transform3D> m_LEDTransformers;           ///< coordinates from center of the module
    std::vector<std::vector<G4double>> m_LEDAngFromSphere; ///< stores rho (mm),theta (deg),phi (deg) of each LED from the center of its corresponding spherical part. Useful to run the particles.

    /**
     * @brief run/beamOn the specified flasher.
     * @details This method triggers the flasher at the given index in the specified module.
     * @param pModuleIndex The index of the module to be flashed (if only one mDOM placed, then 0, otherwise depending on placeIt() order)
     * @param pLEDIndex The index of the flasher within the module.
     */
    void runBeamOnFlasher(G4int pModuleIndex, G4int pLEDIndex) { m_flashers->runBeamOnFlasher(this, pModuleIndex, pLEDIndex); }

private:
    mDOMFlasher *m_flashers;
    mDOMHarness *m_harness;
    G4SubtractionSolid *equatorialReflector(G4VSolid *pSupportStructure, G4Cons *pReflCone, G4double pAngle, G4String pSuffix);
    void setPMTPositions();
    G4UnionSolid *pressureVessel(const G4double pOutRad, G4String pSuffix);
    G4SubtractionSolid *substractHarnessPlug(G4VSolid *pSolid);
    std::tuple<G4SubtractionSolid *, G4UnionSolid *> supportStructure();
    G4SubtractionSolid *substractFlashers(G4VSolid *lSupStructureSolid);
    void setLEDPositions();

    G4bool m_placeHarness = true;
    G4bool m_harnessUnion = true; // it should be true for the first module that you build, and then false
    std::vector<G4ThreeVector> m_positionsPMT;
    std::vector<G4RotationMatrix> m_PMTRotations;
    std::vector<G4RotationMatrix> m_PMTRotPhi;
    std::vector<G4ThreeVector> m_reflectorPositions;

    G4double m_PMToffset;
    G4double m_refConeIdealInRad;
    const G4double m_glassThick = 13.5 * mm; ///< maximum Glass thickness

public:
    const G4double m_cylinderAngle = 2.8 * deg; ///< Deviation angle of cylindrical part of the pressure vessel
    const G4double m_glassOutRad = 176.5 * mm;  ///< outer radius of galss cylinder (pressure vessel)
    const G4double m_cylinderHeight = 27.5 * mm;       ///< height of cylindrical part of glass half-vessel
    const G4double m_glassInRad = m_glassOutRad - m_glassThick;

private:
    const G4double m_gelThicknessFrontPMT = 3.6 * mm;  ///< distance between inner glass surface and tip of PMTs
    const G4double m_gelThickness = 4.5 * mm;          ///< distance between inner glass surface and holding structure, filled with gel
    const G4double m_EqPMTrOffset = 2.6 * mm;          ///< middle PMT circles are slightly further out due to m_EqPMTzOffset
    const G4double m_EqPMTzOffset = 10.0 * mm;         ///< z-offset of middle PMT circles w.r.t. center of glass sphere
    const G4double m_reflectorHalfZ = 15 * mm;           ///< half-height of reflector (before cutting to right form)
    const G4double m_reflectorConeSheetThickness = 0.5 * mm; ///< aluminum sheet thickness true for all reflective cones
    const G4double m_reflectorConeToHolder = 1.55 * mm;      ///< horizontal distance from K??rcher's construction
    const G4double m_thetaPolar = 33.0 * deg;
    const G4double m_thetaEquatorial = 72.0 * deg;
    const G4int m_numberPolarPMTs = 4;
    const G4int m_numberEqPMTs = 8;
    const G4int mRefConeAngle = 51;
    const G4double m_polarEquatorialPMTphiPhase = 0 * deg;
    const G4int m_totalNumberPMTs = (m_numberPolarPMTs + m_numberEqPMTs) * 2;
    const G4double m_supStructureRad = m_glassOutRad - m_glassThick - m_gelThickness;

    const G4double m_thetaEqLED = 61 * deg;   // 61 upper sphere, 180-61 lower sphere
    const G4double m_thetaPolLED = 8.2 * deg; // 8.2 upper sphere, 180-8.2 lower sphere
};

