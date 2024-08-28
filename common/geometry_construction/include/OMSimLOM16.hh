/** @file OMSimLOM16.hh
 *  @brief Construction of LOM16.
 *  @ingroup common
 */

#pragma once

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

class LOM16 : public OMSimOpticalModule
{
public:
    LOM16(G4bool pPlaceHarness = false);
    ~LOM16();
    void construction();
    double getPressureVesselWeight() {return (5.38+5.35)*kg;};
    int getNumberOfPMTs() { return m_totalNumberPMTs;};
    
    G4String getName()
    {
        std::stringstream ss;
        ss << "LOM16/" << m_index;
        return ss.str();
    }
private:
    G4UnionSolid* pressureVessel(const G4double pOutRad, G4String pSuffix);

    //Lom specific functions
    void placeCADSupportStructure();

    void appendEquatorBand();
    
    //for gelpad and PMT creation
    void placePMTsAndGelpads(G4VSolid* lGelSolid,G4LogicalVolume* lGelLogical);
    void setPMTAndGelpadPositions();
    void createGelpadLogicalVolumes(G4VSolid* lGelSolid);
    void placePMTs(G4LogicalVolume* lInnerVolumeLogical);
    void placeGelpads(G4LogicalVolume* lInnerVolumeLogical);

    //selection variables
    G4bool m_placeHarness = true;
    G4bool m_harnessUnion = true; //it should be true for the first module that you build, and then false

    //vectors for positions and rotations
    std::vector<G4ThreeVector> m_positionsPMT;
    std::vector<G4ThreeVector> m_positionsGelpad;
    std::vector<G4double> m_thetaPMT;
    std::vector<G4double> m_phiPMT;

    //helper variables
    std::stringstream m_converter;
    std::stringstream m_converter2;

    //logical of gelpads
    std::vector<G4LogicalVolume*> m_gelPadLogical;

    G4String m_PMTModel =  "pmt_Hamamatsu_4inch";

    const G4double m_xInternalCAD = 68.248*mm;
    const G4double m_yInternalCAD = 0*mm;
    const G4double m_zInternalCAD = -124.218*mm;
    const G4double m_gelPadDZ = 30.0*mm;
    const G4double m_glassOutRad = 153.2*mm;
    const G4double m_glassThick = 12.0*mm;
    const G4double m_glassInRad = m_glassOutRad - m_glassThick;
    const G4double m_cylinderHeight = 68.8*mm;
    const G4double m_cylinderAngle = 2.5*deg;
    const G4double m_gelThicknessFrontPMT = 3.6*mm;
    const G4double m_gelThickness = 4.5*mm;
    const G4double m_EqPMTrOffset = 2.6*mm;
    const G4double m_EqPMTzOffset = 62.5*mm;
    const G4double m_reflectorHalfZ = 15*mm;
    const G4double m_reflectorConeSheetThickness = 0.5*mm;
    const G4double m_reflectorConeToHolder = 1.55*mm;
    const G4double m_thetaPolar = 36.0*deg;
    const G4double m_thetaEquatorial = 62.0*deg;
    const G4int m_numberPolarPMTs = 4;
    const G4int m_numberEqPMTs = 4;
    const G4double m_polarEquatorialPMTphiPhase = 45.0*deg;
    const G4double m_equatorialTiltAngle = 15.0*deg;
    const G4double m_polarPadOpeningAngle = 30.0*deg;
    const G4double m_equatorialPadOpeningAngle = 22.0*deg;
    const G4int m_totalNumberPMTs = (m_numberPolarPMTs + m_numberEqPMTs) * 2;

    G4double m_PMToffset;
    G4double m_maxPMTRadius;

    const G4double m_equatorialBandWidth = 45 * mm; //Total width (both halves)
    const G4double m_equatorialBandThickness = 1 * mm; //Thickness since its a 3D object
};
