#ifndef OMSimAngularAnalysis_h
#define OMSimAngularAnalysis_h 1

#include "OMSimPMTResponse.hh"
#include "OMSimHitManager.hh"

#include <G4ThreeVector.hh>
#include <fstream>

struct effectiveAreaResult {
    double EA;
    double EAError;
};

/** 
 * @class OMSimEffectiveAreaAnalyisis
 * @ingroup EffectiveArea
 */
class OMSimEffectiveAreaAnalyisis
{
public:

    static OMSimEffectiveAreaAnalyisis &getInstance()
    {
        static OMSimEffectiveAreaAnalyisis instance;
        return instance;
    }

	void writeScan(G4double pPhi, G4double pTheta);
	void writeHeader();
    effectiveAreaResult calculateEffectiveArea(double pHits);
    
    G4String mOutputFileName;
    std::fstream mDatafile;
private:
    OMSimEffectiveAreaAnalyisis() = default;
    ~OMSimEffectiveAreaAnalyisis() = default;
    OMSimEffectiveAreaAnalyisis(const OMSimEffectiveAreaAnalyisis &) = delete;
    OMSimEffectiveAreaAnalyisis &operator=(const OMSimEffectiveAreaAnalyisis &) = delete;
};

#endif
