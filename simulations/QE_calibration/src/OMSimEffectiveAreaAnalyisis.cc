#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"




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


