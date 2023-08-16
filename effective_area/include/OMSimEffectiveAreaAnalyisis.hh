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
    OMSimEffectiveAreaAnalyisis(){};
    ~OMSimEffectiveAreaAnalyisis(){};
	void writeScan(G4double pPhi, G4double pTheta);
	void writeHeader();
    effectiveAreaResult calculateEffectiveArea(double pHits);

    G4String mOutputFileName;
    std::fstream mDatafile;
private:

};

#endif
