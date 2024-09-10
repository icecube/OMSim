/** @file OMSimPMTConstruction.hh
 *  @brief Construction of the PMTs.
 *
 *  This class creates the solids of the PMTs and place them in the detector/OMs.
 */

#pragma once
#include "OMSimDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimPMTResponse.hh"
//#include "OMSimDetectorConstruction.hh"

#include <G4UnionSolid.hh>
#include <G4Tubs.hh>
class OMSimDetectorConstruction;


class OMSimPMTConstruction : public OMSimDetectorComponent
{

public:
    OMSimPMTConstruction();

    void construction();
    void configureSensitiveVolume(OMSimDetectorConstruction* pDetConst, G4String pName);

    G4double getDistancePMTCenterToTip();
    G4double getMaxPMTRadius();
    G4VSolid *getPMTSolid();
    G4LogicalVolume *getLogicalVolume();
    G4LogicalVolume* getPhotocathodeLV(){return m_photocathodeLV;};
    double getPMTGlassWeight();

    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    void placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    void selectPMT(G4String pPMTtoSelect);
    void includeHAcoating();

private:
    G4LogicalVolume* m_photocathodeLV;
    G4String m_selectedPMT;
    G4bool m_dynodeSystem = false;
    G4bool m_internalReflections = false;
    G4bool m_HACoatingBool = false;
    G4bool m_constructionFinished = false;
    G4double m_centreToTipDistance;
    std::tuple<G4VSolid *, G4VSolid *> getBulbSolid(G4String pSide);
    std::tuple<G4VSolid *, G4VSolid *> simpleBulbConstruction(G4String pSide);
    std::tuple<G4VSolid *, G4VSolid *> fullBulbConstruction(G4String pSide);
    G4VSolid *frontalBulbConstruction(G4String pSide);

    void readGlobalParameters(G4String pSide);

    G4VSolid *sphereEllipsePhotocathode(G4String p_side);
    G4VSolid *doubleEllipsePhotocathode(G4String pSide);
    G4VSolid *ellipsePhotocathode(G4String p_side);


    void constructHAcoating();
    void constructCathodeBackshield(G4LogicalVolume *pPMTIinner);
    void constructCADdynodeSystem(G4LogicalVolume *p_mother);
    
    OMSimPMTResponse* getPMTResponseInstance();

    G4bool m_simpleBulb = false;
    G4double m_missingTubeLength;
    G4PVPlacement *m_vacuumBackPhysical;
    G4PVPlacement* m_photocathodeRegionVacuumPhysical;
    G4OpticalSurface* m_photocathodeOpticalSurface;

    bool m_checkOverlaps;

    // Variables from json files are saved in the following members
    G4double m_totalLenght;
    G4double m_tubeWidth;
    G4double m_outRad;
    G4double m_ellipseXYaxis;
    G4double m_ellipseZaxis;
    G4double m_sphereEllipseTransition_r;
    G4double m_spherePosY;
    G4double m_ellipsePosY;
};

