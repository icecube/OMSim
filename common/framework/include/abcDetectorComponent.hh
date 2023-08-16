
/**
 * @file abcDetectorComponent.hh
 * @brief Provides helper base class abcDetectorComponent for constructing and managing detector components in OMSim.
 * @ingroup common
 */
#ifndef abcDetectorComponent_h
#define abcDetectorComponent_h 1

#include "OMSimInputData.hh"

#include <G4Transform3D.hh>
#include <G4VisAttributes.hh>
#include <G4SubtractionSolid.hh>
#include <G4PVPlacement.hh>

/**
 *  @class abcDetectorComponent
 *  @brief Abstract base class used for constructing detector components.
 *
 *  @details This class serves as a base class for constructing and manipulating detector components in OMSim. It provides a set of helper functions for creating and arranging components within a detector, as well as methods for altering the geometry, such as subtracting one component from another.  The class maintains an internal list of components, allowing for operations to be performed on all components simultaneously.
 *  The `placeIt` methods allow for the placement of components within a specified logical volume, either via individual positioning and rotation parameters, or via a Geant4 transformation. Furthermore, components from one `abcDetectorComponent` instance can be integrated into another, with user-specified position, rotation, and name extension. This makes it possible to build complex geometrical structures in a modular fashion in cases where a single mother volume cannot be used.
 *
 *  @ingroup common
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

    /**
     * @brief Append component to Components vector.
     * @details This function is used to add a new component to the 'Components' vector. Each component contains the solid volume, logical volume, position, rotation, and a unique name.
     * It checks first if a component with the same name already exists. If it does, a warning message is generated and the name of the new component is appended with a unique suffix to avoid conflicts.
     * The new component is then added to the 'Components' vector.
     *
     * @param pSolid A pointer to a G4VSolid object representing the solid volume of the component.
     * @param pLogical A pointer to a G4LogicalVolume object representing the logical volume of the component.
     * @param pVector A G4ThreeVector object representing the position of the component with respect to the origin.
     * @param pRotation A G4RotationMatrix object representing the rotation of the component with respect to the origin.
     * @param pName A G4String object representing the unique name of the component.
     */
    void appendComponent(G4VSolid *pSolid, G4LogicalVolume *pLogical, G4ThreeVector pVector, G4RotationMatrix pRotation, G4String pName);

    /**
     * @brief Check if a component with a certain name exists in the Components map.
     * @param pName A G4String object representing the unique name of the component to check.
     * @return G4bool True if the component exists in the map, false otherwise.
     */
    G4bool checkIfExists(G4String pName);

    /**
     * @brief Deletes a specified component from the Components map.
     * @param pName A G4String representing the name of the component to delete. The component must exist in the 'Components' map.
     */
    void deleteComponent(G4String pName);

    /**
     * @brief Retrieves a specified component from the Components map.
     * @param pName A G4String name of the component to fetch. The component must exist in the 'Components' map.
     * @return abcDetectorComponent::Component structure containing the solid volume, logical volume, position, rotation, and name of component.
     */
    Component getComponent(G4String pName);

    /**
     * @brief Computes a new position for a sub-component, based on a given global position and rotation.
     * @param pPosition The initial position (a G4ThreeVector) of the sub-component
     * @param pRotation The initial orientation (a G4RotationMatrix) of the sub-component
     * @param pObjectPosition The position (a G4ThreeVector) of the object with respect to the original position
     * @param pObjectRotation The orientation (a G4RotationMatrix) of the object with respect to the original orientation
     * @return G4Transform3D representing the new position and orientation of the component
     */
    G4Transform3D getNewPosition(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation);

    /**
     * @brief Integrates the components of another abcDetectorComponent instance.
     * @details This function is used to incorporate the components of another instance of abcDetectorComponent into the current instance.
     * The provided position and rotation parameters are used to transform the components from the input abcDetectorComponent instance before integrating.
     * Each transformed component from the input instance is then appended to the current instance using the appendComponent() function.
     * A name extension is applied to the original component names for distinction after integration.
     *
     * @param pToIntegrate The abcDetectorComponent instance whose components are to be integrated.
     * @param pPosition The G4ThreeVector representing the position where the new abcDetectorComponent instance will be integrated.
     * @param pRotation The G4RotationMatrix representing the rotation of the new abcDetectorComponent instance.
     * @param pNameExtension A G4String used to extend the original name of the component.
     */
    void integrateDetectorComponent(abcDetectorComponent *pToIntegrate, G4ThreeVector pPosition, G4RotationMatrix pRotation, G4String pNameExtension);

    /**
     * @brief Placement of the DetectorComponent. Each Component is placed in the same mother.
     * @param pPosition G4ThreeVector with position of the components (as in G4PVPlacement()).
     * @param pRotation G4RotationMatrix with rotation of the components (as in G4PVPlacement()).
     * @param pMother G4LogicalVolume where the components is going to be placed (as in G4PVPlacement()).
     * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name.
     */
    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");

    /**
     * @brief Places the components in a specified mother volume using a provided transformation.
     * @param pTrans The G4Transform3D object representing the transformation to be applied to the components.
     * @param pMother G4LogicalVolume where the components is going to be placed (as in G4PVPlacement()).
     * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name.
     */
    void placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension = "");

    /**
     * @brief Subtracts components from a given solid volume using position and rotation.
     * @param pInputVolume The G4VSolid volume that components will be subtracted from.
     * @param pSubstractionPos The G4ThreeVector representing the position that will be used in the transformation of each component before it is subtracted from pInputVolume.
     * @param pSubstractionRot The G4RotationMatrix representing the rotation that will be used in the transformation of each component before it is subtracted from pInputVolume.
     * @param pNewVolumeName The G4String name for the new volume that results from the subtraction.
     *
     * @return Returns a G4SubtractionSolid which is the result of subtracting the abcDetectorComponent's components from pInputVolume.
     */
    G4SubtractionSolid *substractToVolume(G4VSolid *pInputVolume, G4ThreeVector pSubstractionPos, G4RotationMatrix pSubstractionRot, G4String pNewVolumeName);

    /**
     * @brief Subtracts components from a given solid volume.
     * @details This function enables you to subtract the shape defined by components of the abcDetectorComponent instance from a specified input volume. This can be useful for modeling complex geometries where a solid needs to have some parts "cut out" from it. The method goes through each component of the abcDetectorComponent, computes the transformation needed, and subtracts it from the input volume. The resulting subtracted solid is returned.
     *
     * @param pInputVolume The G4VSolid volume that components will be subtracted from.
     * @param pTrans The G4Transform3D representing the transformation that will be applied to each component before it is subtracted from pInputVolume.
     * @param pNewVolumeName The G4String name for the new volume that results from the subtraction.
     *
     * @return Returns a G4SubtractionSolid which is the result of subtracting the abcDetectorComponent's components from pInputVolume.
     */
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
