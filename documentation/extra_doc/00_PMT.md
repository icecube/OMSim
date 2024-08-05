# Including new PMTs

## Detection efficiency

### QE calibration

#### Step 0
```py
import numpy as np
from scipy.optimize import curve_fit, minimize
from dataclasses import dataclass
import matplotlib.pyplot as plt
from typing import Tuple

# Constants
INITIAL_GUESS = [1, 20, 10]
MAX_ITERATIONS = 8000
SINGLE_EXP_THRESHOLD = 5

def double_exp(abs_length: np.ndarray, amplitude: float, effective_thickness: float, reflectivity_coef: float) -> np.ndarray:
    """Calculate double exponential function."""
    return amplitude * (1 - np.exp(-effective_thickness / abs_length)) * np.exp(-reflectivity_coef / abs_length)

def single_exp(abs_length: np.ndarray, amplitude: float, effective_thickness: float) -> np.ndarray:
    """Calculate single exponential function."""
    return amplitude * (1 - np.exp(-effective_thickness / abs_length))

@dataclass
class CalibrationCurve:
    abs_length: np.ndarray
    fraction: np.ndarray
    error: np.ndarray
    
    def __post_init__(self):
        self.popt_double: Tuple[float, float, float] = None
        self.pcov_double: np.ndarray = None
        self.popt_single: Tuple[float, float] = None
        self.pcov_single: np.ndarray = None
        self.max_pos_x: float = None
        self.max_pos_y: float = None

    def process(self):
        """Process the calibration curve data."""
        self.fit_double()
        self.get_max_fit()
        self.fit_single()

    def get_max_fit(self):
        """Find the maximum of the double exponential fit."""
        to_min = lambda x: -double_exp(x, *self.popt_double)
        result = minimize(to_min, x0=10)
        self.max_pos_x = result.x[0]
        self.max_pos_y = double_exp(self.max_pos_x, *self.popt_double)

    def fit_double(self):
        """Fit the data to a double exponential function."""
        try:
            self.popt_double, self.pcov_double = curve_fit(
                double_exp, 
                self.abs_length * 1e9, 
                self.fraction, 
                sigma=self.error, 
                p0=INITIAL_GUESS, 
                maxfev=MAX_ITERATIONS
            )
        except RuntimeError:
            print("Error: Failed to fit double exponential function.")

    def fit_single(self):
        """Fit the data to a single exponential function."""
        mask = self.abs_length * 1e9 > self.max_pos_x * SINGLE_EXP_THRESHOLD
        x = self.abs_length[mask] * 1e9
        try:
            self.popt_single, self.pcov_single = curve_fit(
                single_exp, 
                x, 
                self.fraction[mask], 
                sigma=self.error[mask], 
                p0=self.popt_double[:-1], 
                maxfev=MAX_ITERATIONS
            )
        except RuntimeError:
            print("Error: Failed to fit single exponential function.")

    def plot_single_exp(self, ax):
        """Plot the single exponential fit."""
        if self.popt_single is not None:
            x = np.logspace(np.log10(np.amin(self.abs_length[self.mask])), -4, 100)
            ax.plot(x, single_exp(x * 1e9, *self.popt_single), '--', label='Single Exp Fit')
            ax.errorbar(self.abs_length[self.mask], self.fraction[self.mask], yerr = self.error[self.mask],
                    fmt =  '.', label='Fitted Data')
            
    def get_needed_abs(self, QE: float) -> float:
        """Calculate the needed absorption length for a given quantum efficiency."""
        if self.popt_single is None:
            raise ValueError("Single exponential fit has not been performed.")
        amp, t_eff = self.popt_single
        log_term = np.log(1 - QE / amp)
        return -t_eff / log_term

wvs, a, h,_ =np.loadtxt("step0_QE.dat", unpack=1)
N = 100000
data = {}
plt.figure()
for wv in np.unique(wvs):
    data[wv] = CalibrationCurve(a[wvs==wv], h[wvs==wv]/N, np.sqrt(h[wvs==wv])/N)
    
    plt.plot(data[wv].abs_length, data[wv].fraction)
    data[wv].process()
plt.loglog()

amplitude = [data[key].popt_single[0] for key in data]
eff_thick = [data[key].popt_single[1] for key in data]
max_QE = [curve.max_pos_y for curve in data.values()]
wavelengths = [key for key in data]
np.savetxt("mDOM_Hamamatsu_R15458_QE_matching_parameters.dat",
           np.array([wavelengths_all, amp, eff_thick, max_QE]).T,
           delimiter="\t", header="Wavelength(nm) \t Amplitude \t Effective thickness (nm) \t Max. QE")
```


#### Step 1

#### Step 2