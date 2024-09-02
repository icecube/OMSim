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

#pragma once

#include "OMSimDetectorComponent.hh"
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
class mDOMFlasher : public OMSimDetectorComponent
{
public:
    mDOMFlasher();
    void construction();
    std::tuple<G4UnionSolid *, G4UnionSolid *, G4Tubs *> getSolids();
    void runBeamOnFlasher(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);


private:
    void makeSolids();
    void makeLogicalVolumes();
    void readFlasherProfile();
    GlobalPosition getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);
    void configureGPS(GlobalPosition flasherInfo);

    G4UnionSolid *m_LEDSolid;
    G4UnionSolid *m_flasherHoleSolid;
    G4Tubs *m_glassWindowSolid;

    G4LogicalVolume *m_flasherHoleLogical;
    G4LogicalVolume *m_glassWindowLogical;
    G4LogicalVolume *m_LEDLogical;
    G4bool m_flasherProfileAvailable = false;
    std::vector<double> m_profileX;
    std::vector<double> m_profileY;
};

