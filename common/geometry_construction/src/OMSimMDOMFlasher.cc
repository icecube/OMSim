#include "OMSimMDOM.hh"
#include "OMSimUIinterface.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimTools.hh"
#include "G4TouchableHistoryHandle.hh"
#include "G4TransportationManager.hh"
#include <G4Cons.hh>
#include <G4Ellipsoid.hh>

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                Geometry methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

mDOMFlasher::mDOMFlasher(): abcDetectorComponent() 
{
	construction();
}

void mDOMFlasher::construction()
{
	makeSolids();
	makeLogicalVolumes();

	//  Placement LED itself
	new G4PVPlacement(0, G4ThreeVector(0, 0, -1.7 * mm), mLEDLogical, "LED_physical", mFlasherHoleLogical, false, 0, mCheckOverlaps); // LED to glass 1mm: -1.7mm; LED to glass 0.6mm: -1.1mm
	// Placement LED glass top between air (where LED is) and gel
	new G4PVPlacement(0, G4ThreeVector(0, 0, 4.4 * mm + 0.875 * mm), mGlassWindowLogical, "LED_Glass top_physical", mFlasherHoleLogical, false, 0, mCheckOverlaps);

	appendComponent(mFlasherHoleSolid, mFlasherHoleLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Flasher hole");
	mFlasherHoleLogical->SetVisAttributes(mInvisibleVis);
	mLEDLogical->SetVisAttributes(mLEDvis);
	mGlassWindowLogical->SetVisAttributes(mGlassVis);
}

void mDOMFlasher::makeSolids()
{

	// NOTE: This should not be necessary (substraction with lAirSolid should be enough), but somehow a visual glitch is visible otherwise
	G4Cons *lCutTubeHolderInnerSolid = new G4Cons("cut inner solid", 0, 2.65 * mm, 0, 4.288 * mm, 4.4 * mm, 0, 2 * CLHEP::pi);											  // inner cone
	G4Tubs *lCutTubeHolderOuterSolid = new G4Tubs("cut outer solid", 0, 7.25 * mm, 2 * 0.667 * mm, 0, 2 * CLHEP::pi);													  // outer cone
	G4UnionSolid *lCutTubeHolderSolid = new G4UnionSolid("cut solid", lCutTubeHolderInnerSolid, lCutTubeHolderOuterSolid, 0, G4ThreeVector(0, 0, 4.4 * mm + 0.667 * mm)); // union

	// extra cone only(!) for air between LED and holding structure; fits exactly in cut in holding structure
	G4Cons *lAirInnerSolid = new G4Cons("air inner solid", 0, 2.65 * mm, 0, 4.288 * mm, 4.4 * mm, 0, 2 * CLHEP::pi);				  // inner cone
	G4Tubs *lAirOuterSolid = new G4Tubs("air outer solid", 0, 7.25 * mm, 0.875 * mm, 0, 2 * CLHEP::pi);								  // outer cone
	mFlasherHoleSolid = new G4UnionSolid("air Solid", lAirInnerSolid, lAirOuterSolid, 0, G4ThreeVector(0, 0, 4.4 * mm + 0.875 * mm)); // union

	// glass on top of LED
	mGlassWindowSolid = new G4Tubs("LED Glass top solid", 0, 7.25 * mm, 0.875 * mm, 0, 2 * CLHEP::pi); // glass thickness is 1.75

	// LED itself
	G4Ellipsoid *lLEDTopSolid = new G4Ellipsoid("LEDTop solid", 0.25 * cm, 0.25 * cm, 0.25 * cm, 0, 0.25 * cm); // LED diameter=2.5mm
	G4Tubs *lLEDCylSolid = new G4Tubs("LEDCyl solid", 0, 0.25 * cm, 0.26 * cm, 0, 2 * CLHEP::pi);
	mLEDSolid = new G4UnionSolid("temp", lLEDCylSolid, lLEDTopSolid, 0, G4ThreeVector(0, 0, 0.26 * cm));
}

void mDOMFlasher::makeLogicalVolumes()
{
	mFlasherHoleLogical = new G4LogicalVolume(mFlasherHoleSolid,
											  mData->getMaterial("Ri_Vacuum"),
											  "LED hole");
	mGlassWindowLogical = new G4LogicalVolume(mGlassWindowSolid,
											  mData->getMaterial("RiAbs_Glass_Vitrovex"),
											  "LED Glass top logical");
	mLEDLogical = new G4LogicalVolume(mLEDSolid,
									  mData->getMaterial("Ri_Glass_LED"),
									  "LED logical");
}

std::tuple<G4UnionSolid *, G4UnionSolid *, G4Tubs *> mDOMFlasher::getSolids()
{
	return std::make_tuple(mLEDSolid, mFlasherHoleSolid, mGlassWindowSolid);
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                GPS flashing methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 * @brief run/beamOn the specified flasher.
 * @details This method triggers the flasher at the given index in the specified module.
 * @param pMDOMInstance The mDOM instance to access to the placement OM positions and orientations.
 * @param pModuleIndex The index of the module to be flashed (if only one mDOM placed, then 0, otherwise depending on placeIt() order)
 * @param pLEDIndex The index of the flasher within the module.
 */
void mDOMFlasher::runBeamOnFlasher(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex)
{
	if (!mFlasherProfileAvailable)
	{
		readFlasherProfile();
	}

	// Get Flasher and Rotation Info
	auto lFlasherInfo = getFlasherPositionInfo(pMDOMInstance, pModuleIndex, pLEDIndex);

	// Configure the GPS
	configureGPS(lFlasherInfo);

	// Flash the LED
	OMSimUIinterface::getInstance().runBeamOn();
}

/**
 * @brief Loads profile of mDOM flasher measured in the lab. See Section 7.3 of C. Lozanos PhD Thesis: <https://doi.org/10.5281/zenodo.8107177> or Anna-Sophia's Bachelor thesis (german) <https://www.uni-muenster.de/imperia/md/content/physik_kp/agkappes/abschlussarbeiten/bachelorarbeiten/ba_tenbruck.pdf>.
 */
void mDOMFlasher::readFlasherProfile()
{
	std::vector<G4PV2DDataVector> lData = Tools::loadtxt("../common/data/UserInputData/processedlightspectrum_9_level_2_ext3.cfg", true, 0, '\t');
	mProfileX = lData.at(0);
	mProfileY = lData.at(1);
	mFlasherProfileAvailable = true;
}

/**
 * @brief Retrieves the global position and orientation of a specific flasher in an mDOM module.
 * @param pMDOMInstance Reference to the mDOM instance, which contains the placement details of the module.
 * @param pModuleIndex Index of the module.
 * @param pLEDIndex Index of the flasher within the module.
 * @return The global position and orientation of the flasher, represented by a GlobalPosition struct.
 */
GlobalPosition mDOMFlasher::getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex)
{
	GlobalPosition lGlobalPos;

	if (pModuleIndex < 0 || pModuleIndex >= pMDOMInstance->mPlacedOrientations.size())
	{
		// throw an exception or return an error
		log_critical("mDOM index provided not valid");
		throw std::invalid_argument("mDOM index provided not valid");
	}

	// check if the provided LED index is valid
	if (pLEDIndex < 0 || pLEDIndex >= mPlacedPositions.size())
	{
		// throw an exception or return an error
		log_critical("mDOM Flasher index provided not valid");
		throw std::invalid_argument("mDOM Flasher index provided not valid");
	}

	// Calculate global position
	G4RotationMatrix lFlashingModuleOrientation = pMDOMInstance->mPlacedOrientations.at(pModuleIndex);
	G4ThreeVector lFlashingModulePos = pMDOMInstance->mPlacedPositions.at(pModuleIndex);
	G4ThreeVector lFlasherLocalThreeVector = mPlacedPositions.at(pLEDIndex);
	G4ThreeVector lFlasherGlobalPosition = getNewPosition(lFlashingModulePos, lFlashingModuleOrientation, lFlasherLocalThreeVector, G4RotationMatrix()).getTranslation();
	lGlobalPos.x = lFlasherGlobalPosition.getX();
	lGlobalPos.y = lFlasherGlobalPosition.getY();
	lGlobalPos.z = lFlasherGlobalPosition.getZ();

	// Get rotation of the flasher
	G4Navigator* lNavigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
	lNavigator->LocateGlobalPointAndSetup(G4ThreeVector(lGlobalPos.x, lGlobalPos.y, lGlobalPos.z));
	G4TouchableHistoryHandle lTouchable = lNavigator->CreateTouchableHistoryHandle();
	lGlobalPos.rotation = lTouchable->GetRotation()->inverse();

	return lGlobalPos;
}

/**
 * @brief Configures the GPS for the flasher simulation.
 * @param flasherInfo The position and orientation information of the flasher.
 */
void mDOMFlasher::configureGPS(GlobalPosition pGlobalPos)
{
	OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

	lUIinterface.applyCommand("/gps/pos/type Point");
	lUIinterface.applyCommand("/gps/ang/type user");   // biast = theta
	lUIinterface.applyCommand("/gps/hist/type theta"); // biast = theta

	for (unsigned int u = 0; u < mProfileX.size(); u++)
	{
		lUIinterface.applyCommand("/gps/hist/point", mProfileX.at(u), mProfileY.at(u)); // biast = theta
	}

	lUIinterface.applyCommand("/gps/ang/surfnorm");

	lUIinterface.applyCommand("/gps/pos/rot1", pGlobalPos.rotation.xx(), pGlobalPos.rotation.yx(), pGlobalPos.rotation.zx());
	lUIinterface.applyCommand("/gps/ang/rot1", pGlobalPos.rotation.xx(), pGlobalPos.rotation.yx(), pGlobalPos.rotation.zx());

	lUIinterface.applyCommand("/gps/pos/rot2", -pGlobalPos.rotation.xy(), -pGlobalPos.rotation.yy(), -pGlobalPos.rotation.zy());
	lUIinterface.applyCommand("/gps/ang/rot2", -pGlobalPos.rotation.xy(), -pGlobalPos.rotation.yy(), -pGlobalPos.rotation.zy());
	lUIinterface.applyCommand("/gps/ang/maxtheta 89 deg"); // when too close to 90, give photons that directly hit the structure and do not propagate... photons with theta=90 are anyway weighed very low

	lUIinterface.applyCommand("/gps/pos/centre", pGlobalPos.x, pGlobalPos.y, pGlobalPos.z, "mm");
	lUIinterface.applyCommand("/gps/particle opticalphoton");
	lUIinterface.applyCommand("/gps/energy", 1239.84193 / OMSimCommandArgsTable::getInstance().get<G4double>("wavelength"), "eV");
}
