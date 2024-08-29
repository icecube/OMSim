/**
 * @file
 * @brief Defines the OMSimEffectiveAreaDetector class for effective area simulation detector construction.
 * @ingroup EffectiveArea
 */

#pragma once

#include "OMSimDetectorConstruction.hh"

/**
 * @class OMSimEffectiveAreaDetector
 * @brief Class for detector construction in the effective area simulation.
 * @ingroup EffectiveArea
 */
class OMSimEffectiveAreaDetector : public OMSimDetectorConstruction
{
public:
    OMSimEffectiveAreaDetector() : OMSimDetectorConstruction(){};
    ~OMSimEffectiveAreaDetector(){};

private:
    void constructWorld();
    void constructDetector();
};
