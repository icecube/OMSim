
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
/**
 *  @class abcDetectorComponent
 *  @brief Abstract base class used for constructing detector components.
 *
 *  @details This class serves as a base class for constructing and manipulating detector components in OMSim. It provides a set of helper functions for creating and arranging components within a detector, as well as methods for altering the geometry, such as subtracting one component from another. It utilizes Geant4's classes and datatypes to define and manipulate these components. The class maintains an internal list of components, allowing for operations to be performed on all components simultaneously.
 *  The `placeIt` methods allow for the placement of components within a specified logical volume, either via individual positioning and rotation parameters, or via a Geant4 transformation. Furthermore, components from one `abcDetectorComponent` instance can be integrated into another, with user-specified position, rotation, and name extension. This makes it possible to build complex geometrical structures in a modular fashion in cases where a single mother volume cannot be used.
 *
 *  @ingroup common
 *  @author Martin Unland
 */
class abcDetectorComponent
{
public:
    abcDetectorComponent(){};
    virtual void construction() = 0; // Abstract method you have to define in order to make a derived class from abcDetectorComponent

    InputDataManager *mData; // Instance of OMSimInputdata, which should be started only once.
    bool mCheckOverlaps = true;

    /**
     *  @struct abcDetectorComponent::Component
     *  @brief This struct represents a single detector component within an abcDetectorComponent instance.
     *
     *  @details A Component is defined by several properties which specify its physical properties and location within the detector. These properties include:
     *
     *  - `VSolid`: A pointer to a Geant4 G4VSolid object that represents the physical volume of the component.
     *  - `VLogical`: A pointer to a Geant4 G4LogicalVolume object that contains the material properties, sensitivity settings and visualization attributes of the component.
     *  - `Position`: A G4ThreeVector object representing the position of the component with respect to the origin of its containing volume.
     *  - `Rotation`: A G4RotationMatrix object representing the rotation of the component with respect to its initial orientation.
     *  - `Name`: A G4String object representing the unique name of the component.
     *  All operations available in abcDetectorComponent utilize this struct to perform changes to handle all components.
     */
    struct Component
    {
        G4VSolid *VSolid;
        G4LogicalVolume *VLogical;
        G4ThreeVector Position;
        G4RotationMatrix Rotation;
        G4String Name;
    };

    std::vector<G4ThreeVector> mPlacedPositions;       // store the positions each time the components are placed
    std::vector<G4RotationMatrix> mPlacedOrientations; // store the orientations each time the components are placed
    std::vector<G4Transform3D> mPlacedTranslations;
    std::map<G4String, Component> mComponents;
    std::map<G4String, G4PVPlacement *> mLastPhysicals;

    /**Methods in abcDetectorComponent.cc*/
    void appendComponent(G4VSolid *pSolid, G4LogicalVolume *pLogical, G4ThreeVector pVector, G4RotationMatrix pRotation, G4String pName);
    G4bool checkIfExists(G4String pName);
    void deleteComponent(G4String pName);
    Component getComponent(G4String pName);
    G4Transform3D getNewPosition(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation);
    void integrateDetectorComponent(abcDetectorComponent *pToIntegrate, G4ThreeVector pPosition, G4RotationMatrix pRotation, G4String pNameExtension);
    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    void placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    G4SubtractionSolid *substractToVolume(G4VSolid *pInputVolume, G4ThreeVector pSubstractionPos, G4RotationMatrix pSubstractionRot, G4String pNewVolumeName);
    G4SubtractionSolid *substractToVolume(G4VSolid *pInputVolume, G4Transform3D pTrans, G4String pNewVolumeName);

protected:
    const G4VisAttributes *mGlassVis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.25));
    const G4VisAttributes *mGelVis = new G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.2));
    const G4VisAttributes *mSteelVis = new G4VisAttributes(G4Colour(0.6, 0.6, 0.7, 1.0));
    const G4VisAttributes *mAluVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.9, 1.0));
    const G4VisAttributes *mWhite = new G4VisAttributes(G4Colour(1, 1, 1, 1.0));
    const G4VisAttributes *mAbsorberSemiTransparentVis = new G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 0.5));
    const G4VisAttributes *mAbsorberVis = new G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 1.0));
    const G4VisAttributes *mBoardVis = new G4VisAttributes(G4Colour(0, 1, 0, 1));
    const G4VisAttributes *mBlueVis = new G4VisAttributes(G4Colour(0, 0, 1, 1));
    const G4VisAttributes *mAirVis = new G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.2));
    const G4VisAttributes *mAirVis2 = new G4VisAttributes(G4Colour(0.0, 0, 1., 0.5));
    const G4VisAttributes mInvisibleVis = G4VisAttributes::GetInvisible();
    const G4VisAttributes *mRedVis = new G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1));
    const G4VisAttributes *mBlackVis = new G4VisAttributes(G4Colour(0.0, 0.0, 0.0, 1.0));
    const G4VisAttributes *mLEDvis = new G4VisAttributes(G4Colour(0.2, 0.6, 0.8, 0.5));
    const G4VisAttributes *mPhotocathodeVis = new G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.1));
};

#endif
