# WavePID Simulation

Photon origin tracking study for IceCube optical modules using Geant4.

## Overview

This simulation tracks the origin of each detected photon, distinguishing between:
- **Cerenkov from Muon** - Cherenkov photons emitted directly by muons
- **Cerenkov from Electron** - Cherenkov photons from secondary electrons/positrons
- **Cerenkov from Other** - Cherenkov from other particle types
- **Bremsstrahlung** - Photons from bremsstrahlung radiation
- **Scintillation** - Scintillation photons
- **PrimaryOpticalPhoton** - Primary optical photons from the generator
- **Primary** - Other primary particles
- **Other** - Unclassified photons

## Building

```bash
cd OMSim_wavepid_cleaned
mkdir build && cd build
cmake ..
make OMSim_WavePID_study -j$(nproc)
```

## Usage

### Basic Run
```bash
./OMSim_WavePID_study -n 10 -o output_name
```

### With Macro File (recommended for GPS configuration)
```bash
./OMSim_WavePID_study -o output_name --macro path/to/macro.mac
```

### Command-Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `-n, --numevents` | Number of events to simulate | 0 |
| `-o, --output_file` | Output filename base (creates `<name>_hits.root`) | required |
| `-m, --macro` | Path to macro file for GPS/particle configuration | none |
| `-z, --DOM_zenith` | DOM zenith angle (degrees) | 0.0 |
| `-a, --DOM_azimuth` | DOM azimuth angle (degrees) | 0.0 |
| `-s, --dom_event_spacing` | Distance between DOM and primary particle (m) | 0 |
| `-t, --threads` | Number of threads | 1 |
| `-v, --visual` | Enable visualization | false |

### Example Macro File

```
# muon_test.mac - 100 GeV muon beam
/gps/particle mu-
/gps/energy 100 GeV
/gps/position 0 0 -5 m
/gps/direction 0 0 1
/run/beamOn 10
```

## Output

The simulation produces a ROOT file (`<output>_hits.root`) containing a TTree named `PhotonHits` with:

| Branch | Type | Description |
|--------|------|-------------|
| `eventID` | Int_t | Event number |
| `hitTime` | Double_t | Time of photon hit (ns) |
| `wavelength` | Double_t | Photon wavelength (m) |
| `photonOrigin` | TString | Origin classification |
| `parentID` | Int_t | Parent track ID |
| `parentType` | TString | Parent particle type (mu-, e-, etc.) |
| `parentProcess` | TString | Process that created parent |
| `pmtNumber` | Int_t | PMT number |
| `hitPosition_x/y/z` | Double_t | Hit position (mm) |
| `hitDirection_x/y/z` | Double_t | Hit direction |

## Analysis

Example ROOT macro to analyze results:

```cpp
void analyze() {
    TFile *f = TFile::Open("output_hits.root");
    TTree *t = (TTree*)f->Get("PhotonHits");

    // Count by origin
    TString *origin = new TString();
    t->SetBranchAddress("photonOrigin", &origin);

    std::map<TString, int> counts;
    for (Long64_t i = 0; i < t->GetEntries(); i++) {
        t->GetEntry(i);
        counts[*origin]++;
    }

    for (auto& p : counts) {
        std::cout << p.first << ": " << p.second << std::endl;
    }
}
```

## Architecture

This simulation maintains full compatibility with OMSim master by keeping all modifications within `simulations/wavepid/`. The `common/` directory is unchanged.

### Files in this simulation

**Headers (`include/`):**
- `OMSimTrackingAction.hh` - Singleton tracking all particles
- `OMSimSensitiveDetector.hh` - Extended hit info structure
- `OMSimHitManager.hh` - Extended with photon origin fields
- `OMSimWavePIDDetector.hh` - Detector construction
- `ROOTHitManager.hh` - ROOT TTree output
- Override headers: `OMSim.hh`, `OMSimRunAction.hh`, etc.

**Sources (`src/`):**
- `OMSimTrackingAction.cc` - Stores track ID to particle type mapping
- `OMSimSensitiveDetector.cc` - Classifies photon origins
- `OMSimHitManager.cc` - Manages hit data with ROOT output
- `OMSimWavePIDDetector.cc` - Creates pDOM in IceCube ice
- `OMSimPhysicsList.cc` - Full physics including muons
- `ROOTHitManager.cc` - Writes TTree to ROOT file

**Main executable:**
- `OMSim_WavePID_study.cc` - Entry point

## Dependencies

- Geant4 (11.x, multithreaded)
- ROOT (for output)
- Boost (program_options)
- spdlog, fmt
