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
 * Constructs a pDOM optical module in IceCube ice with configurable
 * orientation for studying photon origin classification.
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
