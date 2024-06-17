#ifndef OMSIM_YIELD_SETUP_HH
#define OMSIM_YIELD_SETUP_HH

#include "abcDetectorComponent.hh"

class Cs137Source : public abcDetectorComponent
{
public:
    Cs137Source(InputDataManager *pData);
    void construction();

};

class OkamotoLargeSample : public abcDetectorComponent
{
public:
    OkamotoLargeSample(InputDataManager *pData);
    void construction();
    G4double getSampleThickness();
    
private:
    G4double mMeanThickness;

};

#endif // OMSIM_YIELD_SETUP_HH