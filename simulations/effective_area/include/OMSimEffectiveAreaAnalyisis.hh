/**
 * @file OMSimAngularAnalysis.hh
 * @brief Defines the OMSimEffectiveAreaAnalyisis class for calculating effective area and writing results to output file.
 * @ingroup EffectiveArea
 */
#pragma once

#include "OMSimPMTResponse.hh"
#include "OMSimHitManager.hh"

#include <G4ThreeVector.hh>
#include <fstream>

/**
 * @brief Struct to hold results of effective area calculations.
 */
struct effectiveAreaResult
{
    double EA;      ///< Effective area.
    double EAError; ///< Uncertainty of effective area.
};

/**
 * @class OMSimEffectiveAreaAnalyisis
 * @brief Responsible for calculating the effective area of optical hits and saving the results.
 * @ingroup EffectiveArea
 */
class OMSimEffectiveAreaAnalyisis
{
private:
    std::vector<int> m_totalMultiplicity;//+++++++
public:
   // OMSimEffectiveAreaAnalyisis(){};
   // ~OMSimEffectiveAreaAnalyisis(){};
    OMSimEffectiveAreaAnalyisis() : m_totalMultiplicity(24, 0) {}//+++++++
    void writeTotalMultiplicity(G4double p_TimeWindow);//+++++++
    void accumulateMultiplicity(G4double p_TimeWindow);//+++++++
    template <typename... Args>
    void writeScan(Args... p_args);
    template <typename... Args>
    void writeHeader(Args... p_args);

    effectiveAreaResult calculateEffectiveArea(double weightedTotal, double countTotal);
    void writeMultiplicity(G4double p_TimeWindow);
    void writeHitInformation();
    G4String m_outputFileNameInfo;
    G4String m_outputFileNameMultiplicity;
    G4String m_outputFileName;
    std::fstream mDatafile;
private:
    G4double mMuonEnergy;
public:
    void setMuonEnergy(G4double energy) { mMuonEnergy = energy; }

};

/**
 * @brief Writes a scan result to the output file.
 * @param p_args The values to be written to the output file.
 */
template <typename... Args>

//writes only if there was a hit
void OMSimEffectiveAreaAnalyisis::writeScan(Args... p_args)
{
    std::vector<double> hits = OMSimHitManager::getInstance().countMergedHits(0, true);
    bool hasNonZero = std::any_of(hits.begin(), hits.end(), [](double val) {
        return val != 0.0;
    });
    if (!hasNonZero) return; 
    std::fstream dataFile;
    dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);
    ((dataFile << p_args << "\t"), ...);
    G4double weightedTotal = 0;
    for (const auto &hit : hits)
    {
        dataFile << hit << "\t";
        weightedTotal = hit;
    }
    hits = OMSimHitManager::getInstance().countMergedHits();
    G4double totalHits = 0;
    for (const auto &hit : hits)
    {
        totalHits = hit;
    }
    effectiveAreaResult effectiveArea = calculateEffectiveArea(weightedTotal, totalHits);
  //  dataFile << effectiveArea.EA << "\t" << effectiveArea.EAError << "\t";
    dataFile << G4endl;
    dataFile.close();
}


// writes all particles, no matter if hit or not
/*void OMSimEffectiveAreaAnalyisis::writeScan(Args... p_args)
{
    std::vector<double> hits = OMSimHitManager::getInstance().countMergedHits(0, true);
    std::fstream dataFile;
    dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);
    // Write all arguments to the file
    ((dataFile << p_args << "\t"), ...);
    G4double weightedTotal = 0;
    for (const auto &hit : hits)
    {
        dataFile << hit << "\t";
        weightedTotal = hit;
    }
    hits = OMSimHitManager::getInstance().countMergedHits(); //unweighted
    G4double totalHits = 0;
    for (const auto &hit : hits)
    {
        totalHits = hit; 
    }
    effectiveAreaResult effectiveArea = calculateEffectiveArea(weightedTotal, totalHits);
    dataFile << effectiveArea.EA << "\t" << effectiveArea.EAError << "\t";
    dataFile << G4endl;
    dataFile.close();
}*/

/**
 * @brief Writes the header line to the output file.
 */
template <typename... Args>
void OMSimEffectiveAreaAnalyisis::writeHeader(Args... p_args)
{
    std::fstream dataFile;
    dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);
    dataFile << "# ";
    ((dataFile << p_args << "\t"), ...);
    dataFile << "hits[1perPMT]"
              << "\t"
              << "total_hits"
              << "\t"
            //  << "EA_Total(cm^2)"
            //  << "\t"
            //  << "EA_Total_error(cm^2)"
              << "\t" << G4endl;
    dataFile.close();
}
