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

mDOMFlasher::mDOMFlasher(): OMSimDetectorComponent() 
{
	construction();
}

void mDOMFlasher::construction()
{
	makeSolids();
	makeLogicalVolumes();

	//  Placement LED itself
	new G4PVPlacement(0, G4ThreeVector(0, 0, -1.7 * mm), m_LEDLogical, "LED_physical", m_flasherHoleLogical, false, 0, m_checkOverlaps); // LED to glass 1mm: -1.7mm; LED to glass 0.6mm: -1.1mm
	// Placement LED glass top between air (where LED is) and gel
	new G4PVPlacement(0, G4ThreeVector(0, 0, 4.4 * mm + 0.875 * mm), m_glassWindowLogical, "LED_Glass top_physical", m_flasherHoleLogical, false, 0, m_checkOverlaps);

	appendComponent(m_flasherHoleSolid, m_flasherHoleLogical, G4ThreeVector(0, 0, 0), G4RotationMatrix(), "Flasher hole");
	m_flasherHoleLogical->SetVisAttributes(m_invisibleVis);
	m_LEDLogical->SetVisAttributes(m_LEDvis);
	m_glassWindowLogical->SetVisAttributes(m_glassVis);
}

void mDOMFlasher::makeSolids()
{

	// NOTE: This should not be necessary (substraction with lAirSolid should be enough), but somehow a visual glitch is visible otherwise
	G4Cons *cutTubeHolderInnerSolid = new G4Cons("cut inner solid", 0, 2.65 * mm, 0, 4.288 * mm, 4.4 * mm, 0, 2 * CLHEP::pi);											  // inner cone
	G4Tubs *cutTubeHolderOuterSolid = new G4Tubs("cut outer solid", 0, 7.25 * mm, 2 * 0.667 * mm, 0, 2 * CLHEP::pi);													  // outer cone
	G4UnionSolid *cutTubeHolderSolid = new G4UnionSolid("cut solid", cutTubeHolderInnerSolid, cutTubeHolderOuterSolid, 0, G4ThreeVector(0, 0, 4.4 * mm + 0.667 * mm)); // union

	// extra cone only(!) for air between LED and holding structure; fits exactly in cut in holding structure
	G4Cons *airInnerSolid = new G4Cons("air inner solid", 0, 2.65 * mm, 0, 4.288 * mm, 4.4 * mm, 0, 2 * CLHEP::pi);				  // inner cone
	G4Tubs *airOuterSolid = new G4Tubs("air outer solid", 0, 7.25 * mm, 0.875 * mm, 0, 2 * CLHEP::pi);								  // outer cone
	m_flasherHoleSolid = new G4UnionSolid("air Solid", airInnerSolid, airOuterSolid, 0, G4ThreeVector(0, 0, 4.4 * mm + 0.875 * mm)); // union

	// glass on top of LED
	m_glassWindowSolid = new G4Tubs("LED Glass top solid", 0, 7.25 * mm, 0.875 * mm, 0, 2 * CLHEP::pi); // glass thickness is 1.75

	// LED itself
	G4Ellipsoid *topLEDSolid = new G4Ellipsoid("LEDTop solid", 0.25 * cm, 0.25 * cm, 0.25 * cm, 0, 0.25 * cm); // LED diameter=2.5mm
	G4Tubs *cylinderLEDsolid = new G4Tubs("LEDCyl solid", 0, 0.25 * cm, 0.26 * cm, 0, 2 * CLHEP::pi);
	m_LEDSolid = new G4UnionSolid("temp", cylinderLEDsolid, topLEDSolid, 0, G4ThreeVector(0, 0, 0.26 * cm));
}

void mDOMFlasher::makeLogicalVolumes()
{
	m_flasherHoleLogical = new G4LogicalVolume(m_flasherHoleSolid,
											  m_data->getMaterial("Ri_Vacuum"),
											  "LED hole");
	m_glassWindowLogical = new G4LogicalVolume(m_glassWindowSolid,
											  m_data->getMaterial("RiAbs_Glass_Vitrovex"),
											  "LED Glass top logical");
	m_LEDLogical = new G4LogicalVolume(m_LEDSolid,
									  m_data->getMaterial("Ri_Glass_LED"),
									  "LED logical");
}

