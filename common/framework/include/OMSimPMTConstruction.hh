/** @file OMSimPMTConstruction.hh
 *  @brief Construction of the PMTs.
 *
 *  This class creates the solids of the PMTs and place them in the detector/OMs.
 */

#ifndef OMSimPMTConstruction_h
#define OMSimPMTConstruction_h 1
#include "abcDetectorComponent.hh"

#include <G4UnionSolid.hh>
#include <G4Tubs.hh>

class OMSimPMTConstruction : public abcDetectorComponent
{
public:
    OMSimPMTConstruction(InputDataManager *pData);

    /**
     * @brief Constructs the PMT solid with all its components.
     */
    void construction();

    /**
     * Returns the distance between the 0.0 position of the PMT solid volume and the plane normal to the PMT frontal tip.
     * @return G4double
     */
    G4double getDistancePMTCenterToTip();

    /**
     * Returns the maximal radius of the frontal part of the PMT.
     * @return G4double
     */
    G4double getMaxPMTRadius();

    /**
     * Returns the solid of the constructed PMT.
     * @return G4UnionSolid of the PMT
     */
    G4VSolid *getPMTSolid();

    /**
     * It returns the bulb glass logical volume (PMT mother).
     */
    G4LogicalVolume *getLogicalVolume();
    double getPMTGlassWeight();
    /**
     * Placement of the PMT and definition of LogicalBorderSurfaces in case internal reflections are needed.
     * @param pPosition G4ThreeVector with position of the module (as in G4PVPlacement())
     * @param pRotation G4RotationMatrix with rotation of the module (as in G4PVPlacement())
     * @param pMother G4LogicalVolume where the module is going to be placed (as in G4PVPlacement())
     * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name
     */
    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");

    /**
     * @see PMT::placeIt
     * @param pTransform G4Transform3D with position & rotation of PMT
     */
    void placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension = "");

    /**
     * Select PMT model to use and assigns mPMT class.
     * @param G4String pPMTtoSelect string with the name of the PMT model
     */
    void selectPMT(G4String pPMTtoSelect);

    void includeHAcoating();

private:
    InputDataManager *mData;
    G4String mSelectedPMT;
    G4bool mDynodeSystem = false;
    G4bool mInternalReflections = false;
    G4bool mHACoatingBool = false;
    G4bool mConstructionFinished = false;

    void constructHAcoating();

    /**
     * The basic shape of the PMT is constructed twice, once for the external solid and once for the internal. A subtraction of these two shapes would yield the glass envelope of the PMT. The function calls either simpleBulbConstruction or fullBulbConstruction, depending on the data provided and simulation type. In case only the frontal curvate of the photocathode has to be well constructed, it calls simpleBulbConstruction. fullBulbConstruction constructs the neck of the PMT precisely, but it needs to have the fit data of the PMT type and is only needed if internal reflections are simulated.
     * @see simpleBulbConstruction
     * @see fullBulbConstruction
     */
    std::tuple<G4VSolid *, G4VSolid *> getBulbSolid(G4String pSide);

    /**
     * Construction of the basic shape of the PMT.
     * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
     */
    std::tuple<G4VSolid *, G4VSolid *> simpleBulbConstruction(G4String pSide);

    /**
     * Construction of the basic shape of the PMT for a full paramterised PMT. This is needed if internal reflections are simulated.
     * @return tuple of G4UnionSolid (the outer shape) and G4SubtractionSolid (the photocathode volume part)
     */
    std::tuple<G4VSolid *, G4VSolid *> fullBulbConstruction(G4String pSide);

    /**
     * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with sphereEllipsePhotocathode were fitted with a sphere and an ellipse.
     * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
     */
    G4UnionSolid *sphereEllipsePhotocathode();

    /**
     * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with sphereDoubleEllipsePhotocathode were fitted with a sphere and two ellipses.
     * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
     */
    G4UnionSolid *sphereDoubleEllipsePhotocathode(G4String pSide);

    /**
     * Construction of the frontal part of the PMT following the fits of the technical drawings. PMTs constructed with doubleEllipsePhotocathode were fitted with two ellipses.
     * @return G4UnionSolid lBulbSolid the frontal solid of the PMT
     */
    G4UnionSolid *doubleEllipsePhotocathode(G4String pSide);
    G4UnionSolid *ellipsePhotocathode();

    void checkPhotocathodeThickness();

    /**
     * Creates and positions a thin disk behind the photocathode volume in order to shield photons coming from behind the PMT. Only used when internal reflections are turned off.
     */
    void constructCathodeBackshield(G4LogicalVolume *pPMTIinner);

    /**
     * Construction & placement of the dynode system entrance for internal reflections. Currently only geometry for Hamamatsu R15458.
     * @param pMother LogicalVolume of the mother, where the dynode system entrance is placed (vacuum volume)
     */
    void constructCADdynodeSystem(G4LogicalVolume *pMother);

    /**
     * Construction of the photocathode layer.
     * @return G4SubtractionSolid
     */
    G4SubtractionSolid *constructPhotocathodeLayer();

    /**
     * Reads the parameter table and assigns the value and dimension of member variables.
     */
    void readGlobalParameters(G4String pSide);

    G4VSolid *frontalBulbConstruction(G4String pSide);

    G4bool mSimpleBulb = false;
    G4double mMissingTubeLength;
    G4PVPlacement *mVacuumBackPhysical;

    bool mCheckOverlaps = true;

    // Variables from json files are saved in the following members
    G4double mTotalLenght;
    G4double mTubeWidth;
    G4double mOutRad;
    G4double mEllipseXYaxis;
    G4double mEllipseZaxis;
    G4double mSphereEllipseTransition_r;
    G4double mSpherePos_y;
    G4double mEllipsePos_y;
};

#endif
//
