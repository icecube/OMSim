/**
 * @file OMSimMDOMFlasher.hh
 * @brief Defines the mDOMFlasher class for simulating the 10 flashers in an mDOM optical module.
 * 
 * @details
 * This file contains the declaration of the mDOMFlasher class and related structures.
 * mDOMFlasher is responsible for creating the flashers in the mDOM, which includes a LED, 
 * the air around it, and a glass window on top. The class provides functionalities for constructing 
 * the solids and logical volumes, retrieving them, and simulating the LED flashing based on a given profile.
 *
 * @author Anna-Sophia Tenbruck, Cristian Lozano, Martin Unland
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

    /**
     * @brief run/beamOn the specified flasher.
     * @details This method triggers the flasher at the given index in the specified module.
     * @param pMDOMInstance The mDOM instance to access to the placement OM positions and orientations.
     * @param pModuleIndex The index of the module to be flashed (if only one mDOM placed, then 0, otherwise depending on placeIt() order)
     * @param pLEDIndex The index of the flasher within the module.
     */
    void runBeamOnFlasher(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);

    /**
     * @brief This is needed to access to the flasher position and rotation
     */
    void setNavigator(G4Navigator *pNavigator) { mNavigator = pNavigator; } 

private:
    void makeSolids();
    void makeLogicalVolumes();

    /**
     * @brief Loads profile of mDOM flasher measured in the lab. See Section 7.3 of C. Lozanos PhD Thesis: <https://doi.org/10.5281/zenodo.8107177> or Anna-Sophia's Bachelor thesis (german) <https://www.uni-muenster.de/imperia/md/content/physik_kp/agkappes/abschlussarbeiten/bachelorarbeiten/ba_tenbruck.pdf>.
     */
    void readFlasherProfile();

    /**
     * @brief Retrieves the global position and orientation of a specific flasher in an mDOM module.
     * @param pMDOMInstance Reference to the mDOM instance, which contains the placement details of the module.
     * @param pModuleIndex Index of the module.
     * @param pLEDIndex Index of the flasher within the module.
     * @return The global position and orientation of the flasher, represented by a GlobalPosition struct.
     */
    GlobalPosition getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex);

    /**
     * @brief Configures the GPS for the flasher simulation.
     * @param flasherInfo The position and orientation information of the flasher.
     */
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
