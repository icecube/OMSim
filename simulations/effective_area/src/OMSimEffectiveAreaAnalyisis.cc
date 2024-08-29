#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"




/**
 * @brief Calculates the effective area based on the number of hits and beam properties.
 * @param p_hits The number of hits.
 * @return Returns a structure with the effective area and its uncertainty.
 */
effectiveAreaResult OMSimEffectiveAreaAnalyisis::calculateEffectiveArea(double p_hits)
{
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	G4double beamRadius = args.get<G4double>("radius")/10.; //in cm
	G4double numberPhotons = args.get<int>("numevents");
	G4double beamArea = CLHEP::pi * beamRadius * beamRadius;
	G4double effectiveArea = p_hits * beamArea / numberPhotons;
	G4double effectiveAreaError = sqrt(p_hits) * beamArea / numberPhotons;
	return { effectiveArea, effectiveAreaError };
}


