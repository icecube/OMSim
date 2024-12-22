# > Radioactive Decays Studies

Study for understanding the background produced by radioactive decays in borosilicate glass of optical modules & PMTs.

> **Warning**: This study has been tested only for Vitrovex glass (mDOM/LOM16) and the 80mm mDOM PMTs. Okamoto glass (D-Egg/LOM18) is currently under investigation.

The simulation considers the measured scintillation parameters and the specific activity of the isotopes to provide insights into the behavior of OMs in both air and ice environments over a time window t_w. The number of time windows simulated is given by the argument `--numevents``.

The primary output is a timestamped list of detected photons, which can be utilized to compute parameters such as the module's expected dark rate. The simulation steps can be summarized as:

- Determining the number of decays within t_w based on measured data.
- Initiating decay chains for each event by positioning an isotope randomly in the pressure vessel.
- Assigning decay times from a uniform distribution within [0, t_w], in case the time difference between mother-daughter decay times surpasses t_w.
- Saving into memory photons detected by the PMTs for downstream analysis.

Important customizations in the simulation involve the extension of Geant4's original scintillation class, facilitating the simulation of more complex decay processes (8 lifetimes), and the modification of the G4RadioactiveDecay class which amends default decay time of isotopes.

The scintillation properties of the mDOM glass were measured in the scope of several theses. For a summary check section 11.2 of [this thesis](https://zenodo.org/record/8121321).

Currently, there are two analysis modes:

1. With the `--multiplicity_study` argument: After each t_w time window, the multiplicity is calculated and saved to a file. Raw data isn't stored, as multiplicity studies generally involve extended simulation durations, leading to large volumes of photon data.

2. Without the `--multiplicity_study` argument: Data pertaining to photons and decayed isotopes is saved to files. If you are using multithreaded mode, then each thread will produce its own file.

### Developer Notes: Geant4 Dependencies and Modifications

This simulation module requires modifications to several Geant4 classes to enable custom handling of decay times. Unfortunately, the Geant4 physics library is not designed with extensibility in mind, making inheritance or selective function overrides impractical. As a result, we had to copy and modify entire class implementations. This approach makes the module highly dependent on the specific Geant4 version being used.

With each Geant4 update, class structures and function implementations may change, requiring updates to this module—an admittedly tedious but necessary process. For Geant4 version 4.13, the following modifications were made:

1. **`G4VRadioactiveDecay`**  
   - Modified the `DecayAnalog` method to customize the handling of decay times.

2. **`G4RadioactiveDecay`**  
   - Adjusted the `DecayIt` method to terminate decay processes when a specified termination isotope is reached, allowing for the intentional disruption of secular equilibrium.

3. **`G4ParticleChangeForRadDecay`**  
   - Overrode the `CheckSecondary` method to disable time consistency checks for secondary particles. By default, Geant4 adjusts the secondary particle time if it is earlier than the parent particle's time, which conflicts with our approach.  
   - Additionally, the `AddSecondary` method from the base class was copied, as otherwise the wrong `CheckSecondary` would be called.

These changes highlight the module's dependance on specific Geant4 implementations. For example, the time consistency checks in `G4ParticleChangeForRadDecay` were introduced in Geant4 version 4.12, requiring additional adjustments.  

**Important:** Always review and update these modifications whenever Geant4 is upgraded to ensure compatibility and correct functionality.
