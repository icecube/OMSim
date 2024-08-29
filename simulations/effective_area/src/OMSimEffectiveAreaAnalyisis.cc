#include "OMSimEffectiveAreaAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"




/**
 * @brief Calculates the effective area based on the number of hits and beam properties.
 * @param p_weightTotal The number of hits weighted.
 * @param p_totalCount The number of hits
 * @return Returns a structure with the effective area and its uncertainty.
 */
effectiveAreaResult OMSimEffectiveAreaAnalyisis::calculateEffectiveArea(double p_weightTotal, double p_totalCount)
{
	OMSimCommandArgsTable &args = OMSimCommandArgsTable::getInstance();
	G4double beamRadius = args.get<G4double>("radius")/10.; //in cm
	G4double numberPhotons = args.get<int>("numevents");
	G4double beamArea = CLHEP::pi * beamRadius * beamRadius;
	G4double effectiveArea = p_weightTotal * beamArea / numberPhotons;
	G4double effectiveAreaError = effectiveArea / sqrt(p_totalCount);
	return { effectiveArea, effectiveAreaError };
}


