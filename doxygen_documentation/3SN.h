/**
 *  @defgroup sngroup Supernova Studies
 *  @brief Comprehensive analysis and simulations focusing on the detection of CCSN neutrinos with segmented sensors.
 *  @details 
 * 
 *  This simulation is designed to emulate the detection of Core Collapse Supernova (CCSN or simply SN in the following) neutrinos using segmented sensors. It directly generates particles resulting from Inverse Beta Decay (IBD) and Electron Neutrino-Electron Elastic Scattering (ENES).
 *
 *  - IBD: \f$\bar{\nu}_e + p \rightarrow n + e^+\f$
 *  - ENES: \f$\nu_e + e^- \rightarrow \nu_e + e^-\f$
 *
 *  In the IBD case, the outgoing neutron is not simulated; hence, the neutron capture process that might be detected is not included in the simulation. Each event is assigned a weight based on the interaction probability, detailed later on this page. By default, the surrounding ice is represented as a cylinder facing the CCSN, simplifying the weight calculations.
 *
 *  Although CCSN models incorporate neutrino oscillations, the default simulation does not include any oscillation scenario in the weight calculations.
 * 
 * * <div style="width: 100%; text-align: center;">
 * <img src="SNsimDiagram.png" width="400" height="400" />
 * <div style="width: 80%; margin: auto;">
 * <br/>
 * Figure 1: <i>Diagram of the steps involved to generate the events for the simulation of CCSN neutrinos. The scheme is valid for both IBD and ENES, except for the threshold check. Here, stands for both electron from ENES or positron from IBD, and ν (este simbolo se vera bien?) for both electronic neutrino from ENES or electronic antineutrino from IBD.. Image from <a href="https://zenodo.org/record/8107177">this thesis</a>.</i>
 * </div>
 * </div>
 * 
 *  The detailed procedure for this simulation is comprehensively described in 
 *  <a href="https://zenodo.org/record/8107177">this thesis</a>, specifically in section 6.4. 
 *  Below is a summary extracted from this source, outlining the steps taken to generate the events:
 *
 *  \begin{enumerate}
 *      \item Utilizing the models, the expected flux per area is calculated from the luminosity 
 *      \(L(t)\) and the mean energy \(\bar{E}(t)\) according to
 *      \begin{equation}
 *          \label{eq:fluxes}
 *          \Phi (t) = \frac{1}{4\pi d^2} \cdot \frac{L(t)}{\bar{E}(t)}.
 *      \end{equation}
 *
 *      \item The distribution of \(\Phi (t)\) is used to sample a time \(t\) of the burst via 
 *      the inverse CDF (Cumulative Distribution Function) method. All other sampling from distributions within this simulation 
 *      also employs the inverse CDF method.
 *
 *      \item For the sampled time \(t\), the corresponding mean energy and mean squared energy 
 *      are extracted from the models. These parameters contribute to the construction of the 
 *      energy spectrum \(f(E,t)\), as elucidated in 
 *      <a href="https://arxiv.org/pdf/1211.3920.pdf">this paper</a>.
 *
 *      \item The energy of the neutrino/antineutrino \(E_\nu\) is sampled from \(f(E,t)\). 
 *      If the energy falls below the IBD threshold, the algorithm reverts to the second step.
 *
 *      \item From \(E_\nu\), the angular cross section is devised. This informs the sampling 
 *      of the angle \(\theta\) between the incoming neutrino and the resulting \(e^-/e^+\). 
 *      The \(\phi\) direction is randomly generated within the range of \(0\) to \(2\pi\).
 *
 *      \item The energy of \(e^-/e^+\) is deduced from \(\theta\) and \(E_\nu\). The interaction 
 *      probability for such an event is ascertained using the total cross section, 
 *      facilitating the computation of the interaction weight, as further described below.
 *
 *      \item The \(e^-/e^+\) is manifested at a random position within the ice volume.
 *  \end{enumerate}
 * 
 * 
 *  @section Input Parameters
 *
 *  Execute `./OMSim_supernova --help` to display all possible input parameters.
 *
 *  A typical command to run the simulation is:
 *
 *  "./OMSim_supernova -n 100 --wheight 20 --wradius 20 --depth_pos 75 -o outputfilename --SNgun 0 --SNtype 0"
 *
 *  @subsection General Parameters
 *  \begin{itemize}
 *    \item -n: Number of particles to be generated.
 *    \item --depth_pos: Index of the vector determining the depth of the simulated modules. Notable values include DustLayer=65, MeanICUProperties(approx)=75, and CleanestIce=88. The data is located in "common/data/Materials/IceCubeICE.dat". The "jDepth_spice" vector provides the depth, with depth_pos serving as the index. This selection affects the effective scattering and absorption lengths of the ice at the specified depth.
 *    \item -o: Output file name. By default, two output files are created: one containing the simulated event information, and another with detection data.
 *  \end{itemize}
 *
 *  @subsection SN Framework Parameters
 *  \begin{itemize}
 *    \item --wheight: Height of the ice's simulated world cylinder.
 *    \item --wradius: Radius of the ice's simulated world cylinder.
 *    \item --SNgun: Chooses the interaction type (0 for IBD, 1 for ENES).
 *    \item --SNtype: Selects the CCSN model. Two models are currently available, provided by the Garching group. These models represent the expected fluxes from two CCSNs resulting in neutron stars, with different progenitor masses (27 and 9.6 solar masses). Simulations yielding this data can be found at https://arxiv.org/abs/1510.04643
 *  \end{itemize}
 *
 *  @subsection Fixed Energy Studies
 *
 *  This section allows users to manually set the mean energy of generated events by providing specific input parameters. For instance:
 *
 *  "--SNfixEnergy --SNmeanE 10.0 --SNalpha 3.0"
 *
 *  \begin{itemize}
 *    \item --SNfixEnergy: Acts as a flag parameter. When invoked, the simulation disregards the actual mean energy of neutrinos corresponding to the burst time. Instead, it adopts the mean energy and the pinching parameter specified by the subsequent two parameters.
 *    \item --SNmeanE: Specifies the mean energy of the neutrinos.
 *    \item --SNalpha: Defines the pinching parameter of the energy distribution (see <a href="https://arxiv.org/pdf/1211.3920.pdf">this paper</a>).
 *  \end{itemize}
 *
 *  Consequently, the neutrinos' energy is sampled from the distribution, derived from these two parameters and the previously mentioned model.
 * 
 *  @section Output Files
 *
 *  Information regarding the files that contain data outputs and insights derived 
 *  from the simulated neutrino events.
 *
 *  @subsection X_info.dat
 *
 *  This file encapsulates data concerning each generated neutrino event. Each entry contains:
 *
 *  \begin{itemize}
 *      \item Time of the neutrino burst.
 *      \item Corresponding mean energy derived from the model.
 *      \item Sampled neutrino energy \(E_\nu\).
 *      \item \(\cos(\theta)\), where \(\theta\) is the angle between the incoming neutrino and the outgoing particle (e- or e+).
 *      \item Energy of the outgoing particle (e- or e+).
 *      \item Interaction weight, calculated using the formula:
 *          \[
 *          W_{\mathrm{int}}(E_\nu) = \sigma(E) \cdot n_{\mathrm{target}} \cdot l,
 *          \]
 *          where:
 *          - \(\sigma(E)\) is the total cross section for the interaction,
 *          - \(n_{\mathrm{target}}\) is the number of targets available for the interaction in the ice,
 *          - \(l\) is the length of the simulated cylindrical world.
 *  \end{itemize}
 *
 *  @subsubsection X_data.dat
 *
 *  This file contains the detection information. Its structure is designed to facilitate various trigger studies, allowing the examination of different time windows. Users might evaluate its structure for convenience, particularly if the trigger check is integrated within the simulation (requiring a predefined time window).
 *
 *  The default content structure is as follows:
 *  \[
 *  \text{Total hits | Modules hit | PMTs hit | ...for each PMT hit...| Module number | PMT number | Hits in that PMT | "...for each Hit..." << " hit time |"}
 *  \]
 *
 *  Note that the number of columns varies per line, contingent on the number of photons detected for the simulated neutrinos.
 * 
*  \section Weights
 *
 *  The output files contain the interaction weight. However, users should also consider a weight factor depending on the 
 *  total flux and the number of simulated events. In the most general case of simulating a single depth and then 
 *  extrapolating the results to the entire detector, another weight factor is needed. This factor accounts for the 
 *  ice properties of the simulated depth with respect to the whole detector. This last factor decreases in importance 
 *  when using high multiplicity conditions, since these events generally interact closer to the module, making the 
 *  photons less sensitive to changes in the optical properties.
 *
 *  These weight factors can be added similarly as explained in 
 *  <a href="https://zenodo.org/record/8107177">this thesis, section 6.4.2</a>.
 *
 *  The total weight is composed of 3 components: the interaction probability, the SN flux through the simulated volume, 
 *  and the different optical properties of the modules at different depths:
 *
 *  \begin{equation}
 *      \label{eq:sn_weights}
 *      W = W_{\mathrm{int}}(E) \cdot W_{\mathrm{flux}}(d) \cdot W_{\mathrm{eff}}.
 *  \end{equation}
 *
 *  Each component is explained as follows:
 *
 *  \begin{itemize}
 *      \item Interaction Probability Weight:
 *          \begin{equation}
 *              \label{eq:sn_weight_int}
 *              W_{\mathrm{int}}(E_\nu) = \sigma(E) \cdot n_{\mathrm{target}} \cdot l,
 *          \end{equation}
 *          where \(\sigma(E_\nu)\) is the total cross section for the interaction, \(n_{\mathrm{target}}\) is the 
 *          number of targets per unit of volume for such interaction, and \(l=40\,m\) is the length of the generation 
 *          volume along the neutrino direction axis. This is the length of the cylinder facing the CCSN.
 *      
 *      \item Flux Weight:
 *          \begin{equation}
 *              \label{eq:sn_weight_flux}
 *              W_{\mathrm{flux}} =  \frac{1}{N_{\mathrm{gen}}} \cdot \frac{r^2}{d^2} \cdot \int \frac{L(t)}{\bar{E}(t)} dt,
 *          \end{equation}
 *          where \(r=20\,m\) is the cylindrical generation volume’s radius, \(d\) is the distance from Earth where the 
 *          CCSN is assumed to occur, and \(N_{\mathrm{gen}}\) is the number of generated events.
 *      
 *      \item Effective Weight:
 *          \begin{equation}
 *              \label{eq:sn_weight_eff}
 *              W_{\mathrm{eff}} = N_{\mathrm{modules}} \cdot \frac{\bar{V}_{\mathrm{eff}}(m)}{V_{\mathrm{eff}}(m,z_{\mathrm{sim}})},
 *          \end{equation}
 *          where \(N_{\mathrm{modules}}\) is the total modules in the simulated detector, \(V_{\mathrm{eff}}(m, 
 *          z_{\mathrm{sim}})\) is the effective volume at the simulation depth, and \(\bar{V}_{\mathrm{eff}}(m)\) is 
 *          the mean effective volume for all modules in the detector. The term \(m\) represents multiplicity, defined 
 *          as the count of different PMTs within a single module that detected the event within a specific time window.
 *  \end{itemize}
 *
 *  The effective volume can be calculated in different ways. Old simulations of the mDOM, described in 
 *  <a href="https://zenodo.org/record/8107177">this thesis, section 6.3</a>, were used in past studies. A linear 
 *  regression of the effective volume concerning the absorption length, derived from various depths, provides each 
 *  depth's effective volume. Although it is advisable to redo these simulations with a current mDOM model, users can 
 *  temporarily use the old data with a regression line of \(V_{\mathrm{eff}} = b \cdot x + c\), where \(x = 1/a\) is 
 *  the inverse of the absorption length. The coefficients \(b\) and \(c\), depending on the event's multiplicity, were 
 *  obtained as follows:
 *
 *  \begin{center}
 *  \begin{tabular}{|c|c|c|}
 *      \hline
 *      PMTs & b (slope, units \(m^2\)) & c (units \(m^3\)) \\
 *      \hline
 *      1 & 15.184 & 0.000 \\
 *      2 & 0.135 & 47.005 \\
 *      3 & 0.031 & 16.170 \\
 *      4 & 0.010 & 8.680 \\
 *      5 & 0.004 & 5.105 \\
 *      \(\geq6\) & 0.003 & 2.909 \\
 *      \hline
 *  \end{tabular}
 *  \end{center}
 *
 *  Note that, for more than 5 PMTs, the same fit line is employed due to insufficient statistics at higher PMTs in the 
 *  effective volume simulation. Additionally, since the absorption length depends on the wavelength, the fits above 
 *  were obtained assuming a wavelength of \(\lambda = 400\,nm\).
 */