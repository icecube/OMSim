#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"


/**
 * @brief Writes the header line to the output file.
 */
void OMSimEffectiveAreaAnalyisis::writeHeader()
{
	mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);
	mDatafile << "# Phi(deg)"
			  << "\t"
			  << "Theta(deg)"
			  << "\t"
			  << "hits[1perPMT]"
			  << "\t"
			  << "total_hits"		
			  << "\t"
			  << "EA_Total(cm^2)"
			  << "\t"
			  << "EA_Total_error(cm^2)"
			  << "\t" << G4endl;
	mDatafile.close();
}


/**
 * @brief Calculates the effective area based on the number of hits and beam properties.
 * @param pHits The number of hits.
 * @return Returns a structure with the effective area and its uncertainty.
 */
effectiveAreaResult OMSimEffectiveAreaAnalyisis::calculateEffectiveArea(double pHits)
{
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	G4double lBeamRadius = lArgs.get<G4double>("radius")/10.; //in cm
	G4double lNumberPhotons = lArgs.get<int>("numevents");
	G4double lBeamArea = CLHEP::pi * lBeamRadius * lBeamRadius;
	G4double lEA = pHits * lBeamArea / lNumberPhotons;
	G4double lEAError = sqrt(pHits) * lBeamArea / lNumberPhotons;
	return { lEA, lEAError };
}

/**
 * @brief Writes a scan result to the output file.
 * @param pPhi The phi angle used in the scan to be written to the output file.
 * @param pTheta The phi angle used in the scan to be written to the output file.
 */
void OMSimEffectiveAreaAnalyisis::writeScan(G4double pPhi, G4double pTheta)
{

	std::vector<double> lHits_0 = OMSimHitManager::getInstance().countHits(0);
	std::vector<double> lHits_1 = OMSimHitManager::getInstance().countHits(1);
	std::vector<double> lHits_2 = OMSimHitManager::getInstance().countHits(2);

	mDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);
	mDatafile << pPhi << "\t" << pTheta << "\t";

	mDatafile << lHits_0.at(1) << "\t";
	mDatafile << lHits_1.at(1) << "\t";
	mDatafile << lHits_2.at(1) << "\t";


	//effectiveAreaResult lEffectiveArea = calculateEffectiveArea(lTotalHits);
	//mDatafile << lEffectiveArea.EA << "\t" << lEffectiveArea.EAError << "\t";
	mDatafile << G4endl;
	mDatafile.close();
}
