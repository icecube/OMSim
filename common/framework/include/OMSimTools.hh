/** @file
 * @brief Definition of Tools namespace, a collection of helper methods.
 * @ingroup common
 */

#pragma once

#include "OMSimLogger.hh"
#include "globals.hh"
#include <TGraph.h>
#include <TH2D.h>
#include <variant>
#include <optional>
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
    std::vector<double> arange(double start, double stop, double step);
    TGraph* create1dInterpolator(const std::vector<double>& pX, const std::vector<double>& pY, const std::string& pName);
    TGraph* create1dInterpolator(const std::string& pFileName);
    TH2D *create2DHistogramFromDataFile(const std::string &pFilePath);

    void ensureDirectoryExists(const std::string &filepath);
    std::pair<std::vector<double>, std::vector<double>> histogram(
        const std::vector<double> &data,
        const std::variant<int, std::vector<double>> &bins = 10,
        const std::optional<std::pair<double, double>> &range = std::nullopt,
        const std::vector<double> &weights = std::vector<double>());

    G4String getThreadIDStr();
    std::vector<std::vector<double>> loadtxt(const std::string &pFilePath,
                                             bool pUnpack = true,
                                             size_t pSkipRows = 0,
                                             char pDelimiter = ' ',
                                             char pComments = '#');
    std::vector<double> linspace(double start, double end, int num_points, bool endpoint = true);
    std::vector<double> logspace(double start, double end, int num_points, double base = 10.0, bool endpoint = true);
    void sortVectorByReference(std::vector<G4double> &referenceVector, std::vector<G4double> &sortVector);
    double median(std::vector<double> p_vec);
    double mean(const std::vector<double> &p_vec, const std::vector<double> &p_weights = {});
    double std(const std::vector<double>& vec,  const std::vector<double> &p_weights = {});
    
    void throwError(const G4String& message);
    std::vector<G4String> splitStringByDelimiter(G4String const &p_string, char p_delim);
    std::vector<G4String> splitStringByDelimiter(char *p_char, char p_delim);
    extern std::string visualisationURL;
};

