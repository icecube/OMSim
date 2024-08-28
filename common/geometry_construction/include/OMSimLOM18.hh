/** @file OMSimLOM18.hh
 *  @brief Construction of LOM18.
 *  @ingroup common
 */
#pragma once

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"

#include <G4LogicalVolume.hh>
#include <G4Polycone.hh>

class LOM18 : public OMSimOpticalModule
{
public:
    LOM18(G4bool pPlaceHarness = false);
    ~LOM18();
    void construction();
    double getPressureVesselWeight() {return 17.0*kg;};
    int getNumberOfPMTs() { return m_totalNumberPMTs;};
    G4String getName()
    {
        std::stringstream ss;
        ss << "LOM18/" << m_index;
        return ss.str();
    }
private:

    G4Polycone* createLOM18OuterSolid();
    G4Polycone* createLOM18InnerSolid();

    void appendEquatorBand();
    void placeCADSupportStructure(G4LogicalVolume* lInnerVolumeLogical);
    void placeCADPenetrator(G4LogicalVolume* lInnerVolumeLogical);
    
    
    void setPMTPositions();
    void createGelpadLogicalVolumes(G4Polycone* lGelSolid);

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




    //from PMTConstruction class (not readable directly...needs to be changed)
    G4double m_totalLenght;
    G4double m_outRad;
    G4double m_spherePosY; 
    G4double m_ellipsePosY; 
    G4double m_ellipseZaxis;

    //helper variables
    std::stringstream m_conv;
    std::stringstream m_converter2;

    //logical of gelpads
    std::vector<G4LogicalVolume*> m_gelPadLogical;

    G4double m_GlassEquatorWidth = 159*mm;
    G4double m_GlassPoleLength = 270*mm;
    G4double m_GlassThickPole = 12.5*mm;
    G4double m_GlassThickEquator = 16.5*mm;
    
    G4double m_thetaCenter = 48.0*deg;
    G4double m_thetaEquatorial = 60.0*deg;
    G4int m_numberPolarPMTs = 1;
    G4int m_NrCenterPMTs = 4;
    G4int m_NrEquatorialPMTs = 4;
    G4double m_EqPMTPhiPhase = 45.0*deg;

    //gelpad specific
    G4double m_polarPadOpeningAngle = 30.0*deg;
    G4double m_centerPadOpeningAngle = 10.0*deg;
    G4double m_equatorialPadOpeningAngle = 5.0*deg;
    G4double m_GelThicknessFrontPolarPMT = 3.5*mm;
    G4double m_gelThicknessFrontCenterPMT = 12.93*mm;
    G4double m_gelThicknessFrontEquatorialPMT = 14.52*mm;

    G4int m_numberPMTsPerHalf = m_numberPolarPMTs + m_NrCenterPMTs + m_NrEquatorialPMTs;
    G4int m_totalNumberPMTs = (m_numberPolarPMTs + m_NrCenterPMTs + m_NrEquatorialPMTs) * 2;
    //from PMT manager
    G4double m_PMToffset;
    G4double m_maxPMTRadius;   

    public:
    G4double m_cylinderAngle = 1.5*deg;
    G4double m_glassOutRad;
};

