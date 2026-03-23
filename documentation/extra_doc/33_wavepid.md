# > WavePID Photon Origin Tracking
[TOC]

In the folder `simulations/wavepid` you find the files pertinent to the WavePID photon origin tracking study.

## Introduction

The WavePID simulation tracks the origin of each detected photon in IceCube optical modules. By classifying photons by their production mechanism and parent particle, this study enables analysis of photon arrival time distributions used in the WavePID particle identification method.

> **Note:** This study was applied and tested solely with `--detector_type 3` (standard DOM with normal quantum efficiency). Other module types are technically supported via the `--detector_type` flag but have not been used or validated in the scope of the WavePID study. The `--efficiency_cut` flag was not used — all photons arriving at the sensitive volume were recorded, as the QE-based die roll affects all photon origins equally and does not impact the origin-based timing distributions.

## Photon Origin Classification

Each detected photon is classified into one of the following origin categories:

| Category | Description |
|----------|-------------|
| **Cerenkov from Muon** | Cherenkov photons emitted directly by muons (mu+, mu-) |
| **Cerenkov from Electron** | Cherenkov photons from secondary electrons/positrons (e+, e-) |
| **Cerenkov from Other** | Cherenkov photons from other particle types |
| **Bremsstrahlung** | Photons from electromagnetic bremsstrahlung |
| **Scintillation** | Scintillation photons from the medium |
| **PrimaryOpticalPhoton** | Primary optical photons from the particle generator |
| **Primary** | Other primary particles |
| **Other** | Unclassified photons |

The classification is performed in `OMSimSensitiveDetector::getPhotonInfo()` using the Geant4 creator process name and the parent particle type stored by `OMSimTrackingAction`.

## Command-Line Arguments

### WavePID-Specific Arguments

| Option | Description | Default |
|--------|-------------|---------|
| `-d, --impact_parameter` | Perpendicular distance from muon track to DOM center (m) | 5.0 |
| `-e, --primary_energy` | Primary particle energy (GeV) | 10.0 |
| `-p, --primary_particle` | Primary particle type (`mu-`, `mu+`, `e-`, `e+`) | `mu-` |
| `-z, --DOM_zenith` | DOM zenith orientation angle (degrees) | 0.0 |
| `-a, --DOM_azimuth` | DOM azimuth orientation angle (degrees) | 0.0 |
| `-m, --macro` | Path to Geant4 macro file (overrides `-d`, `-e`, `-p` if GPS commands are used) | none |

### General OMSim Arguments

| Option | Description | Default |
|--------|-------------|---------|
| `-n, --numevents` | Number of events to simulate | 0 |
| `-o, --output_file` | Output filename base (creates `<name>_hits.root`) | required |
| `--detector_type` | Module type: Single PMT (1), mDOM (2), DOM (3), LOM16 (4), LOM18 (5), D-Egg (6), pDOM HQE (7) | 2 |
| `--environment` | Medium: AIR (0), ice (1), SPICE (2) | 0 |
| `--depth_pos` | Ice depth index: DustLayer=65, MeanIC=75, CleanestIce=88 | 75 |
| `--place_harness` | Place the OM harness geometry (if implemented for the module) | false |
| `--efficiency_cut` | Apply QE-based detection efficiency cut (rolls a detection probability dice per photon) | false |
| `--simple_PMT` | Use simplified PMT model without scan data | false |
| `-t, --threads` | Number of worker threads | 1 |
| `-v, --visual` | Enable visualization mode | false |

### Command-Line vs. Macro File Parameters

The particle source can be configured in two ways:

1. **Command-line arguments** (`-d`, `-e`, `-p`): The `OMSimPrimaryGeneratorAction` uses these to set up a GPS particle source with the correct geometry (impact parameter, Cherenkov cone alignment).

2. **Macro file** (`-m`): Provides full control over the GPS configuration via Geant4 macro commands. When a macro file is provided, its GPS commands override the command-line particle settings.

For simple configurations, command-line arguments are sufficient. For complex setups (multiple particle sources, custom angular distributions), use a macro file.

## Usage Examples

### Basic muon simulation with DOM
```bash
./OMSim_WavePID_study -n 10 --detector_type 3 --environment 2 -d 5 -e 30 -p mu- -o output
```

### Using a macro file
```bash
./OMSim_WavePID_study -o output --macro muon_config.mac
```

Example macro file (`muon_config.mac`):
```
/gps/particle mu-
/gps/energy 30 GeV
/gps/position -8.5 0 5 m
/gps/direction 1 0 0
/run/beamOn 10
```

