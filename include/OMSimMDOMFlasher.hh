#ifndef OMSimMDOMFlasher_H
#define OMSimMDOMFlasher_H

#include "abcDetectorComponent.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4Tubs.hh"

class mDOM;

struct FlasherPositionInfo
{
    G4RotationMatrix orientation;
    G4double phi;
    G4double theta;
    G4double rho;
    G4ThreeVector globalPosition;
};

class mDOMFlasher : public abcDetectorComponent
{
public:
    mDOMFlasher(InputDataManager *pData);
    void construction();
    std::tuple<G4UnionSolid *, G4UnionSolid *, G4Tubs *> getSolids();
    void runBeamOnFlasher(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);

private:
    void makeSolids();
    void makeLogicalVolumes();
    void readFlasherProfile();

    FlasherPositionInfo getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);
    void configureGPS(FlasherPositionInfo flasherInfo);
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
};

#endif // OMSimMDOMFlasher_H
