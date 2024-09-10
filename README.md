#  OMSim Geant4 Framework
[TOC]

**Note:** This project is under active development. If you're interested in using OMSim, please contact martin.u on IC-slack.

**OMSim** is a Geant4 framework for simulating optical modules of the IceCube Observatory. It comprises multiple modules for different studies such as background investigations with radioactive decays and sensitivity analyses. These modules share common files (in the "common" folder) that define, for example, geometries of modules and PMTs, as well as material properties.

For more information, please refer to our [documentation](https://icecube.github.io/OMSim/).

For the latest updates and information, check our [GitHub repository](https://github.com/icecube/OMSim). If you need assistance or want to report problems, please open an issue on our GitHub page or contact the maintainers directly.

## Installation

### Installing Geant4

1. Install Geant4 following [the guide provided by CERN](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/installguide.html). OMSim is currently optimised for Geant4-11 (last tested with 11.2.2).

2. For visualisation tools, include the following CMake options:

   ```bash
   -DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_GDML=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_QT=ON -DGEANT4_USE_RAYTRACER_X11=ON -DGEANT4_USE_XM=ON
   ``` 

   Note: Do not change `GEANT4_BUILD_MULTITHREADED` to OFF, as OMSim supports multithreading.

3. Source the Geant4 library and add this to your .bashrc, where "YOUR_G4_INSTALL" is the path to the install folder of Geant4 in your system:
   ```bash
   source YOUR_G4_INSTALL_PATH/bin/geant4.sh
   ``` 

### Installing Dependencies

Install the required dependencies using:

```bash
sudo apt-get -y install libxerces-c-dev libxmu-dev libxpm-dev libglu1-mesa-dev qtbase5-dev libmotif-dev libargtable2-0 libboost-all-dev libqt53dextras5 libfmt-dev spdlog-dev
``` 

### Installing ROOT

1. Download the latest ROOT binary from [ROOT's official website](https://root.cern/releases/) (e.g., version 6.28/04 at the time of writing), or compile from source.

2. Add the following to your .bashrc for convenience:
   ```bash
   export ROOTSYS=YOUR_ROOT_PATH
   source $ROOTSYS/bin/thisroot.sh
   ``` 

### Compiling OMSim


1. Clone this repository:
   ```bash
   git clone https://github.com/icecube/OMSim.git
   cd OMSim
   ```

2. Create a build directory:
   ```bash
   mkdir build && cd build
   ```

3. Run CMake:
   ```bash
   cmake ..
   ```
   If CMake doesn't find Geant4, use the following, where "YOUR_G4_INSTALL" is the path to the install folder of Geant4 in your system:
   ```bash
   cmake -DGeant4_DIR=YOUR_G4_INSTALL/lib/Geant4-11.2.2/ ..
   ```

4. Compile the project:
   ```bash
   make -j$(nproc)
   ```

## Available studies

OMSim has been utilized in a range of studies, each simulating unique physics, thereby necessitating distinct Physicslist/analysis setups. In this repository, we have compiled a selection of these studies, each contained within its own folder and accompanied by its own main file. Currently available:

- [Effective area](https://icecube.github.io/OMSim/md_2_effective_area.html): calculates the effective area of the optical modules/PMTs.
- [Radioactive decays](https://icecube.github.io/OMSim/md_3_radioactive_decays.html): simulates radioactive decays within the glass of the pressure vessel and the PMT glass. Essential for understanding the primary background of optical modules.
- [Supernova studies](https://icecube.github.io/OMSim/md_4__s_n.html): used for the development of an improved SN trigger for IceCube using multi-PMT modules.

### Customising Compilation
To exclude certain studies from compilation, edit the `simulations/CMakeLists.tx` file and comment out the unwanted `add_subdirectory()` calls before running CMake.


