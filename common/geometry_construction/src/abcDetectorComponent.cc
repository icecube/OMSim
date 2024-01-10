#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimLogger.hh"

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SystemOfUnits.hh>
#include <G4Transform3D.hh>

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
void abcDetectorComponent::appendComponent(G4VSolid *pSolid, G4LogicalVolume *pLogical, G4ThreeVector pVector, G4RotationMatrix pRotation, G4String pName)
{
    if (checkIfExists(pName))
    {
        log_critical("{} already exists in the Components map. I will expand with a suffix, but this is bad!", pName);
        pName = pName + "_" + std::to_string(mComponents.size());
    }
    mComponents[pName] = {
        pSolid,
        pLogical,
        pVector,
        pRotation,
        pName,
    };
}

/**
 * @brief Check if a component with a certain name exists in the Components map.
 * @param pName A G4String object representing the unique name of the component to check.
 * @return G4bool True if the component exists in the map, false otherwise.
 */
G4bool abcDetectorComponent::checkIfExists(G4String pName)
{
    if (mComponents.find(pName) == mComponents.end())
    {
        log_trace("Component {} is not in component dictionary", pName);
        return false;
    }
    return true;
}
/**
 * @brief Deletes a specified component from the Components map.
 * @param pName A G4String representing the name of the component to delete. The component must exist in the 'Components' map.
 */
void abcDetectorComponent::deleteComponent(G4String pName)
{
    log_trace("Deleting component {} from component dictionary", pName);
    if (checkIfExists(pName))
    {
        mComponents.erase(pName);
    }
    else
    {
        log_critical("You are trying to delete {} from a Components dictionary, but it does not exist.", pName);
    }
}
/**
 * @brief Retrieves a specified component from the Components map.
 * @param pName A G4String name of the component to fetch. The component must exist in the 'Components' map.
 * @return abcDetectorComponent::Component structure containing the solid volume, logical volume, position, rotation, and name of component.
 */
abcDetectorComponent::Component abcDetectorComponent::getComponent(G4String pName)
{
    log_trace("Getting component {} from component dictionary", pName);
    if (checkIfExists(pName))
    {
        return mComponents.at(pName);
    }
    else
    {
        log_error("Getting component {} failed!, not found in component dictionary!", pName);
        return mComponents.at(pName); // It will always try to get the component, even if it does not exist, since we want to stop the program if it happens.
    }
}

/**
 * @brief Computes a new position for a sub-component, based on a given global position and rotation.
 * @param pPosition The initial position (a G4ThreeVector) of the sub-component
 * @param pRotation The initial orientation (a G4RotationMatrix) of the sub-component
 * @param pObjectPosition The position (a G4ThreeVector) of the object with respect to the original position
 * @param pObjectRotation The orientation (a G4RotationMatrix) of the object with respect to the original orientation
 * @return G4Transform3D representing the new position and orientation of the component
 */
G4Transform3D abcDetectorComponent::getNewPosition(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation)
{
    return G4Transform3D(pObjectRotation.transform(pRotation), pPosition + pObjectPosition.transform(pRotation));
}

/**
 * @brief Placement of the DetectorComponent. Each Component is placed in the same mother.
 * @param pPosition G4ThreeVector with position of the components (as in G4PVPlacement()).
 * @param pRotation G4RotationMatrix with rotation of the components (as in G4PVPlacement()).
 * @param pMother G4LogicalVolume where the components is going to be placed (as in G4PVPlacement()).
 * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name.
 */
void abcDetectorComponent::placeIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume *&pMother, G4String pNameExtension)
{
    mPlacedTranslations.push_back(G4Transform3D(pRotation, pPosition));
    mPlacedPositions.push_back(pPosition);
    mPlacedOrientations.push_back(pRotation);
    G4Transform3D lTrans;
    for (auto const &[key, Component] : mComponents)
    {
        G4String mssg = "Placing " + key + " in " + pMother->GetName() + ".";
        log_debug(mssg);
        lTrans = getNewPosition(pPosition, pRotation, Component.Position, Component.Rotation);
        mLastPhysicals[key] = new G4PVPlacement(lTrans, Component.VLogical, Component.Name + pNameExtension, pMother, false, 0, mCheckOverlaps);
    }
}
/**
 * @brief Places the components in a specified mother volume using a provided transformation.
 * @param pTrans The G4Transform3D object representing the transformation to be applied to the components.
 * @param pMother G4LogicalVolume where the components is going to be placed (as in G4PVPlacement()).
 * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name.
 */