### Visualization
```bash
./OMSim_WavePID_study --detector_type 3 --simple_PMT -v --macro vis_wavepid.mac
```

## Output Format

The simulation produces a ROOT file (`<output>_hits.root`) containing a TTree named `PhotonHits`.

| Branch | Type | Description |
|--------|------|-------------|
| `eventID` | Int_t | Event number |
| `hitTime` | Double_t | Global time of photon hit (ns) |
| `flightTime` | Double_t | Local time / flight time of the photon (ns) |
| `entryTime` | Double_t | Time photon entered the DOM (ns) |
| `pathLength` | Double_t | Total track length of the photon (m) |
| `generationDetectionDistance` | Double_t | Distance between generation and detection point (mm) |
| `energy` | Double_t | Photon kinetic energy |
| `wavelength` | Double_t | Photon wavelength (nm) |
| `photonOrigin` | TString | Origin classification (see table above) |
| `parentID` | Int_t | Parent track ID |
| `parentType` | TString | Parent particle type (mu-, e-, etc.) |
| `parentProcess` | TString | Geant4 process that created the parent |
| `pmtNumber` | Int_t | PMT number within the module |
| `nPhotoelectrons` | Int_t | Number of photoelectrons (from PMT response) |
| `dir_x/y/z` | Double_t | Photon momentum direction at detection |
| `localPos_x/y/z` | Double_t | Hit position in PMT local coordinates (mm) |
| `globalPos_x/y/z` | Double_t | Hit position in global coordinates (mm) |
| `deltaPos_x/y/z` | Double_t | Vector from generation to detection point (mm) |

## Visualization Macro

A visualization macro is provided in `simulations/wavepid/`:

| Macro | Description |
|-------|-------------|
| `vis_wavepid.mac` | Full trajectory display with particle coloring. Filters optical photons, neutrinos, and gammas. Includes default GPS: 30 GeV mu- at 5m impact parameter. |

## Notes on World Volume Size

The world volume is a sphere of 30 m radius. For simplicity, no attempt was made to tune the world size precisely — the chosen radius fully contains the hadronic/electromagnetic cascade and a sufficiently large segment of the muon track for the target energy range of 1–100 GeV, without a noticeable performance impact.

## Notes on `--efficiency_cut`

The WavePID study did **not** use `--efficiency_cut`. All photons arriving at the sensitive volume (PMT photocathode) were recorded regardless of detection probability. This is because the QE-based die roll affects all photon origins equally and therefore does not impact the origin-based timing distributions that are the focus of this study.

The flag can be enabled if desired:

- **Without** `--efficiency_cut` (default, used in WavePID study): All photons reaching the photocathode are recorded. The detection probability is stored in the output and can be applied as a weight in post-processing.
- **With** `--efficiency_cut`: Only "detected" photons are recorded (detection probability set to 1 in output). This reduces output file size but prevents re-weighting in analysis.

## Notes on Multithreading

The simulation supports multithreading via `--threads`. Each thread maintains its own hit data storage (using `G4ThreadLocal`), which is merged at the end of each run. The `TrackingAction` track-to-particle maps are also `thread_local`, so no mutex is needed — each worker thread owns its maps independently.

Note that running with different thread counts may produce different event-to-thread assignments, so exact hit-by-hit reproducibility across different thread counts is not guaranteed. However, statistical results should be consistent.

## Architecture

This simulation maintains compatibility with the OMSim framework by keeping all modifications within `simulations/wavepid/`. The `common/` directory is unchanged.

The following framework files are overridden locally (via CMakeLists.txt source exclusion):

| Override | Purpose |
|----------|---------|
| `OMSimTrackingAction` | Singleton storing track ID to particle type/process mappings |
| `OMSimSensitiveDetector` | Extended with photon origin classification logic |
| `OMSimHitManager` | Extended with photon origin, parent info, and ROOT output fields |
| `OMSim` | Modified initialization for WavePID-specific setup |
| `OMSimActionInitialization` | Registers WavePID-specific user actions |

Additional files:
- `ROOTHitManager` — manages ROOT TFile/TTree creation and filling
- `OMSimWavePIDDetector` — detector construction (places selected module in chosen environment)
- `OMSimPrimaryGeneratorAction` — GPS-based particle source with impact parameter geometry
- `OMSimPhysicsList` — full physics list including muon interactions, Cherenkov, and scintillation
