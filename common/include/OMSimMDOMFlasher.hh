#ifndef OMSimMDOMFlasher_H
#define OMSimMDOMFlasher_H

#include "abcDetectorComponent.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4Tubs.hh"
#include "G4Navigator.hh"

class mDOM;

struct GlobalPosition
{
    CLHEP::HepRotation rotation;
    G4double x;
    G4double y;
    G4double z;
};

class mDOMFlasher : public abcDetectorComponent
{
public:
    mDOMFlasher(InputDataManager *pData);
    void construction();
    std::tuple<G4UnionSolid *, G4UnionSolid *, G4Tubs *> getSolids();
    void runBeamOnFlasher(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);
    void setNavigator(G4Navigator *pNavigator) { mNavigator = pNavigator; } // this is needed to get the rotation of the flasher

private:
    void makeSolids();
    void makeLogicalVolumes();
    void readFlasherProfile();

    GlobalPosition getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);
    void configureGPS(GlobalPosition flasherInfo);
    G4ThreeVector buildRotVector(G4double phi, G4double theta, G4RotationMatrix orientation);

    G4UnionSolid *mLEDSolid;
    G4UnionSolid *mFlasherHoleSolid;
    G4Tubs *mGlassWindowSolid;

    G4LogicalVolume *mFlasherHoleLogical;
    G4LogicalVolume *mGlassWindowLogical;
    G4LogicalVolume *mLEDLogical;
    G4bool mFlasherProfileAvailable = false;
	std::vector<double> mProfileX;
	std::vector<double> mProfileY;
    G4Navigator* mNavigator;
};

#endif // OMSimMDOMFlasher_H
