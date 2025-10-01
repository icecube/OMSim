#pragma once

#include "G4Types.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"

#include "OMSimHitManager.hh"

#include <vector>
#include <fstream>
#include <tuple>

struct SNEventStats
{
	G4double neutrinoTime;
	G4double meanEnergy;
	G4double neutrinoEnergy;
	G4double cosTheta;
	G4double primaryEnergy;
	G4double weight;
	G4double vertexDistance;
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
	static OMSimSNAnalysis &getInstance();
	
	
	void analyseEvent();
	void initEventStat();
	void processEvent(G4int);
	void writeHeaders();
	void writeInfoFileHeader();
	void writeDataFileHeader();
	void writeInfoFile(G4int);
	void writeDataFile(G4int);
	void mergeFiles();
	void mergeThreadFiles(G4String);

	G4ThreadLocal static SNEventStats *m_eventStat;
	G4ThreadLocal static bool m_headerWasWritten;

private:
	G4String m_outputSufix;
    static OMSimSNAnalysis* m_instance; 
    static G4Mutex m_mutex;             
};

