/**
 * @file OMSimWavePIDDetector.hh
 * @brief Detector construction for WavePID simulation study.
 */
#pragma once

#include "OMSimDetectorConstruction.hh"

/**
 * @class OMSimWavePIDDetector
 * @brief Detector construction for WavePID photon origin tracking study.
 *
 * Constructs the selected optical module (via --detector_type) in the
 * chosen environment (via --environment) for photon origin classification.
 */
class OMSimWavePIDDetector : public OMSimDetectorConstruction
{
public:
    OMSimWavePIDDetector() : OMSimDetectorConstruction() {};
    ~OMSimWavePIDDetector() {};

private:
    void constructWorld();
    void constructDetector();
};
