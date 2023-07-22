/**
 *  @defgroup EffectiveArea Effective area
 *  @brief  All classes that are specific to the effective area studies of the optical modules.
 *
 * @details 
 * @note In the following a short introduction of an effective area study is made with the mDOM as an example. For further details see Chapter 10 of <a href="https://www.uni-muenster.de/imperia/md/content/physik_kp/agkappes/abschlussarbeiten/doktorarbeiten/1702-phd_lclassen.pdf">Lew Classen's thesis</a> or
 * Section 9.3.5 of <a href="https://zenodo.org/record/8121321">this thesis</a>.
 * 
 * During the design of a module its sensitivity is optimised by considering the optical properties of various materials and altering the geometry and positioning of the mechanical components in the Geant4 simulation. 
 * The metric to optimise the module's sensitivity is the effective area \f$A_{\mathrm{eff}}(\theta, \phi)\f$. This represents the area that is detected from a plane wave of incidence angle 
 * of \f$\theta\f$ (zenith) and \f$\phi\f$ (azimuth) with respect to the module. This parameter is calculated with OMSim by simulating a disk emitting mono-energetic photons 
 * perpendicular to its surface with a uniform density profile.
 * The photon generation is made with the module @link AngularScan @endlink, running the method @link AngularScan::runSingleAngularScan @endlink once for each direction to be investigated
 * <div style="width: 100%; text-align: center;">
 * <img src="mDOM_scan.png" width="400" height="400" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 1: <i>Screenshot of an mDOM illuminated by a disk emitter simulating the incidence of a plane wave from the direction \f$\theta\f$ (zenith) and \f$\phi\f$ (azimuth) using the Geant4 visualiser. The photon trajectories are represented by orange lines, and yellow dots mark the locations of physical interactions. Image from <a href="https://zenodo.org/record/8121321">this thesis</a>.</i>
 * </div>
 * </div>
 * 
 *  After simulating $N$ photons from a plane of area \f$A_{\mathrm{rad}}\f$ (which has to be larger than the projected area of the mDOM), the effective area is calculated as
 *    \f[ 
 *        A_{\mathrm{eff}}(\theta, \phi) = \frac{N_{\mathrm{det}}(\theta, \phi)}{N}\cdot A_{\mathrm{rad}},
 *    \f] 
 *    where \f$N_{\mathrm{det}}\f$ is the number of photons detected by the module. To optimise the sensitivity over multiple directions, \f$A_{\mathrm{eff}}(\theta, \phi)\f$ is calculated 
 *    for various angle pairs, and the mean effective area, \f$\overline{A}_{\mathrm{eff}}\f$, is determined. The \f$n\f$ angle pairs are chosen using healpix, which separates a 
 *    sphere into \f$n\f$ pixels of equal area. Thus, 
 *    \f[ 
 *        \overline{A}_{\mathrm{eff}} = \frac{1}{n}\sum_{i=1}^n A_{\mathrm{eff}}(\theta_i, \phi_i).
 *   \f] 
 *
 *    Since the effective area is wavelength dependent, the calculation must be repeated for different wavelengths. Wavelengths shorter than 300nm are absorbed by the pressure vessel,
 *  whereas the QE of the PMT approaches zero at wavelengths longer than 700nm.
 * 
 * @subsection exampleEA Example using healpix
 * 
 * In the following a toy example of the usage of the effective area module is given. Although there are c++ healpix libraries, in my opinion the easiest way of getting the angle pair
 * coordinates is using Healpy in python. You can just save the angle pairs in a text file and load them in OMSim.  
 * 
 */
