#include "OMSimTools.hh"
#include <G4Threading.hh>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <numeric>
#include <filesystem>
#include <TH1D.h>

namespace Tools
{

	/**
	 * @brief Generates a sequence of numbers.
	 *
	 * @param start The start of the interval. The interval includes this value.
	 * @param stop The end of the interval. The interval does not include this value.
	 * @param step The spacing between values. For any output out, this is the distance between two adjacent values, out[i+1] - out[i].
	 * @return A vector of evenly spaced values.
	 * @throws std::invalid_argument if step is zero.
	 */
	std::vector<double> arange(double start, double stop, double step)
	{

		if (step == 0.0)
		{
			throw std::invalid_argument("Step cannot be zero.");
		}

		std::vector<double> result;
		// Calculate the number of elements
		double n = std::ceil((stop - start) / step);

		if (n <= 0)
		{
			return result; // Return empty vector if no elements
		}

		result.reserve(static_cast<size_t>(n));

		for (int i = 0; i < n; ++i)
		{
			result.push_back(start + i * step);
		}

		// Handle potential floating point issues
		if (!result.empty() && step > 0 && result.back() >= stop)
		{
			result.pop_back();
		}
		else if (!result.empty() && step < 0 && result.back() <= stop)
		{
			result.pop_back();
		}

		return result;
	}

	/**
	 * @brief Compute the histogram of a dataset.
	 *
	 * @param data Input data. The histogram is computed over the data.
	 * @param bins The number of equal-width bins in the given range (10, by default).
	 *             If bins is a sequence, it defines the bin edges, including the rightmost edge.
	 * @param range The lower and upper range of the bins. If not provided, range is simply (min(data), max(data)).
	 *              Values outside the range are ignored.
	 * @return A pair of vectors:
	 *         - The first vector contains the values of the histogram (counts in each bin).
	 *         - The second vector contains the bin edges.
	 *
	 * @note This function behaves similarly to NumPy's numpy.histogram:
	 *       - It returns the values of the histogram and the bin edges.
	 *       - The last bin includes the right edge.
	 *       - Values outside the range are ignored.
	 *       - If bins is an integer, it represents the number of bins. If it's a vector, it represents the bin edges.
	 */
	std::pair<std::vector<double>, std::vector<double>> histogram(const std::vector<double> &data,
																  const std::variant<int, std::vector<double>> &bins,
																  const std::optional<std::pair<double, double>> &range)
	{

		// Handle empty input
		if (data.empty())
		{
			if (std::holds_alternative<int>(bins))
			{
				int nbins = std::get<int>(bins);
				return {std::vector<double>(nbins, 0.0), std::vector<double>(nbins + 1)};
			}
			else
			{
				const auto &bin_edges = std::get<std::vector<double>>(bins);
				return {std::vector<double>(bin_edges.size() - 1, 0.0), bin_edges};
			}
		}

		double data_min, data_max;
		if (range)
		{
			data_min = range->first;
			data_max = range->second;
		}
		else
		{
			auto [min_it, max_it] = std::minmax_element(data.begin(), data.end());
			data_min = *min_it;
			data_max = *max_it;
			if (data_min == data_max)
			{
				// Single value case
				data_min -= 0.5;
				data_max += 0.5;
			}
		}

		std::vector<double> bin_edges;
		if (std::holds_alternative<int>(bins))
		{
			int nbins = std::get<int>(bins);
			bin_edges.resize(nbins + 1);
			double step = (data_max - data_min) / nbins;
			for (int i = 0; i <= nbins; ++i)
			{
				bin_edges[i] = data_min + i * step;
			}
		}
		else
		{
			bin_edges = std::get<std::vector<double>>(bins);
		}

		std::vector<double> counts(bin_edges.size() - 1, 0.0);

		for (double value : data)
		{
			if (value >= data_min && value <= data_max)
			{
				auto it = std::upper_bound(bin_edges.begin(), bin_edges.end(), value);
				if (it != bin_edges.begin())
				{
					int index = std::distance(bin_edges.begin(), it) - 1;
					if (index == counts.size() && value == data_max)
					{
						counts.back()++;
					}
					else
					{
						counts[index]++;
					}
				}
			}
		}

		return {counts, bin_edges};
	}

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
	std::vector<std::vector<double>> loadtxt(const std::string &pFilePath, bool pUnpack,
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
	 * @param stop The end value of the sequence, unless endpoint is False.
	 *             In that case, the sequence consists of all but the last of num + 1 evenly spaced samples,
	 *             so that stop is excluded. Note that the step size changes when endpoint is False.
	 * @param num The number of points to generate in the sequence.
	 * @param endpoint If True (default), stop is the last sample. Otherwise, it is not included.
	 * @return A vector of linearly spaced values.
	 * @throws std::invalid_argument if `num` is less than 2.
	 */
	std::vector<double> linspace(double start, double stop, int num, bool endpoint)
	{
		if (num < 2)
		{
			throw std::invalid_argument("Number of points must be at least 2.");
		}
		std::vector<double> vector;
		double step = (stop - start) / (endpoint ? (num - 1) : num);
		for (int i = 0; i < num; ++i)
		{
			vector.push_back(start + i * step);
		}
		return vector;
	}

	/**
	 * @brief Generates a logarithmically spaced vector.
	 *
	 * Creates a vector of values that are logarithmically spaced. The sequence starts at base^start
	 * and ends at base^stop.
	 *
	 * @param start The starting value of the sequence (as a power of base).
	 * @param stop The ending value of the sequence (as a power of base).
	 * @param num The number of points to generate in the sequence. Default is 50.
	 * @param base The base of the log space. Default is 10.0.
	 * @param endpoint Whether to include the stop value in the output. Default is true.
	 * @return A vector of logarithmically spaced values.
	 * @throws std::invalid_argument if `num` is negative.
	 *
	 * @note This function behaves similarly to NumPy's np.logspace:
	 *       - If num is 0, returns an empty vector.
	 *       - start and stop are used as powers of base.
	 *       - The sequence includes base^start and base^stop if endpoint is true.
	 */
	std::vector<double> logspace(double start, double stop, int num, double base, bool endpoint)
	{
		if (num < 0)
		{
			throw std::invalid_argument("Number of samples must be non-negative.");
		}

		std::vector<double> result;
		result.reserve(num);

		if (num == 0)
		{
			return result;
		}

		double start_log = start;
		double stop_log = stop;

		double step = (stop_log - start_log) / (endpoint ? (num - 1) : num);

		for (int i = 0; i < num; ++i)
		{
			double value = std::pow(base, start_log + i * step);
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
	void sortVectorByReference(std::vector<G4double> &pReferenceVector, std::vector<G4double> &pSortVector)
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

	/**
	 * @return Thread id which called method as string
	 */
	G4String getThreadIDStr()
	{
		std::ostringstream oss;
		oss << G4Threading::G4GetThreadId();
		G4String threadIdStr = oss.str();
		return threadIdStr;
	}

	/**
	 * @brief Ensure that the directory of a file to be created exists
	 * @param pFilePath The path of file.
	 */
	void ensureDirectoryExists(const std::string &pFilePath)
	{
		std::filesystem::path full_path(pFilePath);
		std::filesystem::path dir = full_path.parent_path();
		if (!dir.empty() && !std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}
	}

}