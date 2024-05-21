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

    template<typename... Args>
	void writeScan(Args... args);
    template<typename... Args>
	void writeHeader(Args... args);

    effectiveAreaResult calculateEffectiveArea(double pHits);

    G4String mOutputFileName;
    std::fstream mDatafile;
private:

};

/**
 * @brief Writes a scan result to the output file.
 * @param args The values to be written to the output file.
 */
template<typename... Args>
void OMSimEffectiveAreaAnalyisis::writeScan(Args... args) {
    std::vector<double> lHits = OMSimHitManager::getInstance().countHits();
    mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);

    // Write all arguments to the file
   ((mDatafile << args << "\t"), ...);


    G4double lTotalHits = 0;
    for (const auto &hit : lHits) {
        mDatafile << hit << "\t";
        lTotalHits = hit; // last element is total nr of hits
    }

    effectiveAreaResult lEffectiveArea = calculateEffectiveArea(lTotalHits);
    mDatafile << lEffectiveArea.EA << "\t" << lEffectiveArea.EAError << "\t";
    mDatafile << G4endl;
    mDatafile.close();
}


/**
 * @brief Writes the header line to the output file.
 */
template<typename... Args>
void OMSimEffectiveAreaAnalyisis::writeHeader(Args... args)
{
	mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);
	mDatafile << "# ";
     ((mDatafile << args << "\t"), ...);
    mDatafile << "hits[1perPMT]"
			  << "\t"
			  << "total_hits"		
			  << "\t"
			  << "EA_Total(cm^2)"
			  << "\t"
			  << "EA_Total_error(cm^2)"
			  << "\t" << G4endl;
	mDatafile.close();
}


#endif
