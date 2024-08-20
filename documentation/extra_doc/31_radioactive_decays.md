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