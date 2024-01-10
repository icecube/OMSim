#  OMSim

**Under construction. If you want to use OMSim contact martin.u in IC-slack...**

**OMSim** is a Geant4 framework for the simulation of optical modules of the IceCube Observatory. It has been mostly developed by the Münster IceCube Group during different master and PhD theses. The project is separated into different modules depending on the study (e.g. background investigations with radioactive decays, or sensitivity studies). These modules share common files (in the "common" folder) which, for example, define geometries from modules and PMTs or also material properties.

Please check the different Modules in the [documentation](https://icecube.github.io/OMSim/) for further details and examples. 

## Installation

#### Installing Geant4

First you should install Geant4 following [the guide provided by cern](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/installguide.html). OMSim is currently optimised for Geant4-11.1.1. If you want to use the visualisation tools of Geant, you should include the following cmake options: 

```bash
-DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_GDML=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_QT=ON -DGEANT4_USE_RAYTRACER_X11=ON -DGEANT4_USE_XM=ON
``` 
#### Requirements
There are a few dependencies. You can install them using the following command:

```bash
apt-get -y install libxerces-c-dev libxmu-dev libxpm-dev libglu1-mesa-dev qtbase5-dev libmotif-dev libargtable2-0 libboost-all-dev libqt53dextras5 libspdlog-dev libfmt-dev
``` 
If you needed to install more, add a comment it in the Git project, so we can complete the above command 😊

Also you will need ROOT. Download the last binary (e.g. at time of writing latest version is 6.28/04 https://root.cern/releases/release-62804/), or compile the source distribution. For simplicity add the source in your .bashrc (or you will have to source it yourself manually every time you run or compile OMSim):
```bash
export ROOTSYS= YOUR_ROOT_PATH
source $ROOTSYS/bin/thisroot.sh
``` 

#### Compiling OMSim
- Clone this branch.
- Check _CMakeLists.txt_ and change the paths with the Geant4 bins and libraries of your system
- Make a new folder named e.g. "build" 
- From the build folder run cmake as follows
```bash
cmake -D4Geant4_DIR=YOUR_G4_INSTALL/lib/Geant4-11.1.1/ ..
``` 
where "YOUR_G4_INSTALL" is the path to the install folder of Geant4 in your system.

@note If you have to compile the project several times you should add an alias in your bashrc, e.g. 
```bash
alias mycmake="cmake -D4Geant4_DIR=YOUR_G4_INSTALL/lib/Geant4-11.1.1/"
``` 

-Source the Geant4 library (you could add this to your .bashrc)
```bash
source YOUR_G4_INSTALL/bin/geant4.sh
``` 
- Finally, just run ```make``` or ```make -j N``` where N is number of cores you want to use for the compilation.

## Available studies

OMSim has been utilized in a range of studies, each simulating unique physics, thereby necessitating distinct Physicslist/analysis setups. In this repository, we have compiled a selection of these studies, each contained within its own folder and accompanied by its own main file. Currently available:

- [Effective area](https://icecube.github.io/OMSim/group___effective_area.html): calculates the effective area of the optical modules/PMTs.
- [Radioactive decays](https://icecube.github.io/OMSim/group__radioactive.html): simulates radioactive decays within the glass of the pressure vessel and the PMT glass. Essential for understanding the primary background of optical modules.
- [Supernova studies](https://icecube.github.io/OMSim/group__sngroup.html): used for the development of an improved SN trigger for IceCube using multi-PMT modules.
- [Bubble column](https://icecube.github.io/OMSim/group__bubble.html): used for investigating the capability of the mDOM flashers in determining bubble column parameters.

Most users will likely utilize just one of these studies, meaning there's no need to compile all of them. If you wish to exclude certain studies from compilation, you can comment out the undesired ones in the CMakeLists.txt between lines 41-43 (where the **add_subdirectory** commands are located).