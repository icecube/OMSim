#include "OMSimEffiCaliAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimTools.hh"


void OMSimEffiCaliAnalyisis::writeHitPositionHistogram(double x, double y)
{
	log_trace("Writing histogram");
	OMSimHitManager &lHitManager = OMSimHitManager::getInstance();
	HitStats lHits = lHitManager.getMergedHitsOfModule();

	std::vector<double> lR;
	std::vector<double> lDetectionProbability;

	for (int i = 0; i < (int)lHits.eventId.size(); i++)
	{
		double lX = lHits.localPosition.at(i).x() / mm;
		double lY = lHits.localPosition.at(i).y() / mm;
		lR.push_back(std::sqrt(lX * lX + lY * lY));
		lDetectionProbability.push_back(lHits.PMTresponse.at(i).detectionProbability);
	}

	auto lRange = Tools::arange(0, 41.25, 0.25);
	auto [lCounts, lEdges] = Tools::histogram(lR, lRange);
	auto [lCountsWeighted, lEdgesWeighted] = Tools::histogram(lR, lRange, std::nullopt, lDetectionProbability);

	std::fstream lDatafile;
	lDatafile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);

	lDatafile << x << "\t" << y << "\t";
	for (const auto &count : lCounts)
	{ 
		lDatafile << count << "\t";
	}
	for (const auto &count : lCountsWeighted)
	{ 
		lDatafile << count << "\t";
	}
	lDatafile << "\n";
	lDatafile.close();
	log_trace("Finished writing histogram");
}


void OMSimEffiCaliAnalyisis::writeHits(double pWavelength)
{
    std::vector<double> lHits = OMSimHitManager::getInstance().countMergedHits();

    std::fstream lDataFile;
    lDataFile.open(mOutputFileName.c_str(), std::ios::out | std::ios::app);

    lDataFile << pWavelength << "\t";
    lDataFile << lHits.at(0) << "\t";

	//weighted
    lHits = OMSimHitManager::getInstance().countMergedHits(0, true); 
	lDataFile << lHits.at(0) << "\t";

    lDataFile << G4endl;
    lDataFile.close();
}