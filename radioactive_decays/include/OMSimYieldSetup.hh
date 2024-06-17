#ifndef Cs137Source_h
#define Cs137Source_h 1

#include "abcDetectorComponent.hh"

class Cs137Source : abcDetectorComponent
{
public:
    Cs137Source(InputDataManager *pData);
    ~Cs137Source()=default;
    void construction();

};

class OkamotoLargeSample : abcDetectorComponent
{
public:
    OkamotoLargeSample(InputDataManager *pData);
    ~OkamotoLargeSample()=default;
    void construction();

};

#endif