/** @file
 * @ingroup common
 */

#ifndef OMSimHelper_H
#define OMSimHelper_H 1

#include "OMSimLogger.hh"
#include "globals.hh"
/**
 * @class Tools
 * @brief A collection of helper functions for numerical operations.
 * 
 * This class provides static methods for common numerical tasks such as 
 * loading data from a file, and generating linearly and logarithmically 
 * spaced sequences. These methods are utility functions and do not 
 * require instantiation of the `Tools` class.
 * 
 * @ingroup common
 */
class Tools
{
public:
    Tools(){};
    ~Tools(){};
    static std::vector<std::vector<double>> loadtxt(const std::string &pFilePath,
                                                    bool pUnpack = true,
                                                    size_t pSkipRows = 0,
                                                    char pDelimiter = ' ');
    static std::vector<double> linspace(double start, double end, int num_points);
    static std::vector<double> logspace(double start, double end, int num_points);
    static void sortVectorByReference(std::vector<G4double> &referenceVector, std::vector<G4double> &sortVector);
};

#endif
//
