# WavePID Simulation

Photon origin tracking study for IceCube optical modules using Geant4. For full documentation, see [the WavePID documentation page](https://icecube.github.io/OMSim/md_extra__doc_233__wavepid.html) or `documentation/extra_doc/33_wavepid.md`.

> **Note:** This study was applied and tested with `--detector_type 3` (standard DOM with normal QE). Other module types are technically supported via the `--detector_type` flag but have not been used in the scope of the WavePID study.

> **World volume:** A 30 m radius sphere is used for simplicity — it fully contains the cascade and a sufficiently large muon track segment for the target energy range of 1–100 GeV.

## Quick Start

```bash
# Build
mkdir build && cd build
cmake ..
make OMSim_WavePID_study -j$(nproc)

# Run: 10 events, DOM in SPICE ice, 30 GeV mu- at 5m impact parameter
./OMSim_WavePID_study -n 10 --detector_type 3 --environment 2 -d 5 -e 30 -p mu- -o output

# Visualization (vis_nophotons.mac loaded automatically from ../simulations/wavepid/)
./OMSim_WavePID_study --detector_type 3 --simple_PMT -v
```

## Output

ROOT file (`<output>_hits.root`) with TTree `PhotonHits` containing per-photon information: hit time, wavelength, photon origin classification, parent particle info, hit position/direction, and PMT number.

## Detector Types

| `--detector_type` | Module |
|-------------------|--------|
| 1 | Single PMT |
| 2 | mDOM |
| 3 | DOM (used in WavePID study) |
| 4 | LOM16 |
| 5 | LOM18 |
| 6 | D-Egg |
| 7 | pDOM (HQE deepcore) |
