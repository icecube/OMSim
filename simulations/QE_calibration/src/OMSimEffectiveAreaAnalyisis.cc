#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimTools.hh"
/**
 * @brief Calculates the effective area based on the number of hits and beam properties.
 * @param pHits The number of hits.
 * @return Returns a structure with the effective area and its uncertainty.
 */
effectiveAreaResult OMSimEffectiveAreaAnalyisis::calculateEffectiveArea(double pHits)
{
	OMSimCommandArgsTable &lArgs = OMSimCommandArgsTable::getInstance();
	G4double lBeamRadius = lArgs.get<G4double>("radius") / 10.; // in cm
	G4double lNumberPhotons = lArgs.get<int>("numevents");
	G4double lBeamArea = CLHEP::pi * lBeamRadius * lBeamRadius;
	G4double lEA = pHits * lBeamArea / lNumberPhotons;
	G4double lEAError = sqrt(pHits) * lBeamArea / lNumberPhotons;
	return {lEA, lEAError};
}

void OMSimEffectiveAreaAnalyisis::writeHitPositionHistogram(double x, double y)
{
	log_trace("Writing histogram");
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
	HitStats lHits = lHitManager.getMergedHitsOfModule();

	std::vector<double> lR(lHits.eventId.size());

	for (int i = 0; i < (int)lHits.eventId.size(); i++)
	{
		double lX = lHits.localPosition.at(i).x() / mm;
		double lY = lHits.localPosition.at(i).y() / mm;
		lR.push_back(std::sqrt(lX * lX + lY * lY));
	}

	auto lRange = Tools::arange(0, 42.25, 0.25);
	auto [lCounts, lEdges] = Tools::histogram(lR, lRange);

	std::fstream lDatafile;
	lDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);

	lDatafile << x << "\t" << y << "\t";
	for (const auto &count : lCounts)
	{
		lDatafile << count << "\t";
	}
	lDatafile << "\n";
	lDatafile.close();
	log_trace("Finished writing histogram");
}
