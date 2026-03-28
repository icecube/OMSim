/**
 * @file ROOTHitManager.hh
 * @brief ROOT-based output manager for WavePID photon hit data.
 */
#pragma once

#include "TFile.h"
#include "TTree.h"
#include "G4Types.hh"
#include "G4ThreeVector.hh"
#include "TString.h"

/**
 * @struct PhotonInfoROOT
 * @brief Structure for storing photon hit data in ROOT format.
 */
struct PhotonInfoROOT {
    Int_t    eventID;
    Double_t hitTime;
    Double_t flightTime;
    Double_t entryTime;
    Double_t pathLenght;
    Double_t generationDetectionDistance;
    Double_t energy;
    Int_t    PMTnr;
    Double_t dir_x;
    Double_t dir_y;
    Double_t dir_z;
    Double_t localPos_x;
    Double_t localPos_y;
    Double_t localPos_z;
    Double_t globalPos_x;
    Double_t globalPos_y;
    Double_t globalPos_z;
    Double_t deltaPos_x;
    Double_t deltaPos_y;
    Double_t deltaPos_z;
    Double_t wavelength;
    Int_t    parentID;
    TString  parentType;
    TString  photonOrigin;
    TString  parentProcess;
    Int_t    nPhotoelectrons;
};

/**
 * @class ROOTHitManager
 * @brief Manages ROOT file output for photon hit data.
 */
class ROOTHitManager {
public:
    /**
     * @brief Constructor - opens ROOT file and creates TTree.
     * @param filename Path to the output ROOT file.
     */
    ROOTHitManager(const std::string& filename);

    /**
     * @brief Destructor - writes and closes ROOT file.
     */
    ~ROOTHitManager();

    /**
     * @brief Fills the TTree with photon hit information.
     * @param info PhotonInfoROOT structure containing hit data.
     */
    void FillPhotonInfo(const PhotonInfoROOT& info);

private:
    TFile* m_file;
    TTree* m_tree;
    PhotonInfoROOT m_photonInfo;
};
