/**
 *  @defgroup common OMSim framework and geometry
 *  @brief  These files define the geometry and materials that are common for all studies.
 *  @details
 *
 *  @subsection framework OMSim framework
 * 
 *  @subsubsection data Materials and user data
 *  The user defined material data are saved in json files in /common/data to reduce file lengths (original detector construction had over 4k lines mostly due to the material properties!).
 *  These properties are loaded by InputDataManager (@ref OMSimInputData.hh) into the Geant4 framework and you can retrieve them with InputDataManager::getMaterial and InputDataManager::getOpticalSurface.
 *  As different materials have different type of properties, the data is loaded in different ways. The different types of materials are defined in @ref OMSimDataFileTypes.hh.
 *  Furthermore, geometry data is also saved in json files for the PMTs which are parsed directly into a "tree", which is basically a dictionary with the keys and values in the json file. This is because the different PMTs are constructed in a very similar fashion and it does not make sense to define one class per PMT type, as is done with the optical modules.
 *  
 *  If you have data that you want to load, you could define a new type in OMSimDataFileTypes or load it into a tree as earlier, but for simple stuff you can just use InputDataManager::loadtxt which is a static method and works similar to the numpy.loadtxt of python. Example:
 *  ~~~~~~~~~~~~~{.cc}
 * 		std::vector<G4PV2DDataVector> data = InputDataManager::loadtxt("path/file_name.txt");
 *		std::vector<G4double> first_column = data.at(0);
 *  	std::vector<G4double> second_column = data.at(1);
 *      (...)
 *  ~~~~~~~~~~~~~
 * 
 *  @subsubsection geometry Geometry
 *
 *
 * <div style="width: 100%; text-align: center;">
 * <img src="image.png" width="200" height="200" />
 * <br/>
 * <i>Figure: A brief description of the image</i>
 * </div>
 */
