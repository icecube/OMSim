#include "OMSimDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimLogger.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SystemOfUnits.hh>
#include <G4Transform3D.hh>



OMSimDetectorComponent::OMSimDetectorComponent() : m_data(&OMSimInputData::getInstance())
{
    m_checkOverlaps = OMSimCommandArgsTable::getInstance().get<bool>("check_overlaps");
}


/**
 * @brief Append component to Components vector.
 * @details This function is used to add a new component to the 'Components' vector. Each component contains the solid volume, logical volume, position, rotation, and a unique name.
 * It checks first if a component with the same name already exists. If it does, a warning message is generated and the name of the new component is appended with a unique suffix to avoid conflicts.
 * The new component is then added to the 'Components' vector.
 *
 * @param p_solid A pointer to a G4VSolid object representing the solid volume of the component.
 * @param p_logical A pointer to a G4LogicalVolume object representing the logical volume of the component.
 * @param p_vector A G4ThreeVector object representing the position of the component with respect to the origin.
 * @param p_rotation A G4RotationMatrix object representing the rotation of the component with respect to the origin.
 * @param p_name A G4String object representing the unique name of the component.
 */
void OMSimDetectorComponent::appendComponent(G4VSolid *p_solid, G4LogicalVolume *p_logical, G4ThreeVector p_vector, G4RotationMatrix p_rotation, G4String p_name)
{
    if (checkIfExists(p_name))
    {
        log_critical("{} already exists in the Components map. I will expand with a suffix, but this is bad!", p_name);
        p_name = p_name + "_" + std::to_string(m_components.size());
    }
    m_components[p_name] = {
        p_solid,
        p_logical,
        p_vector,
        p_rotation,
        p_name,
    };
}

/**
 * @brief Check if a component with a certain name exists in the Components map.
 * @param p_name A G4String object representing the unique name of the component to check.
 * @return G4bool True if the component exists in the map, false otherwise.
 */
G4bool OMSimDetectorComponent::checkIfExists(G4String p_name)
{
    if (m_components.find(p_name) == m_components.end())
    {
        log_trace("Component {} is not in component dictionary", p_name);
        return false;
    }
    return true;
}
/**
 * @brief Deletes a specified component from the Components map.
 * @param p_name A G4String representing the name of the component to delete. The component must exist in the 'Components' map.
 */
void OMSimDetectorComponent::deleteComponent(G4String p_name)
{
    log_trace("Deleting component {} from component dictionary", p_name);
    if (checkIfExists(p_name))
    {
        m_components.erase(p_name);
    }
    else
    {
        log_critical("You are trying to delete {} from a Components dictionary, but it does not exist.", p_name);
    }
}
/**
 * @brief Retrieves a specified component from the Components map.
 * @param p_name A G4String name of the component to fetch. The component must exist in the 'Components' map.
 * @return OMSimDetectorComponent::Component structure containing the solid volume, logical volume, position, rotation, and name of component.
 */
OMSimDetectorComponent::Component OMSimDetectorComponent::getComponent(G4String p_name)
{
    log_trace("Getting component {} from component dictionary", p_name);
    if (checkIfExists(p_name))
    {
        return m_components.at(p_name);
    }
    else
    {
        log_error("Getting component {} failed!, not found in component dictionary!", p_name);
        return m_components.at(p_name); // It will always try to get the component, even if it does not exist, since we want to stop the program if it happens.
    }
}

/**
 * @brief Computes a new position for a sub-component, based on a given global position and rotation.
 * @param p_position The initial position (a G4ThreeVector) of the sub-component
 * @param p_rotation The initial orientation (a G4RotationMatrix) of the sub-component
 * @param pObjectPosition The position (a G4ThreeVector) of the object with respect to the original position
 * @param pObjectRotation The orientation (a G4RotationMatrix) of the object with respect to the original orientation
 * @return G4Transform3D representing the new position and orientation of the component
 */
G4Transform3D OMSimDetectorComponent::getNewPosition(G4ThreeVector p_position, G4RotationMatrix p_rotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation)
{
    return G4Transform3D(pObjectRotation.transform(p_rotation), p_position + pObjectPosition.transform(p_rotation));
}

/**
 * @brief Placement of the DetectorComponent. Each Component is placed in the same mother.
 * @param p_position G4ThreeVector with position of the components (as in G4PVPlacement()).
 * @param p_rotation G4RotationMatrix with rotation of the components (as in G4PVPlacement()).
 * @param p_mother G4LogicalVolume where the components is going to be placed (as in G4PVPlacement()).
 * @param p_nameExtension G4String name of the physical volume. You should not have two physicals with the same name.
 */
