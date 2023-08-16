#include "OMSimDecaysAnalysis.hh"
#include "OMSimCommandArgsTable.hh"
#include <numeric>

void OMSimDecaysAnalysis::writeMultiplicity(const std::vector<int> &pMultiplicity)
{
	for (const auto &value : pMultiplicity)
	{
		mDatafile << value << "\t";
	}
	mDatafile << G4endl;
}
