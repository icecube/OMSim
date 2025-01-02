#ifndef OMSIM_YIELD_SETUP_HH
#define OMSIM_YIELD_SETUP_HH

#include "OMSimDetectorComponent.hh"

class Cs137Source : public OMSimDetectorComponent
{
public:
    Cs137Source();
    void construction();

};

class Am241Source : public OMSimDetectorComponent
{
public:
    Am241Source();
    void construction();

};

class OkamotoLargeSample : public OMSimDetectorComponent
{
public:
    OkamotoLargeSample();
    void construction();
    G4double getSampleThickness();
    
private:
    G4double m_meanThickness;

};


class OkamotoSmallSample : public OMSimDetectorComponent
{
public:
    OkamotoSmallSample();
    void construction();
    G4double getSampleThickness();
    
private:
    G4double m_meanThickness;

};

class AMETEKSiliconDetector : public OMSimDetectorComponent
{
public:
    AMETEKSiliconDetector();
    void construction();
};



#endif // OMSIM_YIELD_SETUP_HH