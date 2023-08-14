#include "OMSimAnalysisManager.hh"
#include "OMSimCommandArgsTable.hh"

/**
 * @brief Counts the number of hits for each PMT
 * @return A vector containing the counts of hits for each PMT.
 */

std::vector<double> OMSimAnalysisManager::countHits()
{
	G4int lDOMIdx = OMSimCommandArgsTable::getInstance().get<G4int>("detector_type");

	int lNumPMTs;
	if (lDOMIdx == 1)
	{
		lNumPMTs = 1;
	} // single PMT
	else if (lDOMIdx == 2)
	{
		lNumPMTs = 24;
	} // mDOM
	else if (lDOMIdx == 3)
	{
		lNumPMTs = 1;
	} // PDOM
	else if (lDOMIdx == 4)
	{
		lNumPMTs = 16;
	} // LOM16
	else if (lDOMIdx == 5)
	{
		lNumPMTs = 18;
	} // LOM18
	else if (lDOMIdx == 6)
	{
		lNumPMTs = 2;
	} // DEGG
	else
	{
		lNumPMTs = 99;
	} // custom

	std::vector<double> lHits(lNumPMTs + 2, 0.0);

	for (int i = 0; i < (int)mHits.PMT_hit.size(); i++)
	{
		lHits[mHits.PMT_hit.at(i)] += 1; // mHits.PMT_response.at(i).DetectionProbability;
		lHits[lNumPMTs + 1] += 1;
	}

	return lHits;
}

/**
 * @brief Writes a header to the output file.
 */
void OMSimAnalysisManager::writeHeader()
{
	mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);
	mDatafile << "# Phi(deg)"
			  << "\t"
			  << "Theta(deg)"
			  << "\t"
			  << "Radius(mm)"
			  << "\t"
			  << "HitsPMTs"
			  << "\t"
			  << "Total"
			  << "\t" << G4endl;
	mDatafile.close();
}

/**
 * @brief Writes a scan result to the output file.
 * @param pPhi The phi angle used in the scan to be written to the output file.
 * @param pTheta The phi angle used in the scan to be written to the output file.
 */
void OMSimAnalysisManager::writeScan(G4double pPhi, G4double pTheta)
{
	std::vector<double> lHits = countHits();
	mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);
	mDatafile << pPhi << "\t" << pTheta << "\t";
	for (const auto &hit : lHits)
	{
		mDatafile << hit << "\t";
	}
	mDatafile << G4endl;
	mDatafile.close();
}

/**
 * @brief Empties the HitStats struct.
 */
void OMSimAnalysisManager::reset()
{
	mHits = HitStats();
}
