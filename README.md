#  OMSim

**OMSim** is a Geant4 framework for the simulation of optical modules of the IceCube Observatory. It has been mostly developed by the Münster IceCube Group during different master and PhD theses. The project is separated into different modules depending on the study (e.g. background investigations with radioactive decays, or sensitivity studies), each with its own main & executable. These modules share common files (in the "common" folder) which, for example, define geometries from modules and PMTs or also material properties.

Please check the different Modules in the [documentation](https://martinunland.github.io/OMSim) for further details and examples. 

## Installation

#### Installing Geant4

First you should install Geant4 following [the guide provided by cern](https://geant4-userdoc.web.cern.ch/UsersGuides/InstallationGuide/html/installguide.html). OMSim is currently optimised for Geant4-11.1.2. If you want to use the visualisation tools of Geant, you should include the following cmake options: 

```bash
-DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_GDML=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_USE_QT=ON -DGEANT4_USE_RAYTRACER_X11=ON -DGEANT4_USE_XM=ON
``` 

There are a few dependencies. You can install them using the following command:

```bash
apt-get -y install libxerces-c-dev libxmu-dev libxpm-dev libglu1-mesa-dev qtbase5-dev libmotif-dev libargtable2-0 libboost-all-dev
``` 
If you needed to install more, add a comment it in the Git project, so we can complete the above command 😊

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


- Finally, just run ```make``` or ```make -j N``` where N is number of cores you want to use for the compilation.

