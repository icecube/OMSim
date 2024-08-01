/** @file
 * @ingroup common
 */

#ifndef OMSimHelper_H
#define OMSimHelper_H 1

#include "OMSimLogger.hh"
#include "globals.hh"

/**
 * @namespace Tools
 * @brief A collection of helper functions for numerical operations.
 * 
 * This namespace provides static methods for common numerical tasks such as 
 * loading data from a file, and generating linearly and logarithmically 
 * spaced sequences. 
 * 
 * @ingroup common
 */
namespace Tools
{
    std::vector<std::vector<double>> loadtxt(const std::string &pFilePath,
                                                    bool pUnpack = true,
                                                    size_t pSkipRows = 0,
                                                    char pDelimiter = ' ');
    std::vector<double> linspace(double start, double end, int num_points);
    std::vector<double> logspace(double start, double end, int num_points);
    void sortVectorByReference(std::vector<G4double> &referenceVector, std::vector<G4double> &sortVector);
    G4String getThreadIDStr();
};

#endif
//
