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
	 * @param p_start The p_start of the interval. The interval includes this value.
	 * @param p_stop The end of the interval. The interval does not include this value.
	 * @param p_step The spacing between values. For any output out, this is the distance between two adjacent values, out[i+1] - out[i].
	 * @return A vector of evenly spaced values.
	 * @throws std::invalid_argument if p_step is zero.
	 */
	std::vector<double> arange(double p_start, double p_stop, double p_step)
	{

		if (p_step == 0.0)
		{
			throw std::invalid_argument("Step cannot be zero.");
		}

		std::vector<double> result;
		// Calculate the number of elements
		double n = std::ceil((p_stop - p_start) / p_step);

		if (n <= 0)
		{
			return result; // Return empty vector if no elements
		}

		result.reserve(static_cast<size_t>(n));

		for (int i = 0; i < n; ++i)
		{
			result.push_back(p_start + i * p_step);
		}

		// Handle potential floating point issues
		if (!result.empty() && p_step > 0 && result.back() >= p_stop)
		{
			result.pop_back();
		}
		else if (!result.empty() && p_step < 0 && result.back() <= p_stop)
		{
			result.pop_back();
		}

		return result;
	}

	/**
	 * @brief Creates a TGraph interpolator from x and y data points.
	 *
	 * @param p_X Vector of x-coordinates.
	 * @param p_y Vector of y-coordinates.
	 * @param p_name Optional name for the TGraph. It should be unique!
	 * @return Pointer to the new TGraph object.
	 * @note Caller is responsible for deleting the returned TGraph!
	 */
	TGraph *create1dInterpolator(const std::vector<double> &p_X, const std::vector<double> &p_y, const std::string &p_name)
	{
		auto interpolator = new TGraph(p_X.size(), p_X.data(), p_y.data());
		if (!p_name.empty())
		{
			interpolator->SetName(p_name.c_str());
		}
		return interpolator;
	}

	/**
	 * @brief Creates a TGraph interpolator from x and y in file.
	 * @param p_filename
	 * @return Pointer to the new TGraph object.
	 * @note Caller is responsible for deleting the returned TGraph!
	 */
	TGraph *create1dInterpolator(const std::string &p_filename)
	{
		auto interpolator = new TGraph(p_filename.c_str());
		interpolator->SetName(p_filename.c_str());
		return interpolator;
	}

	/**
	 * @brief Create a histogram from provided data.
	 *
	 * Loads the data from a given path and constructs a histogram based on the data.
	 *
	 * @param p_filePath Path to the data file.
	 * @param pTH2DName Name of the histogram.
	 * @return Pointer to the created histogram.
	 * @note Caller is responsible for deleting the returned TH2D!
	 */
	TH2D *create2DHistogramFromDataFile(const std::string &p_filePath)
	{
		// Load the data
		std::vector<std::vector<double>> data = Tools::loadtxt(p_filePath, true, 0, '\t');

		// Deduce the number of bins and the bin widths
		double lBinWidthX, lBinWidthY;
		for (size_t i = 1; i < data[0].size(); i++)
		{
			if (data[0][i] - data[0][i - 1] > 0)
			{
				lBinWidthX = data[0][i] - data[0][i - 1];
				break;
			}
		}
		for (size_t i = 1; i < data[1].size(); i++)
		{
			if (data[1][i] - data[1][i - 1] > 0)
			{
				lBinWidthY = data[1][i] - data[1][i - 1];
				break;
			}
		}
		double minX = *std::min_element(data[0].begin(), data[0].end()) - lBinWidthX / 2.0;
		double maxX = *std::max_element(data[0].begin(), data[0].end()) + lBinWidthX / 2.0;
		double minY = *std::min_element(data[1].begin(), data[1].end()) - lBinWidthY / 2.0;
		double maxY = *std::max_element(data[1].begin(), data[1].end()) + lBinWidthY / 2.0;
		int numberBinsX = (int)((maxX - minX) / lBinWidthX);
		int numberBinsY = (int)((maxY - minY) / lBinWidthY);

		// Create histogram
		TH2D *histogramTH2D = new TH2D(p_filePath.c_str(), "title", numberBinsX, minX, maxX, numberBinsY, minY, maxY);

		// Fill the histogram
		for (size_t i = 0; i < data[0].size(); i++)
		{
			histogramTH2D->Fill(data[0][i], data[1][i], data[2][i]);
		}

		return histogramTH2D;
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
																  const std::optional<std::pair<double, double>> &p_range,
																  const std::vector<double> &p_weights)
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
		std::vector<double> actual_weights = (p_weights.empty() || p_weights.size() != data.size())
												 ? std::vector<double>(data.size(), 1.0)
												 : p_weights;
		double data_min, data_max;
		if (p_range)
		{
			data_min = p_range->first;
			data_max = p_range->second;
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
			double p_step = (data_max - data_min) / nbins;
			for (int i = 0; i <= nbins; ++i)
			{
				bin_edges[i] = data_min + i * p_step;
			}
		}
		else
		{
			bin_edges = std::get<std::vector<double>>(bins);
		}

		std::vector<double> counts(bin_edges.size() - 1, 0.0);

		for (size_t i = 0; i < data.size(); ++i)
		{
			double value = data[i];
			double weight = actual_weights[i];
			if (value >= data_min && value <= data_max)
			{
				auto it = std::upper_bound(bin_edges.begin(), bin_edges.end(), value);
				if (it != bin_edges.begin())
				{
					int index = std::distance(bin_edges.begin(), it) - 1;
					if (index == counts.size() && value == data_max)
					{
						counts.back() += weight;
					}
					else
					{
						counts[index] += weight;
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
	 * @param p_filePath The path to the input file.
	 * @param p_unpack Optional. If true, the returned data is transposed, i.e.,
	 * unpacked into columns. Default is true.
	 * @param p_skipRows Optional. The number of lines to skip at the beginning of
	 * the file. Default is 0.
	 * @param p_delimiter The character used to separate values in each line of the
	 * input file.
	 * @param p_comments Optional. The character used to indicate the p_start of a
	 * comment. Default is '#'.
	 * @return A 2D vector of doubles. The outer vector groups all columns (or
	 * rows if 'unpack' is false), and each inner vector represents one of the
	 * columns (or one of the rows if 'unpack' is false) of data file.
	 * @throws std::runtime_error if the file cannot be opened.
	 */
	std::vector<std::vector<double>> loadtxt(const std::string &p_filePath, bool p_unpack,
											 size_t p_skipRows, char p_delimiter, char p_comments)
	{
		std::vector<std::vector<double>> data;
		std::ifstream inFile(p_filePath);

		if (!inFile.is_open())
		{
			log_error("Could not open file {}", p_filePath);
			throw std::runtime_error("Could not open file " + p_filePath);
		}

		std::string line;
		size_t rowCounter = 0;

		while (getline(inFile, line))
		{
			if (rowCounter++ < p_skipRows)
				continue;
			if (!line.empty() && line[0] == p_comments)
				continue;
			std::vector<double> row;
			std::stringstream ss(line);
			std::string item;
			while (getline(ss, item, p_delimiter))
			{
				row.push_back(stod(item));
			}
			data.push_back(row);
		}

		if (p_unpack)
		{
			size_t numCols = data[0].size();
			std::vector<std::vector<double>> transposedData(
				numCols, std::vector<double>(data.size()));

			for (size_t i = 0; i < data.size(); ++i)
			{
				for (size_t j = 0; j < numCols; ++j)
				{
					transposedData[j][i] = data[i][j];
				}
			}
			return transposedData;
		}
		else
		{
			return data;
		}
	}

	/**
	 * @brief Generates a linearly spaced vector.
	 *
	 * Creates a vector of equally spaced values between `p_start` and `end`.
	 *
	 * @param p_start The starting value of the sequence.
	 * @param p_stop The end value of the sequence, unless p_endpoint is False.
	 *             In that case, the sequence consists of all but the last of p_num + 1 evenly spaced samples,
	 *             so that p_stop is excluded. Note that the p_step size changes when p_endpoint is False.
	 * @param p_num The number of points to generate in the sequence.
	 * @param p_endpoint If True (default), p_stop is the last sample. Otherwise, it is not included.
	 * @return A vector of linearly spaced values.
	 * @throws std::invalid_argument if `p_num` is less than 2.
	 */
	std::vector<double> linspace(double p_start, double p_stop, int p_num, bool p_endpoint)
	{
		if (p_num < 2)
		{
			throw std::invalid_argument("Number of points must be at least 2.");
		}
		std::vector<double> vector;
		double p_step = (p_stop - p_start) / (p_endpoint ? (p_num - 1) : p_num);
		for (int i = 0; i < p_num; ++i)
		{
			vector.push_back(p_start + i * p_step);
		}
		return vector;
	}

	/**
	 * @brief Generates a logarithmically spaced vector.
	 *
	 * Creates a vector of values that are logarithmically spaced. The sequence starts at base^p_start
	 * and ends at base^p_stop.
	 *
	 * @param p_start The starting value of the sequence (as a power of base).
	 * @param p_stop The ending value of the sequence (as a power of base).
	 * @param p_num The number of points to generate in the sequence. Default is 50.
	 * @param base The base of the log space. Default is 10.0.
	 * @param p_endpoint Whether to include the p_stop value in the output. Default is true.
	 * @return A vector of logarithmically spaced values.
	 * @throws std::invalid_argument if `p_num` is negative.
	 *
	 * @note This function behaves similarly to NumPy's np.logspace:
	 *       - If p_num is 0, returns an empty vector.
	 *       - p_start and p_stop are used as powers of base.
	 *       - The sequence includes base^p_start and base^p_stop if p_endpoint is true.
	 */
	std::vector<double> logspace(double p_start, double p_stop, int p_num, double base, bool p_endpoint)
	{
		if (p_num < 0)
		{
			throw std::invalid_argument("Number of samples must be non-negative.");
		}

		std::vector<double> result;
		result.reserve(p_num);

		if (p_num == 0)
		{
			return result;
		}

		double start_log = p_start;
		double stop_log = p_stop;

		double p_step = (stop_log - start_log) / (p_endpoint ? (p_num - 1) : p_num);

		for (int i = 0; i < p_num; ++i)
		{
			double value = std::pow(base, start_log + i * p_step);
			result.push_back(value);
		}

		return result;
	}

	/**
	 *  @brief Sorts two vectors (p_sortVector & p_referenceVector) based on the order of values in p_referenceVector.
	 *
	 *  @param p_referenceVector The ordering of these values will determine the final order of both vectors.
	 *  @param p_sortVector The vector to be sorted according to the p_referenceVector.
	 *
	 *  @throws std::invalid_argument if the vectors do not have the same size.
	 */
	void sortVectorByReference(std::vector<G4double> &p_referenceVector, std::vector<G4double> &p_sortVector)
	{
		log_trace("Sorting vector");
		// Check if the vectors have the same size
		if (p_referenceVector.size() != p_sortVector.size())
		{
			// Handle error
			throw std::invalid_argument("The two vectors must have the same size.");
		}

		// Create a vector of indices
		std::vector<std::size_t> indices(p_referenceVector.size());
		std::iota(indices.begin(), indices.end(), 0);

		// Sort the indices based on the values in p_referenceVector
		std::sort(indices.begin(), indices.end(),
				  [&p_referenceVector](std::size_t i1, std::size_t i2)
				  { return p_referenceVector[i1] < p_referenceVector[i2]; });

		// Create temporary vectors to hold the sorted data
		std::vector<G4double> sortedSortVector(p_sortVector.size());
		std::vector<G4double> sortedReferenceVector(p_referenceVector.size());

		// Apply the sorted indices to the vectors
		for (std::size_t i = 0; i < indices.size(); ++i)
		{
			sortedSortVector[i] = p_sortVector[indices[i]];
			sortedReferenceVector[i] = p_referenceVector[indices[i]];
		}

		// Replace the original vectors with the sorted ones
		p_sortVector = std::move(sortedSortVector);
		p_referenceVector = std::move(sortedReferenceVector);
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
	 * @param p_filePath The path of file.
	 */
	void ensureDirectoryExists(const std::string &p_filePath)
	{
		std::filesystem::path full_path(p_filePath);
		std::filesystem::path dir = full_path.parent_path();
		if (!dir.empty() && !std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
		}
	}

	void throwError(const G4String &p_message)
	{
		log_error(p_message);
		throw std::runtime_error(p_message);
	}

	std::vector<G4String> splitStringByDelimiter(G4String const &p_string, char p_delim)
	{
		std::vector<G4String> result;
		std::istringstream iss(p_string);

		for (G4String token; std::getline(iss, token, p_delim);)
		{
			result.push_back(std::move(token));
		}

		return result;
	}

	std::vector<G4String> splitStringByDelimiter(char *p_char, char p_delim)
	{
		return splitStringByDelimiter(G4String(p_char), p_delim);
	}

	double median(std::vector<double> p_vec)
	{
		if (p_vec.empty())
		{
			throwError("Vector is empty");
		}
		auto n = p_vec.size() / 2;
		std::nth_element(p_vec.begin(), p_vec.begin() + n, p_vec.end());
		if (p_vec.size() % 2 == 0)
		{
			return (p_vec[n - 1] + p_vec[n]) / 2.0;
		}
		else
		{
			return p_vec[n];
		}
	}
	double mean(const std::vector<double> &p_vec, const std::vector<double> &p_weights)
	{
		if (p_vec.empty())
		{
			throwError("Vector is empty");
		}
		if (!p_weights.empty() && p_vec.size() != p_weights.size())
		{
			throwError("Vectors and weights must have the same size");
		}

		if (p_weights.empty())
		{
			// Calculate the mean of p_vec without weights
			return std::accumulate(p_vec.begin(), p_vec.end(), 0.0) / p_vec.size();
		}
		else
		{
			// Calculate the weighted mean
			double weighted_sum = 0.0;
			double weight_sum = 0.0;
			for (size_t i = 0; i < p_vec.size(); ++i)
			{
				weighted_sum += p_vec[i] * p_weights[i];
				weight_sum += p_weights[i];
			}
			return weighted_sum / weight_sum;
		}
	}

	double std(const std::vector<double> &p_vec, const std::vector<double> &p_weights)
	{
		if (p_vec.size() < 2)
		{
			throwError("Vector must have at least two elements");
		}

		double meanV = mean(p_vec, p_weights);

		if (p_weights.empty())
		{
			// Unweighted case
			double sum_sq_diff = 0.0;
			for (const auto &value : p_vec)
			{
				double diff = value - meanV;
				sum_sq_diff += diff * diff;
			}
			return std::sqrt(sum_sq_diff / (p_vec.size() - 1));
		}
		else
		{
			// Weighted case
			if (p_vec.size() != p_weights.size())
			{
				throwError("Vectors and weights must have the same size");
			}

			double sum_sq_diff = 0.0;
			double sum_weights = 0.0;
			double sum_weights_squared = 0.0;

			for (size_t i = 0; i < p_vec.size(); ++i)
			{
				double diff = p_vec[i] - meanV;
				sum_sq_diff += p_weights[i] * diff * diff;
				sum_weights += p_weights[i];
				sum_weights_squared += p_weights[i] * p_weights[i];
			}

			return std::sqrt(sum_sq_diff * sum_weights / (sum_weights * sum_weights - sum_weights_squared));
		}
	}

	std::string visualisationURL = "https://icecube.github.io/OMSim/md_extra_doc_2_technicalities.html#autotoc_md30";
}