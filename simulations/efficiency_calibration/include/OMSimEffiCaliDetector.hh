#pragma once
#include "OMSimDetectorConstruction.hh"


class OMSimEffiCaliDetector : public OMSimDetectorConstruction
{
public:
    OMSimEffiCaliDetector() : OMSimDetectorConstruction(){};
    ~OMSimEffiCaliDetector(){};

private:
    void constructWorld();
    void constructDetector();
};

