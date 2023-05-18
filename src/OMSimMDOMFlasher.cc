#include "OMSimMDOMFlasher.hh"
#include "OMSimMDOM.hh"
#include "abcDetectorComponent.hh"

#include "G4Cons.hh"
#include "G4Ellipsoid.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4SystemOfUnits.hh"
#include "G4Transform3D.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include "G4VisAttributes.hh"
#include "OMSimLogger.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimUIinterface.hh"

mDOMFlasher::mDOMFlasher(InputDataManager *pData)
{
	mData = pData;
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

void mDOMFlasher::readFlasherProfile()
{
	std::vector<double> led_profile_x;
	std::vector<double> led_profile_y;
	// led_profile_x = readColumnDouble(glightprofile_file, 1);
	// led_profile_y = readColumnDouble(glightprofile_file, 2);
}

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
}

FlasherPositionInfo mDOMFlasher::getFlasherPositionInfo(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex)
{
	FlasherPositionInfo info;
	info.orientation = pMDOMInstance->mPlacedOrientations.at(pModuleIndex);
	std::vector<std::vector<G4double>> lLED_AngFromSphere = pMDOMInstance->mLED_AngFromSphere;
	// check if the provided LED index is valid
	if (pLEDIndex < 0 || pLEDIndex >= lLED_AngFromSphere.size())
	{
		// throw an exception or return an error
		log_critical("mDOM Flasher index provided not valid");
		throw std::invalid_argument("mDOM Flasher index provided not valid");
	}

	// Access LED angles from sphere
	info.phi = pMDOMInstance->mLED_AngFromSphere.at(pLEDIndex).at(2);
	info.theta = pMDOMInstance->mLED_AngFromSphere.at(pLEDIndex).at(1);
	info.rho = pMDOMInstance->mLED_AngFromSphere.at(pLEDIndex).at(0);

	// Calculate global position
	G4RotationMatrix lFlashingModuleOrientation = pMDOMInstance->mPlacedOrientations.at(pModuleIndex);
	G4ThreeVector lFlashingModulePos = pMDOMInstance->mPlacedPositions.at(pModuleIndex);
	G4ThreeVector lFlasherLocalThreeVector = mPlacedPositions.at(pLEDIndex);
	G4Transform3D lFlasherGlobalPosition = getNewPosition(lFlashingModulePos, lFlashingModuleOrientation, lFlasherLocalThreeVector, G4RotationMatrix());

	// Store global position in info
	info.globalPosition = lFlasherGlobalPosition.getTranslation();

	return info;
}

G4ThreeVector mDOMFlasher::getFlasherGlobalThreeVector(mDOM *pMDOMInstance, G4int pModuleIndex, G4int pLEDIndex, G4RotationMatrix orientation)
{
	G4ThreeVector lFlashingModulePos = pMDOMInstance->mPlacedPositions.at(pModuleIndex);

	// get the positions like: position of the LED + position of the module with the corresponding rotation matrix
	G4ThreeVector lFlasherLocalThreeVector = mPlacedPositions.at(pLEDIndex);
	G4Transform3D lFlasherGlobalPosition = getNewPosition(lFlashingModulePos, orientation, lFlasherLocalThreeVector, G4RotationMatrix());

	return lFlasherGlobalPosition.getTranslation();
}

void mDOMFlasher::configureGPS(FlasherPositionInfo flasherInfo)
{	OMSimUIinterface &lUIinterface = OMSimUIinterface::getInstance();

	lUIinterface.applyCommand("/gps/pos/type Point");
	lUIinterface.applyCommand("/gps/ang/type user");	  // biast = theta
	lUIinterface.applyCommand("/gps/hist/type theta"); // biast = theta

	// for (unsigned int u = 0; u < led_profile_x.size(); u++)
	// {
	// 	applyCommand("/gps/hist/point", led_profile_x.at(u), led_profile_y.at(u)); // biast = theta
	// }

	lUIinterface.applyCommand("/gps/ang/surfnorm");

	// build angles rot1 and rot2
	G4ThreeVector rot1Vector = buildRotVector(flasherInfo.phi, 0, flasherInfo.orientation);
	lUIinterface.applyCommand("/gps/pos/rot1", rot1Vector.getX(), rot1Vector.getY(), rot1Vector.getZ());
	lUIinterface.applyCommand("/gps/ang/rot1", rot1Vector.getX(), rot1Vector.getY(), rot1Vector.getZ());

	G4ThreeVector rot2Vector = buildRotVector(flasherInfo.phi, flasherInfo.theta, flasherInfo.orientation);
	lUIinterface.applyCommand("/gps/pos/rot2", rot2Vector.getX(), rot2Vector.getY(), rot2Vector.getZ());
	lUIinterface.applyCommand("/gps/ang/rot2", rot2Vector.getX(), rot2Vector.getY(), rot2Vector.getZ());
	lUIinterface.applyCommand("/gps/ang/maxtheta 89 deg"); // when too close to 90, give photons that directly hit the structure and do not propagate... photons with theta=90 are anyway weighed very low

	lUIinterface.applyCommand("/gps/pos/centre", flasherInfo.globalPosition.getX(), flasherInfo.globalPosition.getY(), flasherInfo.globalPosition.getZ(), "mm");
	lUIinterface.applyCommand("/gps/particle opticalphoton");
	lUIinterface.applyCommand("/gps/energy", 1239.84193 / OMSimCommandArgsTable::getInstance().get<G4double>("wavelength"), "eV");
}

G4ThreeVector mDOMFlasher::buildRotVector(G4double phi, G4double theta, G4RotationMatrix orientation)
{
	G4double xPrime, yPrime, zPrime;

	if (theta == 0)
	{
		xPrime = -sin(phi);
		yPrime = cos(phi);
		zPrime = 0;
	}
	else
	{
		xPrime = cos(theta) * cos(phi);
		yPrime = cos(theta) * sin(phi);
		zPrime = -sin(theta);
	}

	// Rotate this vector with the proper module rotation (no translation this time!)
	G4ThreeVector rotVector = G4ThreeVector(xPrime, yPrime, zPrime);
	G4Transform3D rotTrans = getNewPosition(G4ThreeVector(), orientation, rotVector, G4RotationMatrix());
	return rotTrans.getTranslation();
}


