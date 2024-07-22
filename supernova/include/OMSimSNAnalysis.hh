#ifndef OMSimSNAnalysis_h
#define OMSimSNAnalysis_h 1

#include "G4Types.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"

#include "OMSimHitManager.hh"

#include <vector>
#include <fstream>
#include <tuple>

struct SNEventStats
{
	G4double neutrino_time;
	G4double mean_energy;
	G4double neutrino_energy;
	G4double cos_theta;
	G4double primary_energy;
	G4double weight;
};

class OMSimSNAnalysis
{
public:
	OMSimSNAnalysis();
	~OMSimSNAnalysis(){};

	/**
	 * @brief Returns the singleton instance of the OMSimSNAnalysis.
	 * @return A reference to the singleton instance.
	 */
	static OMSimSNAnalysis &getInstance()
	{
		static OMSimSNAnalysis instance;
		return instance;
	}
	
	void analyseEvent();
	void initEventStat();
	void processEvent();
	void writeHeaders();
	void writeInfoFileHeader();
	void writeDataFileHeader();
	void writeInfoFile();
	void writeDataFile();

	G4ThreadLocal static SNEventStats *mEventStat;
	G4ThreadLocal static bool mHeaderWasWritten;

private:
	G4String mOutputSuffix;
};

#endif
