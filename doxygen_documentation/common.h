/**
 *  @defgroup common OMSim framework and geometry
 *  @brief These files define the geometry and materials common to all studies.
 *
 *  @subsection framework OMSim framework
 *
 *  @subsubsection data Materials and User Data
 *  User-defined material data are stored in JSON files under /common/data to minimize file length (the original detector construction had over 4,000 lines, mainly due to material properties!).
 *  The InputDataManager (@ref OMSimInputData.hh) loads these properties into the Geant4 framework. They can be retrieved using the InputDataManager::getMaterial and InputDataManager::getOpticalSurface methods.
 *  Since different materials have various properties, the data is loaded in multiple ways. These different material types are defined in @ref OMSimDataFileTypes.hh.
 *  Additionally, geometry data for the PMTs are also stored in JSON files. These are parsed directly into a "tree" (essentially a dictionary containing the JSON file's keys and values). This method was adopted because various PMTs are constructed similarly, eliminating the need to define a unique class for each PMT type, as is done for the optical modules.
 *
 *  If you wish to load additional data, you can either define a new type in OMSimDataFileTypes or load it into a tree as previously mentioned. For simpler tasks, use the static method InputDataManager::loadtxt, which operates similarly to Python's numpy.loadtxt. For example:
 *  ~~~~~~~~~~~~~{.cc}
 *  std::vector<G4PV2DDataVector> data = InputDataManager::loadtxt("path/file_name.txt");
 *  std::vector<G4double> first_column = data.at(0);
 *  std::vector<G4double> second_column = data.at(1);
 *  // ...
 *  ~~~~~~~~~~~~~
 *
 *  @subsubsection geometry Geometry
 * 
 *  Each optical module and harness is defined in its respective class, located in the /common/optical_modules/ folder. These inherit from the virtual base class OpticalModule. This inheritance ensures the definition of functions to retrieve the pressure vessel's weight (necessary for radioactive decay studies) and the count of PMTs inside the module. The OpticalModule interface inherits from abcDetectorComponent, a general helper class simplifying construction. Most OMSim geometries inherit from this class, as illustrated in the dependency diagram below:
 *  \htmlonly
 *  <div class="center">
 *      <img src="classabc_detector_component__inherit__graph.svg" 
 *           width="747" height="187" alt="Inheritance diagram">
 *  </div>
 *  \endhtmlonly
 *  The OMSimPMTConstruction in /common/framework constructs PMTs. There are two PMT construction types. The first is simple, with a solid photocathode where all entering photons are recorded. The second type simulates the photocathode as a thin layer, also representing the internal components. For more information, refer to Chapter 9 of <a href="https://zenodo.org/record/8121321">Martin Unland's thesis</a>.
 *  Each PMT model's construction is quite similar. However, the frontal window shape varies among models, leading to diverse combinations of ellipsoids and spheres.
 * 
 * <div style="width: 100%; text-align: center;">
 * <img src="simulation_vs_sketch_old.png" width="350" height="400" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 1: <i>Cross section of simple mDOM PMT model. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 * <div style="width: 100%; text-align: center;">
 * <img src="simulation_vs_sketch_old.png" width="500" height="410" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 1: <i>Side view of complex mDOM PMT model. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 * In the complex PMT model the photocathode has an absorption length that matches the measured quantum efficiency of the mDOM PMTs. For the other PMT models this matching has to be still be performed.
 * 
 * <div style="width: 100%; text-align: center;">
 * <img src="PW_beam_geant4_TT.png" width="500" height="410" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 3: <i>QE of simulation with the absorption length currently used compared to measurements. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 * @subsubsection response PMT response
 * @warning Detailed PMT response currently only for mDOM PMT
 * 
 * The class OMSImPMTResponse provides a realistic PMT response (charge and transit time) using the measured scans from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>. To turn on this feature, the bool-switch command line argument "pmt_response" has to be given.
 * For more information see Section 9.3.4 of the thesis.
 * 
 * <div style="width: 100%; text-align: center;">
 * <img src="PW_beam_geant4_TT.png" width="500" height="410" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 4: <i>PMT response compared to measurement for different light sources. Image from <a href="https://zenodo.org/record/8121321">M. Unland's thesis</a>.</i>
 * </div>
 * </div>
 * 
 */
