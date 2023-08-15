#ifndef OMSimAngularAnalysis_h
#define OMSimAngularAnalysis_h 1

#include "OMSimPMTResponse.hh"
#include "OMSimHitManager.hh"

#include <G4ThreeVector.hh>
#include <fstream>

/** 
 * @class OMSimEffectiveAreaAnalyisis
 * @ingroup EffectiveArea
 */
class OMSimEffectiveAreaAnalyisis : public OMSimHitManager
{
public:
    static OMSimEffectiveAreaAnalyisis &getInstance()
    {
        static OMSimEffectiveAreaAnalyisis instance;
        return instance;
    }
	void writeScan(G4double pPhi, G4double pTheta);
	void writeHeader();

private:
    OMSimEffectiveAreaAnalyisis() = default;
    ~OMSimEffectiveAreaAnalyisis() = default;
    OMSimEffectiveAreaAnalyisis(const OMSimEffectiveAreaAnalyisis &) = delete;
    OMSimEffectiveAreaAnalyisis &operator=(const OMSimEffectiveAreaAnalyisis &) = delete;
};

#endif
