 /**
 * @file OMSimAngularAnalysis.hh
 * @brief Defines the OMSimEffectiveAreaAnalyisis class for calculating effective area and writing results to output file.
 * @ingroup EffectiveArea
 */
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

/**
 * @brief Writes a scan result to the output file.
 * @param pPhi The phi angle used in the scan to be written to the output file.
 * @param pTheta The phi angle used in the scan to be written to the output file.
 */
	void writeScan(G4double pPhi, G4double pTheta);
	void writeHeader();
    effectiveAreaResult calculateEffectiveArea(double pHits);

    G4String mOutputFileName;
    std::fstream mDatafile;
private:

};

#endif