void OMSimDetectorComponent::placeIt(G4ThreeVector p_position, G4RotationMatrix p_rotation, G4LogicalVolume *&p_mother, G4String p_nameExtension)
{
    m_placedTranslations.push_back(G4Transform3D(p_rotation, p_position));
    m_placedPositions.push_back(p_position);
    m_placedOrientations.push_back(p_rotation);
    G4Transform3D trans;
    for (auto const &[key, Component] : m_components)
    {
        G4String mssg = "Placing " + key + " in " + p_mother->GetName() + ".";
        log_trace(mssg);
        trans = getNewPosition(p_position, p_rotation, Component.Position, Component.Rotation);
        m_lastPhysicals[key] = new G4PVPlacement(trans, Component.VLogical, Component.Name + p_nameExtension, p_mother, false, 0, m_checkOverlaps);
    }
}
/**
 * @brief Places the components in a specified mother volume using a provided transformation.
 * @param p_trans The G4Transform3D object representing the transformation to be applied to the components.
 * @param p_mother G4LogicalVolume where the components is going to be placed (as in G4PVPlacement()).
 * @param p_nameExtension G4String name of the physical volume. You should not have two physicals with the same name.
 */
void OMSimDetectorComponent::placeIt(G4Transform3D p_trans, G4LogicalVolume *&p_mother, G4String p_nameExtension)
{
    m_placedTranslations.push_back(p_trans);
    m_placedPositions.push_back(p_trans.getTranslation());
    m_placedOrientations.push_back(p_trans.getRotation());
    G4Transform3D trans;
    for (auto const &[key, Component] : m_components)
    {
        log_trace("Placing {} in {}.", key, p_mother->GetName());
        trans = getNewPosition(p_trans.getTranslation(), p_trans.getRotation(), Component.Position, Component.Rotation);
        m_lastPhysicals[key] = new G4PVPlacement(trans, Component.VLogical, Component.Name + p_nameExtension, p_mother, false, 0, m_checkOverlaps);
    }
}
/**
 * @brief Integrates the components of another OMSimDetectorComponent instance.
 * @details This function is used to incorporate the components of another instance of OMSimDetectorComponent into the current instance.
 * The provided position and rotation parameters are used to transform the components from the input OMSimDetectorComponent instance before integrating.
 * Each transformed component from the input instance is then appended to the current instance using the appendComponent() function.
 * A name extension is applied to the original component names for distinction after integration.
 *
 * @param p_toIntegrate The OMSimDetectorComponent instance whose components are to be integrated.
 * @param p_position The G4ThreeVector representing the position where the new OMSimDetectorComponent instance will be integrated.
 * @param p_rotation The G4RotationMatrix representing the rotation of the new OMSimDetectorComponent instance.
 * @param p_nameExtension A G4String used to extend the original name of the component.
 */
void OMSimDetectorComponent::integrateDetectorComponent(OMSimDetectorComponent *p_toIntegrate, G4ThreeVector p_position, G4RotationMatrix p_rotation, G4String p_nameExtension)
{
    G4Transform3D trans;
    for (auto const &[key, Component] : p_toIntegrate->m_components)
    {
        trans = getNewPosition(p_position, p_rotation, Component.Position, Component.Rotation);
        appendComponent(Component.VSolid, Component.VLogical, trans.getTranslation(), trans.getRotation(), Component.Name + p_nameExtension);
    }
}

/**
 * @brief Subtracts components from a given solid volume using position and rotation.
 * @param p_inputVolume The G4VSolid volume that components will be subtracted from.
 * @param p_substractionPos The G4ThreeVector representing the position that will be used in the transformation of each component before it is subtracted from p_inputVolume.
 * @param p_substractionRot The G4RotationMatrix representing the rotation that will be used in the transformation of each component before it is subtracted from p_inputVolume.
 * @param p_newVolumeName The G4String name for the new volume that results from the subtraction.
 *
 * @return Returns a G4SubtractionSolid which is the result of subtracting the OMSimDetectorComponent's components from p_inputVolume.
 */