void abcDetectorComponent::placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension)
{
    mPlacedTranslations.push_back(pTrans);
    mPlacedPositions.push_back(pTrans.getTranslation());
    mPlacedOrientations.push_back(pTrans.getRotation());
    G4Transform3D lTrans;
    for (auto const &[key, Component] : mComponents)
    {
        log_debug("Placing {} in {}.", key, pMother->GetName());
        lTrans = getNewPosition(pTrans.getTranslation(), pTrans.getRotation(), Component.Position, Component.Rotation);
        mLastPhysicals[key] = new G4PVPlacement(lTrans, Component.VLogical, Component.Name + pNameExtension, pMother, false, 0, mCheckOverlaps);
    }
}
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
void abcDetectorComponent::integrateDetectorComponent(abcDetectorComponent *pToIntegrate, G4ThreeVector pPosition, G4RotationMatrix pRotation, G4String pNameExtension)
{
    G4Transform3D lTrans;
    for (auto const &[key, Component] : pToIntegrate->mComponents)
    {
        lTrans = getNewPosition(pPosition, pRotation, Component.Position, Component.Rotation);
        appendComponent(Component.VSolid, Component.VLogical, lTrans.getTranslation(), lTrans.getRotation(), Component.Name + pNameExtension);
    }
}

/**
 * @brief Subtracts components from a given solid volume using position and rotation.
 * @param pInputVolume The G4VSolid volume that components will be subtracted from.
 * @param pSubstractionPos The G4ThreeVector representing the position that will be used in the transformation of each component before it is subtracted from pInputVolume.
 * @param pSubstractionRot The G4RotationMatrix representing the rotation that will be used in the transformation of each component before it is subtracted from pInputVolume.
 * @param pNewVolumeName The G4String name for the new volume that results from the subtraction.
 *
 * @return Returns a G4SubtractionSolid which is the result of subtracting the abcDetectorComponent's components from pInputVolume.
 */
G4SubtractionSolid *abcDetectorComponent::substractToVolume(G4VSolid *pInputVolume, G4ThreeVector pSubstractionPos, G4RotationMatrix pSubstractionRot, G4String pNewVolumeName)
{
    G4SubtractionSolid *lSubstractedVolume;
    G4Transform3D lTrans;
    G4int iCounter = 0;
    for (auto const &[key, Component] : mComponents)
    {
        lTrans = getNewPosition(pSubstractionPos, pSubstractionRot, Component.Position, Component.Rotation);
        G4String mssg = "Substracting " + key + " from " + pInputVolume->GetName() + ".";
        log_debug(mssg);
        if (iCounter == 0)
        {
            lSubstractedVolume = new G4SubtractionSolid("SubstractedVolume", pInputVolume, Component.VSolid, lTrans);
        }
        else
        {
            lSubstractedVolume = new G4SubtractionSolid("SubstractedVolume", lSubstractedVolume, Component.VSolid, lTrans);
        }
        iCounter++;
    }
    return lSubstractedVolume;
}

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
G4SubtractionSolid *abcDetectorComponent::substractToVolume(G4VSolid *pInputVolume, G4Transform3D pTrans, G4String pNewVolumeName)
{
    G4SubtractionSolid *lSubstractedVolume;
    G4int iCounter = 0;
    for (auto const &[key, Component] : mComponents)
    {
        G4String mssg = "Substracting " + key + " from " + pInputVolume->GetName() + ".";
        log_debug(mssg);
        if (iCounter == 0)
        {
            lSubstractedVolume = new G4SubtractionSolid("SubstractedVolume", pInputVolume, Component.VSolid, pTrans);
        }
        else
        {
            lSubstractedVolume = new G4SubtractionSolid("SubstractedVolume", lSubstractedVolume, Component.VSolid, pTrans);
        }
        iCounter++;
    }
    return lSubstractedVolume;
}