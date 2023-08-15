#include "OMSimEffectiveAreaAnalyisis.hh"

/**
 * @brief Writes a header to the output file.
 */
void OMSimEffectiveAreaAnalyisis::writeHeader()
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
void OMSimEffectiveAreaAnalyisis::writeScan(G4double pPhi, G4double pTheta)
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

