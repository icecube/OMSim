
/**
 * @file OMSimDetectorComponent.hh
 * @brief Provides helper base class OMSimDetectorComponent for constructing and managing detector components in OMSim.
 * @ingroup common
 */
#pragma once

#include "OMSimInputData.hh"

#include <G4Transform3D.hh>
#include <G4VisAttributes.hh>
#include <G4SubtractionSolid.hh>
#include <G4PVPlacement.hh>

/**
 *  @class OMSimDetectorComponent
 *  @brief Abstract base class used for constructing detector components.
 *
 *  @details This class serves as a base class for constructing and manipulating detector components in OMSim. It provides a set of helper functions for creating and arranging components within a detector, as well as methods for altering the geometry, such as subtracting one component from another.  The class maintains an internal list of components, allowing for operations to be performed on all components simultaneously.
 *  The `placeIt` methods allow for the placement of components within a specified logical volume, either via individual positioning and rotation parameters, or via a Geant4 transformation. Furthermore, components from one `OMSimDetectorComponent` instance can be integrated into another, with user-specified position, rotation, and name extension. This makes it possible to build complex geometrical structures in a modular fashion in cases where a single mother volume cannot be used.
 *
 *  @ingroup common
 */
class OMSimDetectorComponent
{
public:
    OMSimDetectorComponent();
    virtual void construction() = 0; ///<  Abstract method you have to define in order to make a derived class from OMSimDetectorComponent

    OMSimInputData *m_data; ///<  Instance of OMSimInputdata, which should be started only once.
    bool m_checkOverlaps = false;

    /**
     *  @struct OMSimDetectorComponent::Component
     *  @brief This struct represents a single detector component within an OMSimDetectorComponent instance.
     *  @details A Component is defined by several properties which specify its physical properties and location within the detector. 
     *  All operations available in OMSimDetectorComponent utilize this struct to perform changes to handle all components.
     */
    struct Component
    {
        G4VSolid *VSolid; ///< A pointer to a Geant4 G4VSolid object that represents the physical volume of the component.
        G4LogicalVolume *VLogical; ///< A pointer to a Geant4 G4LogicalVolume object that contains the material properties, sensitivity settings and visualization attributes of the component.
        G4ThreeVector Position; ///< A G4ThreeVector object representing the position of the component with respect to the origin of its containing volume.
        G4RotationMatrix Rotation; ///< A G4RotationMatrix object representing the rotation of the component with respect to its initial orientation.
        G4String Name; ///<  A G4String object representing the unique name of the component.
    };

    std::vector<G4ThreeVector> m_placedPositions;       ///<  store the positions each time the components are placed
    std::vector<G4RotationMatrix> m_placedOrientations; ///<  store the orientations each time the components are placed
    std::vector<G4Transform3D> m_placedTranslations; ///<  store the translation each time the components are placed
    std::map<G4String, Component> m_components;  ///<  dictionary with each component
    std::map<G4String, G4PVPlacement *> m_lastPhysicals; ///<  dictionary with the (last) G4PVPlacement of each component mComponents produced after calling placeIt

    void appendComponent(G4VSolid *pSolid, G4LogicalVolume *pLogical, G4ThreeVector pVector, G4RotationMatrix pRotation, G4String pName);
    G4bool checkIfExists(G4String pName);

    Component getComponent(G4String pName);
    G4Transform3D getNewPosition(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation);
    
    void integrateDetectorComponent(OMSimDetectorComponent *pToIntegrate, G4ThreeVector pPosition, G4RotationMatrix pRotation, G4String pNameExtension);
    void deleteComponent(G4String pName);

    void placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume *&pMother, G4String pNameExtension = "");
    void placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension = "");

    G4SubtractionSolid *substractToVolume(G4VSolid *pInputVolume, G4ThreeVector pSubstractionPos, G4RotationMatrix pSubstractionRot, G4String pNewVolumeName);
    G4SubtractionSolid *substractToVolume(G4VSolid *pInputVolume, G4Transform3D pTrans, G4String pNewVolumeName);

protected:
    // Use static const members for G4VisAttributes
    static const G4VisAttributes m_glassVis;
    static const G4VisAttributes m_gelVis;
    static const G4VisAttributes m_steelVis;
    static const G4VisAttributes m_aluVis;
    static const G4VisAttributes m_whiteVis;
    static const G4VisAttributes m_absorberSemiTransparentVis;
    static const G4VisAttributes m_absorberVis;
    static const G4VisAttributes m_boardVis;
    static const G4VisAttributes m_blueVis;
    static const G4VisAttributes m_airVis;
    static const G4VisAttributes m_airVis2;
    static const G4VisAttributes m_redVis;
    static const G4VisAttributes m_blackVis;
    static const G4VisAttributes m_LEDvis;
    static const G4VisAttributes m_photocathodeVis;
    static const G4VisAttributes m_invisibleVis;
};

