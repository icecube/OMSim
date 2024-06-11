/**
 * @file OMSimMDOMFlasher.hh
 * @brief Defines the mDOMFlasher class for simulating the 10 flashers in an mDOM optical module.
 * 
 * @details
 * This file contains the declaration of the mDOMFlasher class and related structures.
 * mDOMFlasher is responsible for creating the flashers in the mDOM, which includes a LED, 
 * the air around it, and a glass window on top. The class provides functionalities for constructing 
 * the solids and logical volumes, retrieving them, and simulating the LED flashing based on a given profile.
 * @ingroup common
 */

#ifndef OMSimMDOMFlasher_H
#define OMSimMDOMFlasher_H

#include "abcDetectorComponent.hh"
#include <G4UnionSolid.hh>
#include <G4Navigator.hh>

class mDOM;


/**
 * @struct GlobalPosition
 * @brief Holds the global position and orientation of a flasher.
 */
struct GlobalPosition
{
    CLHEP::HepRotation rotation;
    G4double x;
    G4double y;
    G4double z;
};

/**
 * @class mDOMFlasher
 * @brief The mDOMFlasher class represents the 10 flashers in an mDOM optical module.
 *
 * @details
 * This class is responsible for creating the flashers in the mDOM,
 * which includes a LED, the air around it, and a glass window on top of it. This class also has methods for retrieving flasher solids,
 * activating a specific flasher in a particular module, and configuring the GPS for flasher simulations.
 * @ingroup common
 */
class mDOMFlasher : public abcDetectorComponent
{
public:
    mDOMFlasher(InputDataManager *pData);
    void construction();
    std::tuple<G4UnionSolid *, G4UnionSolid *, G4Tubs *> getSolids();
    void runBeamOnFlasher(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);

    /**
     * @brief This is needed to access to the flasher position and rotation
     */
    void setNavigator(G4Navigator *pNavigator) { mNavigator = pNavigator; } 

private:
    void makeSolids();
    void makeLogicalVolumes();
    void readFlasherProfile();
    GlobalPosition getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);
    void configureGPS(GlobalPosition flasherInfo);

    G4UnionSolid *mLEDSolid;
    G4UnionSolid *mFlasherHoleSolid;
    G4Tubs *mGlassWindowSolid;

    G4LogicalVolume *mFlasherHoleLogical;
    G4LogicalVolume *mGlassWindowLogical;
    G4LogicalVolume *mLEDLogical;
    G4bool mFlasherProfileAvailable = false;
    std::vector<double> mProfileX;
    std::vector<double> mProfileY;
    G4Navigator *mNavigator;
};

#endif // OMSimMDOMFlasher_H
