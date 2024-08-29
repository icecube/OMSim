
#pragma once

#include "OMSimPMTResponse.hh"
#include "OMSimHitManager.hh"

#include <G4ThreeVector.hh>
#include <fstream>


class OMSimEffiCaliAnalyisis
{
public:
    OMSimEffiCaliAnalyisis(){};
    ~OMSimEffiCaliAnalyisis(){};

    void writeHits(double pWavelength);
    void writeHitPositionHistogram(double x, double y);
    G4String m_outputFileName;
};



