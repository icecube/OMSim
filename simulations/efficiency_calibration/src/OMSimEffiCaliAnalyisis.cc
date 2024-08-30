#include "OMSimEffiCaliAnalyisis.hh"
#include "OMSimCommandArgsTable.hh"
#include "OMSimHitManager.hh"
#include "OMSimTools.hh"


void OMSimEffiCaliAnalyisis::writeHitPositionHistogram(double x, double y)
{
	log_trace("Writing histogram");
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();
	HitStats hits = hitManager.getMergedHitsOfModule();

	std::vector<double> r;
	std::vector<double> detectionProbability;

	for (int i = 0; i < (int)hits.eventId.size(); i++)
	{
		double X = hits.localPosition.at(i).x() / mm;
		double Y = hits.localPosition.at(i).y() / mm;
		r.push_back(std::sqrt(X * X + Y * Y));
		detectionProbability.push_back(hits.PMTresponse.at(i).detectionProbability);
	}

	auto lRange = Tools::arange(0, 41.25, 0.25);
	auto [lCounts, lEdges] = Tools::histogram(r, lRange);
	auto [lCountsWeighted, lEdgesWeighted] = Tools::histogram(r, lRange, std::nullopt, detectionProbability);

	std::fstream dataFile;
	dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);

	dataFile << x << "\t" << y << "\t";
	for (const auto &count : lCounts)
	{ 
		dataFile << count << "\t";
	}
	for (const auto &count : lCountsWeighted)
	{ 
		dataFile << count << "\t";
	}
	dataFile << "\n";
	dataFile.close();
	log_trace("Finished writing histogram");
}



void OMSimEffiCaliAnalyisis::writePositionStatistics(double x, double wavelength)
{
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();
	HitStats hits = hitManager.getMergedHitsOfModule();

	std::vector<double> r;

	for (int i = 0; i < (int)hits.eventId.size(); i++)
	{
		double X = hits.localPosition.at(i).x() / mm;
		double Y = hits.localPosition.at(i).y() / mm;
		r.push_back(std::sqrt(X * X + Y * Y));
	}

	std::fstream dataFile;
	dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);

	dataFile << x << "\t" << wavelength << "\t";
	if (r.size()>2){
		dataFile << Tools::mean(r) << "\t";
		dataFile << Tools::median(r) << "\t";
		dataFile << Tools::std(r) << "\t";
		dataFile << r.size()<< "\t";
	}
	else{
		dataFile << "0\t0\t0\t0\t";
	}
	dataFile << "\n";
	dataFile.close();
}


void OMSimEffiCaliAnalyisis::writePositionPulseStatistics(double x, double y, double wavelength)
{
	OMSimHitManager &hitManager = OMSimHitManager::getInstance();
	HitStats hits = hitManager.getMergedHitsOfModule();

	std::vector<double> gain, transit_time, weigth;

	for (int i = 0; i < (int)hits.eventId.size(); i++)
	{
		gain.push_back(hits.PMTresponse.at(i).PE);
		weigth.push_back(1/(hits.PMTresponse.at(i).detectionProbability*hits.PMTresponse.at(i).detectionProbability));
		transit_time.push_back(hits.PMTresponse.at(i).transitTime);
	}

	std::fstream dataFile;
	dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);

	dataFile << x << "\t" << y << "\t" <<  wavelength << "\t";
	if (transit_time.size()>2){
		dataFile << Tools::mean(gain, weigth) << "\t";
		dataFile << Tools::std(gain, weigth) << "\t";
		dataFile << Tools::mean(transit_time, weigth)  << "\t";
		dataFile << Tools::std(transit_time, weigth)  << "\t";
		dataFile << transit_time.size()<< "\t";
	}
	else{
		dataFile << "0\t0\t0\t0\t0\t";
	}
	dataFile << "\n";
	dataFile.close();
}


void OMSimEffiCaliAnalyisis::writeHits(double p_wavelength)
{
    std::vector<double> hits = OMSimHitManager::getInstance().countMergedHits();

    std::fstream dataFile;
    dataFile.open(m_outputFileName.c_str(), std::ios::out | std::ios::app);

    dataFile << p_wavelength << "\t";
    dataFile << hits.at(0) << "\t";

	//weighted
    hits = OMSimHitManager::getInstance().countMergedHits(0, true); 
	dataFile << hits.at(0) << "\t";

    dataFile << G4endl;
    dataFile.close();
}