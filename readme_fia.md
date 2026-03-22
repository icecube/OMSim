# Muon Background Simulation (Fia Master Thesis)

## Overview

In addition to radioactive decays, the simulation includes a model for atmospheric muons.
This functionality is implemented within the existing **radioactive decays study** and uses the same detector geometry and physics configuration.
In **effective_area** I tried things, but the final simulation was done in **radioactive decays**.

The simulation combines two independent background sources:

* **Radioactive decays**: local, isotropic light emission inside detector materials
* **Atmospheric muons**: external, through-going particles producing extended tracks

Both components are simulated within a common time window and contribute to the final detector signal.

---

## Simulation Time Window

The simulation is performed over a configurable time window `T`, set via:

`--time_window`

This parameter determines:

* the number of generated muons,
* the number of radioactive decays,
* and the temporal distribution of all events.

---

## Muon Generation

Atmospheric muons are simulated as a stochastic flux. The number of generated muons follows:

```
N_μ ~ Poisson(Φ · A · T)
```

where:

* `Φ` is the muon flux,
* `A` is the effective generation area,
* `T` is the simulation time window.

Muon arrival times are sampled uniformly in `[0, T]`, corresponding to a constant flux.

---

## Angular Distribution

Muon directions follow:

```
I(θ) ∝ cosⁿ(θ)
```


---

## Energy Spectrum

Muon energies are sampled from a modified Gaisser parameterization.
This provides a realistic spectrum from GeV to TeV energies.

---

## Propagation and Optical Photon Production

Each muon is simulated as an individual event (`runBeamOn(1)`) using the Geant4 General Particle Source (GPS).

The following optical processes may contribute:

* Cherenkov radiation
* Scintillation

Optional runtime switches:

`--scint_off` → disable scintillation
`--cherenkov_off` → disable Cherenkov radiation

Have a look in the PhysicsList for the processes for the muons.
---

## Event Structure and Timing

* Each muon is simulated as a separate event
* Event timestamps are assigned during generation
* The global detector signal is reconstructed from these timestamps in post-processing

---

## Multiplicity Studies

The simulation can optionally compute hit multiplicities using:

`--multiplicity_study`

something might be not correct here.

In this mode:

* coincident hits within a time window (`--multiplicity_time_window`) are analyzed
* multiplicity distributions are computed

**Note:**
Hit-level output is still written even when multiplicity mode is enabled...

---

## Output

The simulation produces output files with detailed detector information.

### Hit Output (`*_hits.dat`)

Each line corresponds to a detected photon hit:

```
[eventID] [time (s)] [PMT number] [energy]
```

This is THE perfect output, since multiplicity etc can all be analyzed afterwards from the timestamps.
---

### Multiplicity Output (`*_multiplicity.dat`)

If multiplicity mode is enabled, an additional file is written containing multiplicity information.

This represents the number of coincident hits within the defined time window.

Note: there might still be a conflict when disabling --multiplicity_study.
---

## Example Usage

A typical simulation run:

```bash
./OMSim_radioactive_decays \
  --no_PMT_decays \
  --efficiency_cut \
  --multiplicity_study \
  -n {n} \
  --time_window 120 \
  --place_harness \
  --temperature {l} \
  -o {output_file} \
  --environment {e} \
  --threads {threads} \
  --detector_type {detector_type}
```
