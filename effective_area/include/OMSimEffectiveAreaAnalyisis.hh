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

/**
 * @brief Struct to hold results of effective area calculations.
 */
struct effectiveAreaResult {
    double EA; ///< Effective area.
    double EAError; ///< Uncertainty of effective area.
};

/** 
 * @class OMSimEffectiveAreaAnalyisis
 * @brief Responsible for calculating the effective area of optical hits and saving the results.
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
