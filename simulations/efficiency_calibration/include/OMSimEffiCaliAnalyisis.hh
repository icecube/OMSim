
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
    void writePositionStatistics(double x, double wavelength);
    void writePositionPulseStatistics(double x, double y, double wavelength);
    G4String m_outputFileName;
};



