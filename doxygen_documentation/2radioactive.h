/**
 *  @defgroup radioactive Radioactive decays
 *  @brief  Simulations for understanding the background produced by radioactive decays in borosilicate glass of optical modules & PMTs.
 *  @details 
 *  @warning
 *    This study has been tested only for Vitrovex glass (mDOM/LOM16) and the 80mm mDOM PMTs. Okamoto glass (D-Egg/LOM18) is currently under investigation.
 * 
 *  The simulation considers the measured scintillation parameters and the specific activity of the isotopes to provide insights into the behavior of OMs in both air and ice environments over a time window \f$t_{w}\f$. The number of time windows simulated is given by the argument --numevents.
 *  The primary output is a timestamped list of detected photons, which can be utilized to compute parameters such as the module's expected dark rate. The simulation steps can be summarized as:
 *  - Determining the number of decays within \f$t_{w}\f$ based on measured data.
 *  - Initiating decay chains for each event by positioning an isotope randomly in the pressure vessel.
 *  - Assigning decay times from a uniform distribution within [0, \f$t_{w}\f$], in case the time difference between mother-daughter decay times surpasses \f$t_{w}\f$.
 *  - Capturing photons detected by the PMTs for downstream analysis.
 *  
 *  Important customizations in the simulation involve the extension of Geant4's original scintillation class, facilitating the simulation of more complex decay processes (8 lifetimes), and the modification the G4RadioactiveDecay class which amends default decay time of isotopes. 
 *  The scintillation properties of the mDOM glass were measured in the scope of several theses. For a summary check section 11.2 of <a href="https://zenodo.org/record/8121321">this thesis</a>.
 *  
 *  At the time there are two analysis modes:
 *  - with --multiplicity_study argument: after each \f$t_{w}\f$ time window the multiplicity is calculated and saved to a file. The raw data is not saved, as multiplicity studies generally involve long simulation times (i.e. large number of photons)
 *  - without --multiplicity_study argument: the data of photons and decayed isotopes are saved into files.
 */
