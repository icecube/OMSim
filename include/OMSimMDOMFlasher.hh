#ifndef OMSimMDOMFlasher_H
#define OMSimMDOMFlasher_H

#include "abcDetectorComponent.hh"
#include "G4LogicalVolume.hh"
#include "G4SystemOfUnits.hh"
#include "G4UnionSolid.hh"
#include "G4Tubs.hh"
#include "OMSimMDOM.hh"
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
    mDOMFlasher(OMSimInputData *pData);
    void Construction();
    std::tuple<G4UnionSolid *, G4UnionSolid *, G4Tubs *> GetSolids();
    void runBeamOnFlasher(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);

private:
    void MakeSolids();
    void MakeLogicalVolumes();
    void ReadFlasherProfile();

    FlasherPositionInfo getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);
    G4ThreeVector getFlasherGlobalThreeVector(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex, G4RotationMatrix orientation);
    void configureGPS(FlasherPositionInfo flasherInfo);
    G4ThreeVector buildRotVector(G4double phi, G4double theta, G4RotationMatrix orientation);

    template <typename T>
    void appendToStream(std::stringstream &stream, const T &val)
    {
        stream << ' ' << val;
    }
    void appendToStream(std::stringstream &stream)
    {
        // do nothing
    }
    template <typename T, typename... Args>
    void appendToStream(std::stringstream &stream, const T &val, const Args &...args)
    {
        stream << ' ' << val;
        appendToStream(stream, args...);
    }

    template <typename... Args>
    void applyCommand(const std::string &command, const Args &...args)
    {
        std::stringstream stream;
        stream << command;
        appendToStream(stream, args...);
        // UI->ApplyCommand(stream.str());
    }
    G4UnionSolid *mLEDSolid;
    G4UnionSolid *mFlasherHoleSolid;
    G4Tubs *mGlassWindowSolid;

    G4LogicalVolume *mFlasherHoleLogical;
    G4LogicalVolume *mGlassWindowLogical;
    G4LogicalVolume *mLEDLogical;
    G4bool mFlasherProfileAvailable = false;
};

#endif // OMSimMDOMFlasher_H
