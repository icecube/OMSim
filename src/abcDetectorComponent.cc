/** @file abcDetectorComponent.cc
 *  @brief Abstract base clase with a set of helper functions for building stuff
 *
 *  @author Martin Unland
 *
 *  @version revision 2, Geant4 10.7
 *
 *  @todo  - 

    Changelog:
        18 July 2022    Martin Unland
                        -Changed the component vector to a map to access more easily each component
                        -Overloaded PlaceIt to also accept Transform3D
 */


#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimLogger.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"

/**
 * Append one component to Components vector. 
 * @param pSolid Solid of component
 * @param pLogical Logical of component
 * @param pVector G4ThreeVector with position of component wrt 0
 * @param pRotation Rotation of component wrt 0
 * @param pName Name of component with which you can use the get method @see GetComponent()
 */
void abcDetectorComponent::AppendComponent(G4VSolid* pSolid, G4LogicalVolume* pLogical, G4ThreeVector pVector, G4RotationMatrix pRotation, G4String pName) {
    if (CheckIfExists(pName)){
        G4String mssg = pName+" already exists in the Components map. I will expand with a suffix, but this is bad!";
        critical(mssg);
        pName = pName+"_"+std::to_string(mComponents.size());
    }
    mComponents[pName] = {
        pSolid,
        pLogical,
        pVector,
        pRotation,
        pName,
        };
}


G4bool abcDetectorComponent::CheckIfExists(G4String pName) {
    if (mComponents.find(pName) == mComponents.end()){
        return false;
    }
    return true;
}
/**
 * Delete a component from Components vector. 
 * @param pName Name of component which you wanna delete
 */
void abcDetectorComponent::DeleteComponent(G4String pName) {
    if (CheckIfExists(pName)){
        mComponents.erase(pName);}
    else{
    G4String mssg = "You are trying to delete "+pName+" from a Components dictionary, but it does not exist.";
    critical(mssg);
    }
}

/**
 * Get a component from Components vector. 
 * @param pName Name of component which you wanna have
 */
abcDetectorComponent::Component abcDetectorComponent::GetComponent(G4String pName) {
    CheckIfExists(pName);
    return mComponents.at(pName); //It will always try to get the component, even if it does not exist, since we want to stop the program if it happens.
}


G4Transform3D abcDetectorComponent::GetNewPosition(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation)
{
    return G4Transform3D(pObjectRotation.transform(pRotation), pPosition + pObjectPosition.transform(pRotation));
}



/**
 * Placement of the DetectorComponent. Each Component is placed in the same mother.
 * @param pPosition G4ThreeVector with position of the components (as in G4PVPlacement())
 * @param pRotation G4RotationMatrix with rotation of the components (as in G4PVPlacement())
 * @param pMother G4LogicalVolume where the components is going to be placed (as in G4PVPlacement())
 * @param pIncludeHarness bool Harness is placed if true
 * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name
 */
void abcDetectorComponent::PlaceIt(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4LogicalVolume*& pMother, G4String pNameExtension)
{
    mPlacedPositions.push_back(pPosition);
    mPlacedOrientations.push_back(pRotation);
    G4Transform3D lTrans;
    for (auto const & [key, Component] : mComponents) {
        G4String mssg = "Placing "+key+" in "+pMother->GetName()+".";
        debug(mssg);
        lTrans = GetNewPosition(pPosition, pRotation, Component.Position, Component.Rotation);
        mLastPhysicals[key] = new G4PVPlacement(lTrans, Component.VLogical, Component.Name + pNameExtension, pMother, false, 0, mCheckOverlaps);
    }

}

/**
 * Placement of the DetectorComponent. Each Component is placed in the same mother.
 * @param pTrans G4Transform3D with position of the component
 * @param pMother G4LogicalVolume where the component is going to be placed (as in G4PVPlacement())
 * @param pIncludeHarness bool Harness is placed if true
 * @param pNameExtension G4String name of the physical volume. You should not have two physicals with the same name
 */
void abcDetectorComponent::PlaceIt(G4Transform3D pTrans, G4LogicalVolume*& pMother, G4String pNameExtension)
{
    mPlacedPositions.push_back(pTrans.getTranslation());
    mPlacedOrientations.push_back(pTrans.getRotation());
    G4Transform3D lTrans;
    for (auto const & [key, Component] : mComponents) {
        G4String mssg = "Placing "+key+" in "+pMother->GetName()+".";
        debug(mssg);
        lTrans = GetNewPosition(pTrans.getTranslation(), pTrans.getRotation(), Component.Position, Component.Rotation);
        mLastPhysicals[key] = new G4PVPlacement(lTrans, Component.VLogical, Component.Name + pNameExtension, pMother, false, 0, mCheckOverlaps);
    }

}


/**
 * Integrate components of another abcDetectorComponent instance. You can translate/rotate before integrating for coordinate system matching.
 * @param pToIntegrate abcDetectorComponent instance whose components we want to integrate
 * @param pPosition G4ThreeVector with position of new detector component where it will be integrated
 * @param pRotation G4RotationMatrix with rotation of new detector component
 * @param pNameExtension G4String to extend original name of component
 */
void abcDetectorComponent::IntegrateDetectorComponent(abcDetectorComponent* pToIntegrate, G4ThreeVector pPosition, G4RotationMatrix pRotation, G4String pNameExtension)
{
    G4Transform3D lTrans;
    for (auto const & [key, Component] : pToIntegrate->mComponents) {
        lTrans = GetNewPosition(pPosition, pRotation, Component.Position, Component.Rotation);
        AppendComponent(Component.VSolid, Component.VLogical, lTrans.getTranslation(), lTrans.getRotation(), Component.Name+pNameExtension);
    }

}


G4SubtractionSolid* abcDetectorComponent::SubstractToVolume(G4VSolid* pInputVolume, G4ThreeVector pSubstractionPos, G4RotationMatrix pSubstractionRot, G4String pNewVolumeName)
{
    G4SubtractionSolid* lSubstractedVolume;
    G4Transform3D lTrans;
    G4int iCounter = 0;
    for (auto const & [key, Component] : mComponents) {
        lTrans = GetNewPosition(pSubstractionPos, pSubstractionRot, Component.Position, Component.Rotation);
        G4String mssg = "Substracting "+key+" from "+pInputVolume->GetName()+".";
        debug(mssg);
        if (iCounter == 0) {
            lSubstractedVolume = new G4SubtractionSolid("SubstractedVolume", pInputVolume, Component.VSolid, lTrans);
        } else {
            lSubstractedVolume = new G4SubtractionSolid("SubstractedVolume", lSubstractedVolume, Component.VSolid, lTrans);
        }
        iCounter++;
    }
    return lSubstractedVolume;
}