G4SubtractionSolid *OMSimDetectorComponent::substractToVolume(G4VSolid *p_inputVolume, G4ThreeVector p_substractionPos, G4RotationMatrix p_substractionRot, G4String p_newVolumeName)
{
    G4SubtractionSolid *substractedVolume;
    G4Transform3D trans;
    G4int counter = 0;
    for (auto const &[key, Component] : m_components)
    {
        trans = getNewPosition(p_substractionPos, p_substractionRot, Component.Position, Component.Rotation);
        G4String mssg = "Substracting " + key + " from " + p_inputVolume->GetName() + ".";
        log_trace(mssg);
        if (counter == 0)
        {
            substractedVolume = new G4SubtractionSolid("SubstractedVolume", p_inputVolume, Component.VSolid, trans);
        }
        else
        {
            substractedVolume = new G4SubtractionSolid("SubstractedVolume", substractedVolume, Component.VSolid, trans);
        }
        counter++;
    }
    return substractedVolume;
}

/**
 * @brief Subtracts components from a given solid volume.
 * @details This function enables you to subtract the shape defined by components of the OMSimDetectorComponent instance from a specified input volume. This can be useful for modeling complex geometries where a solid needs to have some parts "cut out" from it. The method goes through each component of the OMSimDetectorComponent, computes the transformation needed, and subtracts it from the input volume. The resulting subtracted solid is returned.
 *
 * @param p_inputVolume The G4VSolid volume that components will be subtracted from.
 * @param p_trans The G4Transform3D representing the transformation that will be applied to each component before it is subtracted from p_inputVolume.
 * @param p_newVolumeName The G4String name for the new volume that results from the subtraction.
 *
 * @return Returns a G4SubtractionSolid which is the result of subtracting the OMSimDetectorComponent's components from p_inputVolume.
 */
G4SubtractionSolid *OMSimDetectorComponent::substractToVolume(G4VSolid *p_inputVolume, G4Transform3D p_trans, G4String p_newVolumeName)
{
    G4SubtractionSolid *substractedVolume;
    G4int counter = 0;
    for (auto const &[key, Component] : m_components)
    {
        G4String mssg = "Substracting " + key + " from " + p_inputVolume->GetName() + ".";
        log_trace(mssg);
        if (counter == 0)
        {
            substractedVolume = new G4SubtractionSolid("SubstractedVolume", p_inputVolume, Component.VSolid, p_trans);
        }
        else
        {
            substractedVolume = new G4SubtractionSolid("SubstractedVolume", substractedVolume, Component.VSolid, p_trans);
        }
        counter++;
    }
    return substractedVolume;
}



const G4VisAttributes OMSimDetectorComponent::m_glassVis = G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.25));
const G4VisAttributes OMSimDetectorComponent::m_gelVis = G4VisAttributes(G4Colour(0.45, 0.5, 0.35, 0.2));
const G4VisAttributes OMSimDetectorComponent::m_steelVis = G4VisAttributes(G4Colour(0.6, 0.6, 0.7, 1.0));
const G4VisAttributes OMSimDetectorComponent::m_aluVis = G4VisAttributes(G4Colour(0.8, 0.8, 0.9, 1.0));
const G4VisAttributes OMSimDetectorComponent::m_whiteVis = G4VisAttributes(G4Colour(1, 1, 1, 1.0));
const G4VisAttributes OMSimDetectorComponent::m_absorberSemiTransparentVis = G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 0.5));
const G4VisAttributes OMSimDetectorComponent::m_absorberVis = G4VisAttributes(G4Colour(0.2, 0.2, 0.2, 1.0));
const G4VisAttributes OMSimDetectorComponent::m_boardVis = G4VisAttributes(G4Colour(0, 1, 0, 1));
const G4VisAttributes OMSimDetectorComponent::m_blueVis = G4VisAttributes(G4Colour(0, 0, 1, 1));
const G4VisAttributes OMSimDetectorComponent::m_airVis = G4VisAttributes(G4Colour(0.7, 0.7, 0.8, 0.2));
const G4VisAttributes OMSimDetectorComponent::m_airVis2 = G4VisAttributes(G4Colour(0.0, 0, 1., 0.5));
const G4VisAttributes OMSimDetectorComponent::m_redVis = G4VisAttributes(G4Colour(1.0, 0.0, 0.0, 1));
const G4VisAttributes OMSimDetectorComponent::m_blackVis = G4VisAttributes(G4Colour(0.0, 0.0, 0.0, 1.0));
const G4VisAttributes OMSimDetectorComponent::m_LEDvis = G4VisAttributes(G4Colour(0.2, 0.6, 0.8, 0.5));
const G4VisAttributes OMSimDetectorComponent::m_photocathodeVis = G4VisAttributes(G4Colour(1.0, 1.0, 0.0, 0.1));
const G4VisAttributes OMSimDetectorComponent::m_invisibleVis = G4VisAttributes::GetInvisible();