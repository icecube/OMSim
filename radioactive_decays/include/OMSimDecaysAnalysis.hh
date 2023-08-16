#ifndef OMSimDecaysAnalysis_h
#define OMSimDecaysAnalysis_h 1

#include <G4ThreeVector.hh>
#include <fstream>

class OMSimDecaysAnalysis
{
public:
    OMSimDecaysAnalysis(){};
    ~OMSimDecaysAnalysis(){};
    void writeMultiplicity(const std::vector<int> &pMultiplicity);
    G4String mOutputFileName;

private:
    
    std::fstream mDatafile;
};

#endif
