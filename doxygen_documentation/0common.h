/**
 *  @defgroup common OMSim framework and geometry
 *  @brief These files define the geometry and materials common to all studies.
 *
 *  @subsection framework OMSim framework
 *
 *  @subsubsection data Materials and User Data
 *  User-defined material data are stored in JSON files under `/common/data` to minimize file length (the original detector construction had over 4,000 lines, mainly due to material properties!).
 *  The `InputDataManager` (@ref OMSimInputData.hh) loads these properties directly into the Geant4 framework. Materials loaded via this class can be retrieved using Geant4's conventional method `G4Material::GetMaterial`, but the framework also provides the wrapper `InputDataManager::getMaterial` to handle default parameters.
 *  The class also provides an analogue method for optical surfaces `InputDataManager::getOpticalSurface` which does not exists in Geant4.
 * 
 *  Since different materials have different type of properties, the data is loaded in multiple ways. These different material types are defined in @ref OMSimDataFileTypes.hh.
 *  Additionally, geometry data used during PMT construction are also stored in JSON files (`/common/data/PMTs`). These are saved in a "tree" (essentially a dictionary containing the JSON file's keys and values) in `InputDataManager::mTable`. 
 *  This approach was adopted because various PMTs are constructed similarly, eliminating the need to define a unique class for each PMT type, as is done for the optical modules. 
 *  Thus, an instance of `InputDataManager` is passed always to all classes related to geometry construction (see @link geometry @endlink).
 *
 *  If you wish to load additional data, you can either define a new type in OMSimDataFileTypes or load it into a tree as previously mentioned. For simpler tasks, use the static method `InputDataManager::loadtxt`, which operates similarly to Python's numpy.loadtxt. For example:
 *  ~~~~~~~~~~~~~{.cc}
 *  std::vector<G4PV2DDataVector> data = InputDataManager::loadtxt("path/file_name.txt");
 *  std::vector<G4double> first_column = data.at(0);
 *  std::vector<G4double> second_column = data.at(1);
 *  // ...
 *  ~~~~~~~~~~~~~
 *
 *  @subsubsection geometry Geometry construction
 * 
 *  Each optical module and harness is defined in its respective class, located in the `/common/geometry_construction/` folder. These inherit from the virtual base class `OMSimOpticalModule`. 
 *  This inheritance ensures the definition of functions to retrieve the pressure vessel's weight (necessary for radioactive decay studies) and the count of PMTs inside the module. The `OMSimOpticalModule` interface inherits from `abcDetectorComponent`, a general helper class simplifying construction. Most OMSim geometries inherit from this class, as illustrated in the dependency diagram below:
 *  \htmlonly
 *  <div class="center">
 *      <img src="classabc_detector_component__inherit__graph.svg" 
 *           width="747" height="187" alt="Inheritance diagram">
 *  </div>
 *  \endhtmlonly
 * 
 *  The `OMSimPMTConstruction` class constructs PMTs. There are two PMT construction approaches. The first is simple, with a solid photocathode where all entering photons are recorded. The second type simulates the photocathode as a thin layer, also representing the internal components accounting for internal reflections. For more information, refer to Chapter 9 of <a href="https://zenodo.org/record/8121321">Martin Unland's thesis</a>.
 *  The construction of different PMT models (e.g. the 3'' or 10'' PMTs) is quite similar. However, the frontal window shape varies among models, leading to diverse combinations of ellipsoids and spheres.
 * 
 * <div style="width: 100%; text-align: center;">
 * <img src="simulation_vs_sketch_old.png" width="280" height="320" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 1: <i>Cross section of simple mDOM PMT model. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 * <div style="width: 100%; text-align: center;">
 * <img src="simulation_dynodes_screenshot.png" width="400" height="328" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 2: <i>Side view of complex mDOM PMT model. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 * In the complex PMT model the photocathode has an absorption length that matches the measured quantum efficiency of the mDOM PMTs. For the other PMT models this matching has to be still be performed.
 * 
 * <div style="width: 100%; text-align: center;">
 * <img src="QE_meas_VS_simulation.png" width="360" height="308" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 3: <i>QE of simulation with the absorption length currently used compared to measurements. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 * 
 * @subsubsection response PMT Response and Saving Hits
 * 
 * @warning Only the mDOM PMT currently supports a detailed PMT response.
 * 
 * For photon detection in both simple and complex geometries, the photons must be absorbed within the photocathode volume. The PMTs' photocathodes are made sensitive through the OMSimSensitiveDetector class, following Geant4's G4VSensitiveDetector pattern. This configuration is achieved by invoking `OMSimOpticalModule::configureSensitiveVolume` (or `OMSimPMTConstruction::configureSensitiveVolume` when simulating a single PMT). 
 * It is essential to invoke this method in the detector construction, as it needs the instance of `OMSimDetectorConstruction` to call `G4VUserDetectorConstruction::SetSensitiveDetector` for successful operation in Geant4 (refer to `OMSimDetectorConstruction::setSensitiveDetector`).
 * 
 * Every particle passage through the photocathode triggers the `OMSimSensitiveDetector::ProcessHits` method. It verifies if the particle is a photon and whether it was absorbed. For a deeper understanding of Geant4's philosophy concerning G4VSensitiveDetector, consult the <a href="https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/hit.html?highlight=g4vsensitivedetector#g4vsensitivedetector">Geant4 guide for application developers</a>.
 * 
 * In `OMSimPMTConstruction::configureSensitiveVolume`, PMTs are associated with an instance of OMSimPMTResponse, contingent on the PMT under simulation. This class offers a precise PMT simulation by sampling from real measurements, obtaining the relative transit time, charge (in PE), and detection probability (using the measured scans from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>). For detailed insights, refer to Section 9.3.4 of the linked thesis.
 * 
 * The data of absorbed photons is managed by the `OMSimHitManager` singleton. To analyse and export this data, consult example methods like `OMSimEffectiveAreaAnalyisis::writeScan` and `OMSimDecaysAnalysis::writeHitInformation` for direction.
 * 
 *  <div style="width: 100%; text-align: center;">
 * <img src="PW_beam_geant4_TT.png" width="256" height="440" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 4: <i>PMT response compared to measurement for different light sources. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 * An additional feature allows for the direct application of a QE cut. This ensures that only absorbed photons passing the QE test are retained in `OMSimHitManager`. To enable this feature, provide the "QE_cut" argument via the command line.
 * 
 * @note In most scenarios, it's not recommended to use --QE_cut since it reduces your statistics. Its presence in OMSim is primarily for testing purposes. It's generally better to perform post-analysis using the saved `OMSimPMTResponse::PMTPulse::DetectionProbability` for each absorbed photon.
 * 
 * @subsubsection dummyresponse Making other volumes sensitive to photons
 * 
 * For some studies you might want a volume to detect photons, without this being necessarely a PMT.
 * For such cases, the framework has a provision in place: use the `OMSimSensitiveDetector` and pass `DetectorType::GeneralPhotonDetector` as its constructor argument.
 * In this case the `OMSimSensitiveDetector::ProcessHits` will use an instance `NoResponse` as PMT response, which is just a dummy placeholder.  
 * Remember to inform the `HitManager` that this detector is equivalent to "a single PMT", ensuring internal vector sizes adjust accordingly.
 * This approach ensures a smooth integration of the photosensitive volume within the current system. Below is an example illustrating how this can be incorporated within the detector construction:
 * 
 * ~~~~~~~~~~~~~{.cc}
 *  #include "OMSimSensitiveDetector.hh"
 *  //... Assume the logical volume of your detector has been defined and is referred to as "lDetectorLV"
 *  OMSimSensitiveDetector* lSensitiveDetector = new OMSimSensitiveDetector("myDetector", DetectorType::GeneralPhotonDetector); 
 *  OMSimHitManager lHitManager =  OMSimHitManager::getInstance();
 *  lHitManager.setNumberOfPMTs(1, lHitManager.getNextDetectorIndex());
 *  setSensitiveDetector(lDetectorLV, lSensitiveDetector);
 * ~~~~~~~~~~~~~
 * In this case, `OMSimSensitiveDetector::ProcessHits` will store all absorbed photons. The number of photons absorbed will depend on the absorption length of the material connected to the logical volume. 
 * If there's a need to make a volume sensitive to particles other than photons, add a new entry to the `DetectorType` enum (in `OMSimSensitiveDetector.hh`) and incorporate a new method that handles this scenario in `OMSimSensitiveDetector::ProcessHits`. 
 * You might also track these particles in `OMSimTrackingAction` or `OMSimSteppingAction`, but using a class derived from `G4VSensitiveDetector` aligns with the philosophy of Geant4.
 */

