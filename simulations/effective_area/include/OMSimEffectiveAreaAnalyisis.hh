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
public:
    OMSimEffectiveAreaAnalyisis(){};
    ~OMSimEffectiveAreaAnalyisis(){};

    template <typename... Args>
    void writeScan(Args... p_args);
    template <typename... Args>
    void writeHeader(Args... p_args);

    effectiveAreaResult calculateEffectiveArea(double p_hits);
    G4String m_outputFileName;
};

/**
 * @brief Writes a scan result to the output file.
 * @param p_args The values to be written to the output file.
 */
template <typename... Args>
void OMSimEffectiveAreaAnalyisis::writeScan(Args... p_args)
{
    std::vector<double> hits = OMSimHitManager::getInstance().countMergedHits();

    std::fstream dataFile;
    dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);

    // Write all arguments to the file
    ((dataFile << p_args << "\t"), ...);

    G4double totalHits = 0;
    for (const auto &hit : hits)
    {
        dataFile << hit << "\t";
        totalHits = hit; // last element is total nr of hits
    }

    effectiveAreaResult effectiveArea = calculateEffectiveArea(totalHits);
    dataFile << effectiveArea.EA << "\t" << effectiveArea.EAError << "\t";
    dataFile << G4endl;
    dataFile.close();
}

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
              << "EA_Total(cm^2)"
              << "\t"
              << "EA_Total_error(cm^2)"
              << "\t" << G4endl;
    dataFile.close();
}
