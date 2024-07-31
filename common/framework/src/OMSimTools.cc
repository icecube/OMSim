#include "OMSimTools.hh"
#include <fstream>    // For std::ifstream
#include <sstream>    // For std::stringstream
#include <stdexcept>  // For std::runtime_error
#include <numeric>
/**
 * @brief Reads numerical data from a file and returns it as a 2D vector.
 * Similar to numpy.loadtxt.
 *
 * @param pFilePath The path to the input file.
 * @param pUnpack Optional. If true, the returned data is transposed, i.e.,
 * unpacked into columns. Default is true.
 * @param pSkipRows Optional. The number of lines to skip at the beginning of
 * the file. Default is 0.
 * @param pDelimiter The character used to separate values in each line of the
 * input file.
 * @return A 2D vector of doubles. The outer vector groups all columns (or
 * rows if 'unpack' is false), and each inner vector represents one of the
 * columns (or one of the rows if 'unpack' is false) of data file.
 * @throws std::runtime_error if the file cannot be opened.
 */
std::vector<std::vector<double>> Tools::loadtxt(const std::string &pFilePath, bool pUnpack,
												size_t pSkipRows, char pDelimiter)
{
	std::vector<std::vector<double>> lData;
	std::ifstream lInFile(pFilePath);

	if (!lInFile.is_open())
	{
		log_error("Could not open file {}", pFilePath);
		throw std::runtime_error("Could not open file " + pFilePath);
	}

	std::string lLine;
	size_t lRowCounter = 0;

	while (getline(lInFile, lLine))
	{
		if (lRowCounter++ < pSkipRows)
			continue;
		std::vector<double> lRow;
		std::stringstream lSs(lLine);
		std::string lItem;
		while (getline(lSs, lItem, pDelimiter))
		{
			lRow.push_back(stod(lItem));
		}
		lData.push_back(lRow);
	}

	if (pUnpack)
	{
		size_t lNumCols = lData[0].size();
		std::vector<std::vector<double>> lTransposedData(
			lNumCols, std::vector<double>(lData.size()));

		for (size_t i = 0; i < lData.size(); ++i)
		{
			for (size_t j = 0; j < lNumCols; ++j)
			{
				lTransposedData[j][i] = lData[i][j];
			}
		}
		return lTransposedData;
	}
	else
	{
		return lData;
	}
}

/**
 * @brief Generates a linearly spaced vector.
 *
 * Creates a vector of equally spaced values between `start` and `end`.
 *
 * @param start The starting value of the sequence.
 * @param end The ending value of the sequence.
 * @param num_points The number of points to generate in the sequence.
 * @return A vector of linearly spaced values.
 * @throws std::invalid_argument if `num_points` is less than 2.
 */
std::vector<double> Tools::linspace(double start, double end, int num_points)
{
	if (num_points < 2)
	{
		throw std::invalid_argument("Number of points must be at least 2.");
	}
	std::vector<double> vector;
	double step = (end - start) / (num_points - 1);
	for (int i = 0; i < num_points; ++i)
	{
		vector.push_back(start + i * step);
	}
	return vector;
}

/**
 * @brief Generates a logarithmically spaced vector.
 *
 * Creates a vector of values that are logarithmically spaced between `start` and `end`.
 *
 * @param start The starting value of the sequence (log10 of the starting value).
 * @param end The ending value of the sequence (log10 of the ending value).
 * @param num_points The number of points to generate in the sequence.
 * @return A vector of logarithmically spaced values.
 * @throws std::invalid_argument if `num_points` is less than 2 or if `start` or `end` are non-positive.
 */

std::vector<double> Tools::logspace(double start, double end, int num_points)
{

	if (num_points < 2)
	{
		throw std::invalid_argument("Number of points must be at least 2.");
	}
	if (start <= 0 || end <= 0)
	{
		throw std::invalid_argument("Start and end must be positive values.");
	}
	std::vector<double> result;
	result.reserve(num_points);

	double start_log = std::log10(start);
	double end_log = std::log10(end);
	double step = (end_log - start_log) / (num_points - 1);

	for (int i = 0; i < num_points; ++i)
	{
		double value = std::pow(10, start_log + i * step);
		result.push_back(value);
	}

	return result;
}



/**
 *  @brief Sorts two vectors (pSortVector & pReferenceVector) based on the order of values in pReferenceVector.
 *
 *  @param pReferenceVector The ordering of these values will determine the final order of both vectors.
 *  @param pSortVector The vector to be sorted according to the pReferenceVector.
 *
 *  @throws std::invalid_argument if the vectors do not have the same size.
 */
void Tools::sortVectorByReference(std::vector<G4double> &pReferenceVector, std::vector<G4double> &pSortVector)
{
    log_trace("Sorting vector");
    // Check if the vectors have the same size
    if (pReferenceVector.size() != pSortVector.size())
    {
        // Handle error
        throw std::invalid_argument("The two vectors must have the same size.");
    }

    // Create a vector of indices
    std::vector<std::size_t> lIndices(pReferenceVector.size());
    std::iota(lIndices.begin(), lIndices.end(), 0);

    // Sort the indices based on the values in pReferenceVector
    std::sort(lIndices.begin(), lIndices.end(),
              [&pReferenceVector](std::size_t i1, std::size_t i2)
              { return pReferenceVector[i1] < pReferenceVector[i2]; });

    // Create temporary vectors to hold the sorted data
    std::vector<G4double> lSortedSortVector(pSortVector.size());
    std::vector<G4double> lSortedReferenceVector(pReferenceVector.size());

    // Apply the sorted indices to the vectors
    for (std::size_t i = 0; i < lIndices.size(); ++i)
    {
        lSortedSortVector[i] = pSortVector[lIndices[i]];
        lSortedReferenceVector[i] = pReferenceVector[lIndices[i]];
    }

    // Replace the original vectors with the sorted ones
    pSortVector = std::move(lSortedSortVector);
    pReferenceVector = std::move(lSortedReferenceVector);
}