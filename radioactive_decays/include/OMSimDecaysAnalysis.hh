#ifndef OMSimDecaysAnalysis_h
#define OMSimDecaysAnalysis_h 1

#include <G4ThreeVector.hh>
#include <fstream>

class OMSimDecaysAnalysis
{
public:
    static OMSimDecaysAnalysis &getInstance()
    {
        static OMSimDecaysAnalysis instance;
        return instance;
    }

    void writeMultiplicity(const std::vector<int> &pMultiplicity);
    G4String mOutputFileName;
    void appendDecay(G4String pParticleName, G4double pDecayTime, G4ThreeVector pDecayPosition){};

private:
    std::fstream mDatafile;

    OMSimDecaysAnalysis() = default;
    ~OMSimDecaysAnalysis() = default;
    OMSimDecaysAnalysis(const OMSimDecaysAnalysis &) = delete;
    OMSimDecaysAnalysis &operator=(const OMSimDecaysAnalysis &) = delete;
};

#endif
