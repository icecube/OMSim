# Technicalities for Developers
[TOC]

## The Tools namespace
 
The tools namespace provide several methods that could help you. For example, `Tools::loadtxt`, `Tools::linspace` and `Tools::logspace` operate similarly to their Python's numpy counterparts:

```cpp
#include "OMSimTools.hh"
std::vector<G4PV2DDataVector> lData = Tools::loadtxt("path/file_name.txt", true);
std::vector<G4double> lFirstColumn = lData.at(0);
std::vector<G4double> lSecondColumn = lData.at(1);

std::vector<double> lWavelengths = Tools::linspace(275, 750, 96);
std::vector<double> lAbsLengths = Tools::logspace(-9, -4, 20);

std::vector<double> data = {1, 2, 2, 3, 3, 3, 4, 4, 5};
auto [counts, edges] = Tools::histogram(data, 5);

// ...
```

---
---

## Nomenclature

We follow these naming conventions:

Methods:
- Written in camelCase, starting with a lowercase letter
- Example: `calculateDistance()`
- Note: This contrasts with Geant4's methods, which always start with an uppercase letter

Classes:
- Written in PascalCase (also known as UpperCamelCase)
- Core framework classes should start with `OMSim`
- Example: `OMSimDetectorConstruction`

Namespaces
- Written in PascalCase
- Example: `ToolsNamespace`

Member Variables:
- Prefix: `m_`
- Written in camelCase after the prefix
- Example: `m_particleEnergy`

Global Variables:
- Prefix: `g_`
- Written in camelCase after the prefix
- Example: `g_worldVolume`

Function Parameters:
- Prefix: `p_`
- Written in camelCase after the prefix
- Example: `p_initialPosition`

Local Variables
- No prefix, written in camelCase
- Example: `localParticle`

By adhering to these conventions, we aim to:
1. Clearly distinguish OMSim code from Geant4 code (so we can for example find+replace code without worries)
2. Easily identify the scope and purpose of variables and functions
3. Maintain consistency throughout the codebase, enhancing readability and maintainability

Remember to apply these conventions consistently across all new code and when refactoring existing code.

---
---


## Matching PMT Efficiency to Measurements

