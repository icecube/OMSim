
#ifndef abcDetectorComponent_h
#define abcDetectorComponent_h 1

#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"
#include "G4VisAttributes.hh"
#include "OMSimInputData.hh"
#include <G4SubtractionSolid.hh>
#include <G4PVPlacement.hh>
#include "G4MultiUnion.hh"
#include <sstream>

class abcDetectorComponent
{
public:
    abcDetectorComponent(){};
    virtual void construction() = 0; // Abstract method you have to define in order to make a derived class from abcDetectorComponent

    InputDataManager* mData; // Instance of OMSimInputdata, which should be started only once.
    bool mCheckOverlaps = true;
    
    struct Component{     // Struct of variables of component of a DetectorComponent
        G4VSolid* VSolid; //Solid Volume of component
        G4LogicalVolume* VLogical; //Logical Volume of component
        G4ThreeVector Position; //Position of component wrt to 0
        G4RotationMatrix Rotation; //Rotation of component wrt to 0
        G4String Name; // Name of Component
    };

    
    std::vector<G4ThreeVector> mPlacedPositions; //store the positions each time the components are placed
    std::vector<G4RotationMatrix> mPlacedOrientations; //store the orientations each time the components are placed
    std::vector<G4Transform3D> mPlacedTranslations;
    std::map<G4String, Component> mComponents;
    std::map<G4String, G4PVPlacement*> mLastPhysicals;

    /**Methods in abcDetectorComponent.cc*/
    void appendComponent(G4VSolid* pSolid, G4LogicalVolume* pLogical, G4ThreeVector pVector, G4RotationMatrix pRotation, G4String pName);
    G4bool checkIfExists(G4String pName);
    void deleteComponent(G4String pName);
    Component getComponent(G4String pName);
    G4Transform3D getNewPosition(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation);
    void integrateDetectorComponent(abcDetectorComponent* pToIntegrate, G4ThreeVector pPosition, G4RotationMatrix pRotation, G4String pNameExtension);
    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume*& pMother, G4String pNameExtension = "");
    void placeIt(G4Transform3D pTrans, G4LogicalVolume*& pMother, G4String pNameExtension = "");
    G4SubtractionSolid* substractToVolume(G4VSolid* pInputVolume, G4ThreeVector pSubstractionPos, G4RotationMatrix pSubstractionRot, G4String pNewVolumeName);
    G4SubtractionSolid* substractToVolume(G4VSolid* pInputVolume, G4Transform3D pTrans, G4String pNewVolumeName);
protected:
    
    const G4VisAttributes* mGlassVis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.25));
    const G4VisAttributes* mGelVis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.2));
    const G4VisAttributes* mSteelVis = new G4VisAttributes(G4Colour(0.6, 0.6, 0.7, 1.0));
    const G4VisAttributes* mAluVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.9, 1.0));
    const G4VisAttributes* mWhite = new G4VisAttributes(G4Colour(1, 1, 1, 1.0));
    const G4VisAttributes* mAbsorberSemiTransparentVis = new G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 0.5));
    const G4VisAttributes* mAbsorberVis = new G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 1.0));
    const G4VisAttributes* mBoardVis = new G4VisAttributes(G4Colour(0, 1, 0, 1));
    const G4VisAttributes* mBlueVis = new G4VisAttributes(G4Colour(0, 0, 1, 1));
    const G4VisAttributes* mAirVis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.2));
    const G4VisAttributes* mAirVis2 = new G4VisAttributes(G4Colour(0.0, 0, 1., 0.5));
    const G4VisAttributes mInvisibleVis = G4VisAttributes::GetInvisible();
    const G4VisAttributes* mRedVis = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1));
    const G4VisAttributes* mBlackVis = new G4VisAttributes(G4Colour(0.0, 0.0, 0.0, 1.0));
    const G4VisAttributes* mLEDvis= new G4VisAttributes(G4Colour(0.2,0.6,0.8,0.5));	    
    const G4VisAttributes *mPhotocathodeVis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.1));
};


#endif