std::tuple<G4UnionSolid *, G4UnionSolid *, G4Tubs *> mDOMFlasher::getSolids()
{
	return std::make_tuple(m_LEDSolid, m_flasherHoleSolid, m_glassWindowSolid);
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                GPS flashing methods
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/**
 * @brief run/beamOn the specified flasher.
 * @details This method triggers the flasher at the given index in the specified module.
 * @param p_instanceMDOM The mDOM instance to access to the placement OM positions and orientations.
 * @param p_moduleIndex The index of the module to be flashed (if only one mDOM placed, then 0, otherwise depending on placeIt() order)
 * @param p_LEDIndex The index of the flasher within the module.
 */
void mDOMFlasher::runBeamOnFlasher(mDOM *p_instanceMDOM, G4int p_moduleIndex, G4int p_LEDIndex)
{
	if (!m_flasherProfileAvailable)
	{
		readFlasherProfile();
	}

	// Get Flasher and Rotation Info
	auto flasherInfo = getFlasherPositionInfo(p_instanceMDOM, p_moduleIndex, p_LEDIndex);

	// Configure the GPS
	configureGPS(flasherInfo);

	// Flash the LED
	OMSimUIinterface::getInstance().runBeamOn();
}

/**
 * @brief Loads profile of mDOM flasher measured in the lab. See Section 7.3 of C. Lozanos PhD Thesis: <https://doi.org/10.5281/zenodo.8107177> or Anna-Sophia's Bachelor thesis (german) <https://www.uni-muenster.de/imperia/md/content/physik_kp/agkappes/abschlussarbeiten/bachelorarbeiten/ba_tenbruck.pdf>.
 */
void mDOMFlasher::readFlasherProfile()
{
	std::vector<G4PV2DDataVector> lData = Tools::loadtxt("../common/data/UserInputData/processedlightspectrum_9_level_2_ext3.cfg", true, 0, '\t');
	m_profileX = lData.at(0);
	m_profileY = lData.at(1);
	m_flasherProfileAvailable = true;
}

/**
 * @brief Retrieves the global position and orientation of a specific flasher in an mDOM module.
 * @param p_instanceMDOM Reference to the mDOM instance, which contains the placement details of the module.
 * @param p_moduleIndex Index of the module.
 * @param p_LEDIndex Index of the flasher within the module.
 * @return The global position and orientation of the flasher, represented by a GlobalPosition struct.
 */
GlobalPosition mDOMFlasher::getFlasherPositionInfo(mDOM *p_instanceMDOM, G4int p_moduleIndex, G4int p_LEDIndex)
{
	GlobalPosition globalPosition;

	if (p_moduleIndex < 0 || p_moduleIndex >= p_instanceMDOM->m_placedOrientations.size())
	{
		// throw an exception or return an error
		log_critical("mDOM index provided not valid");
		throw std::invalid_argument("mDOM index provided not valid");
	}

	// check if the provided LED index is valid
	if (p_LEDIndex < 0 || p_LEDIndex >= m_placedPositions.size())
	{
		// throw an exception or return an error
		log_critical("mDOM Flasher index provided not valid");
		throw std::invalid_argument("mDOM Flasher index provided not valid");
	}

	// Calculate global position
	G4RotationMatrix flashingModuleOrientation = p_instanceMDOM->m_placedOrientations.at(p_moduleIndex);
	G4ThreeVector flashingModulePosition = p_instanceMDOM->m_placedPositions.at(p_moduleIndex);
	G4ThreeVector flasherLocalVector = m_placedPositions.at(p_LEDIndex);
	G4ThreeVector flasherGlobalVector = getNewPosition(flashingModulePosition, flashingModuleOrientation, flasherLocalVector, G4RotationMatrix()).getTranslation();
	globalPosition.x = flasherGlobalVector.getX();
	globalPosition.y = flasherGlobalVector.getY();
	globalPosition.z = flasherGlobalVector.getZ();

	// Get rotation of the flasher
	G4Navigator* navigator = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking();
	navigator->LocateGlobalPointAndSetup(G4ThreeVector(globalPosition.x, globalPosition.y, globalPosition.z));
	G4TouchableHistoryHandle touchable = navigator->CreateTouchableHistoryHandle();
	globalPosition.rotation = touchable->GetRotation()->inverse();

	return globalPosition;
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

	for (unsigned int u = 0; u < m_profileX.size(); u++)
	{
		lUIinterface.applyCommand("/gps/hist/point", m_profileX.at(u), m_profileY.at(u)); // biast = theta
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