In order to simulate the PMT efficiency correctly, each photon is given a weight that corresponds to the detection probability ([click here](md_extra_doc_0_common.html#autotoc_md5) for more details). These weights are calculated using data files to match measurements. These data files have to be generated for each new PMT that is to be added to the framework. The module `efficiency_calibration` facilitates this procedure. In the following, the mDOM PMT is used as an example.

### Step 1: Fraction of absorbed photons

The optical properties (refractive index, thickness, and absorption length) of the glass tube and photocathode determine the number of photons absorbed in the photocathode. The optical properties of the photocathode (defined in file `Surf_Generic_Photocathode_20nm.dat`) are from this paper https://arxiv.org/abs/physics/0408075 .

To calculate the quantum efficiency weight, we have to know how many photons are absorbed in the photocathode in the simulation. For this, go through the following steps:
 - The most precise procedure is to simulate the beam used during the QE measurement (see for example `Beam::configureErlangenQESetup` and `Beam::runErlangenQEBeam` for the beam used in Erlangen). If you don't know the divergence / beam size of the setup, you may use the one from Erlangen first.
 - In `OMSim_efficiency_calibration.cc` change the method `runQEbeamSimulation()` to use the newly defined beam instead of `runErlangenQEBeam`. Define there also the wavelength range to be simulated.
 - Run the simulation with the following command, changing the PMT number to the one you want to simulate (make sure that your PMT is in the list in `OMSimPMTConstruction::selectPMT`, or hard code it in `OMSimEffCaliDetector.cc`)
```bash
./OMSim_efficiency_calibration --pmt_model 0 --simulation_step 1 -n 500000 --threads 4 --detail_pmt --output_file step1
```
 - Check results and uncertainty. You want to determine the fraction pretty accurately, you may repeat the simulation at the UV region with higher statistics (change wavelength range in `runQEbeamSimulation()`).

To obtain the fraction I just run this python code:

```py
wvs, h,_ =np.loadtxt("step1.dat", unpack=1)
N=500000
err = np.sqrt(h) / N
h /= N

plt.figure()
plt.errorbar(wvs, h, fmt=".", yerr = err, label ="Absorbed photons")
plt.ylabel("Fraction")
plt.xlabel("Wavelength (nm)")
plt.grid()
plt.legend()
np.savetxt("mDOM_Hamamatsu_R15458_CAT_intrinsic_QE.dat",
           np.array([wvs, h, err]).T,
           delimiter="\t", header="Wavelength(nm) \t QE \t error")
```
Here the results of the mDOM PMT including its mean QE as comparison:

<div style="width: 100%; text-align: center;">
<img src="step1_results_mdom.png" width="450" height="320" alt="" />
<div style="width: 80%; margin: auto;">
<br/>
</div>
</div>

> **Note**: Ensure that the QE of the PMT you intend to use is always smaller than the obtained absorbed fraction. If this is not the case you will have to change the optical properties of the photocathode and/or the tube glass!

If everything looks good, save the file (in the example above `mDOM_Hamamatsu_R15458_CAT_intrinsic_QE.dat`) in `common/data/PMTs/measurement_matching_data/QE/` and move to step 2.

### Step 2: Expand OMSimPMTResponse and Verify QE

Each PMT has its own derived class in `OMSimPMTResponse.cc`. If your PMT has not class yet, create one following the other PMTs as example. In its constructor add a call to `configureQEweightInterpolator()` adding the file that you generated in the last step and a default QE file for this PMT. Make sure you are not creating any CE weight interpolator at this point (this happens in step 3 of this documentation), as otherwise the weights will be smaller than expected from QE only!

Now run the simulation again 
```bash
./OMSim_efficiency_calibration --pmt_model 0 --simulation_step 2 -n 100000 --threads 4 --detail_pmt --output_file step2
```

and check that the weights are being calculated correctly

```py
wvs, h, w =np.loadtxt("step2.dat", unpack=1)
N=100000
err = np.sqrt(h)*w/N**2
h /= N
w /= N


plt.figure()
plt.errorbar(wvs, w, fmt=".", yerr = err, label ="Mean sum of photon weights Geant4")
plt.xlabel("Wavelength (nm)")
plt.grid()
plt.legend()
```

<div style="width: 100%; text-align: center;">
<img src="step2_qe.png" width="450" height="320" alt="" />
<div style="width: 80%; margin: auto;">
<br/>
</div>
</div>

### Step 3: Matching detection efficiency scan

The last step is to create the collection efficiency weights to match the relative detection efficiency scans. For this the scan measurement is replicated in the simulation, scanning the PMT in a XY grid. The output file of the simulation of this step is a histogram with the position of absorbed photons for each beam position.

 - As before, we have to simulate the beam used during the scan measurement (see for example `Beam::configureXYZScan_PicoQuantSetup` and `Beam::runBeamPicoQuantSetup` for the beam used in Münster)
 - In in `runXYZfrontalScan()` of `OMSim_efficiency_calibration.cc` change the scan range (`lX` vector) and radius limit (`lRlim`) according to the diameter of your PMT
 - Also adjust the binning of the output histogram in `OMSimEffiCaliAnalyisis::writeHitPositionHistogram`
 - Run the XY grid simulation. 10000 photons per grid position should be enough, but you may increase / decrease statistics as you want
 ```bash
./OMSim_efficiency_calibration --pmt_model 0 --simulation_step 3 -n 10000 --threads 4 --detail_pmt --output_file step3
```
 - Fit the weights using simulation data. The analysis done for the mDOM can be found in the notebook located in `documentation/notebooks/detection_efficiency_matching/`.

 - Save the weights in a file and store it in `common/data/PMTs/measurement_matching_data/CE_weight/`
 - Add the `configureCEweightInterpolator()` in the constructor of your PMT class using the new file as input
 - Run the simulation again and check if the weights are correct.

 
---
---