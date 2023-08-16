#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"
#include "OMSimLogger.hh"

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4SystemOfUnits.hh>
#include <G4Transform3D.hh>

void abcDetectorComponent::appendComponent(G4VSolid *pSolid, G4LogicalVolume *pLogical, G4ThreeVector pVector, G4RotationMatrix pRotation, G4String pName)
{
    if (checkIfExists(pName))
    {
        G4String mssg = pName + " already exists in the Components map. I will expand with a suffix, but this is bad!";
        log_critical(mssg);
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

G4bool abcDetectorComponent::checkIfExists(G4String pName)
{
    if (mComponents.find(pName) == mComponents.end())
    {
        return false;
    }
    return true;
}

void abcDetectorComponent::deleteComponent(G4String pName)
{
    if (checkIfExists(pName))
    {
        mComponents.erase(pName);
    }
    else
    {
        G4String mssg = "You are trying to delete " + pName + " from a Components dictionary, but it does not exist.";
        log_critical(mssg);
    }
}

abcDetectorComponent::Component abcDetectorComponent::getComponent(G4String pName)
{
    checkIfExists(pName);
    return mComponents.at(pName); // It will always try to get the component, even if it does not exist, since we want to stop the program if it happens.
}

G4Transform3D abcDetectorComponent::getNewPosition(G4ThreeVector pPosition, G4RotationMatrix pRotation, G4ThreeVector pObjectPosition, G4RotationMatrix pObjectRotation)
{
    return G4Transform3D(pObjectRotation.transform(pRotation), pPosition + pObjectPosition.transform(pRotation));
}

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

void abcDetectorComponent::placeIt(G4Transform3D pTrans, G4LogicalVolume *&pMother, G4String pNameExtension)
{
    mPlacedTranslations.push_back(pTrans);
    mPlacedPositions.push_back(pTrans.getTranslation());
    mPlacedOrientations.push_back(pTrans.getRotation());
    G4Transform3D lTrans;
    for (auto const &[key, Component] : mComponents)
    {
        G4String mssg = "Placing " + key + " in " + pMother->GetName() + ".";
        log_debug(mssg);
        lTrans = getNewPosition(pTrans.getTranslation(), pTrans.getRotation(), Component.Position, Component.Rotation);
        mLastPhysicals[key] = new G4PVPlacement(lTrans, Component.VLogical, Component.Name + pNameExtension, pMother, false, 0, mCheckOverlaps);
    }
}

void abcDetectorComponent::integrateDetectorComponent(abcDetectorComponent *pToIntegrate, G4ThreeVector pPosition, G4RotationMatrix pRotation, G4String pNameExtension)
{
    G4Transform3D lTrans;
    for (auto const &[key, Component] : pToIntegrate->mComponents)
    {
        lTrans = getNewPosition(pPosition, pRotation, Component.Position, Component.Rotation);
        appendComponent(Component.VSolid, Component.VLogical, lTrans.getTranslation(), lTrans.getRotation(), Component.Name + pNameExtension);
    }
}

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