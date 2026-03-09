/**
 * @file ROOTHitManager.cc
 * @brief Implementation of ROOT-based output manager for WavePID.
 */
#include "ROOTHitManager.hh"
#include "OMSimLogger.hh"
#include <stdexcept>

ROOTHitManager::ROOTHitManager(const std::string& filename)
    : m_file(nullptr), m_tree(nullptr)
{
    log_trace("Creating ROOT file: {}", filename);

    m_file = TFile::Open(filename.c_str(), "RECREATE");
    if (!m_file || m_file->IsZombie()) {
        log_error("Failed to create ROOT file: {}", filename);
        throw std::runtime_error("Failed to create ROOT file: " + filename);
    }

    m_tree = new TTree("PhotonHits", "WavePID Photon Hit Information");

    // Create branches for all photon hit information
    m_tree->Branch("eventID", &m_photonInfo.eventID, "eventID/I");
    m_tree->Branch("hitTime", &m_photonInfo.hitTime, "hitTime/D");
    m_tree->Branch("entryTime", &m_photonInfo.entryTime, "entryTime/D");
    m_tree->Branch("flightTime", &m_photonInfo.flightTime, "flightTime/D");
    m_tree->Branch("pathLenght", &m_photonInfo.pathLenght, "pathLenght/D");
    m_tree->Branch("generationDetectionDistance", &m_photonInfo.generationDetectionDistance, "generationDetectionDistance/D");
    m_tree->Branch("energy", &m_photonInfo.energy, "energy/D");
    m_tree->Branch("PMTnr", &m_photonInfo.PMTnr, "PMTnr/I");
    m_tree->Branch("dir_x", &m_photonInfo.dir_x, "dir_x/D");
    m_tree->Branch("dir_y", &m_photonInfo.dir_y, "dir_y/D");
    m_tree->Branch("dir_z", &m_photonInfo.dir_z, "dir_z/D");
    m_tree->Branch("localPos_x", &m_photonInfo.localPos_x, "localPos_x/D");
    m_tree->Branch("localPos_y", &m_photonInfo.localPos_y, "localPos_y/D");
    m_tree->Branch("localPos_z", &m_photonInfo.localPos_z, "localPos_z/D");
    m_tree->Branch("globalPos_x", &m_photonInfo.globalPos_x, "globalPos_x/D");
    m_tree->Branch("globalPos_y", &m_photonInfo.globalPos_y, "globalPos_y/D");
    m_tree->Branch("globalPos_z", &m_photonInfo.globalPos_z, "globalPos_z/D");
    m_tree->Branch("deltaPos_x", &m_photonInfo.deltaPos_x, "deltaPos_x/D");
    m_tree->Branch("deltaPos_y", &m_photonInfo.deltaPos_y, "deltaPos_y/D");
    m_tree->Branch("deltaPos_z", &m_photonInfo.deltaPos_z, "deltaPos_z/D");
    m_tree->Branch("wavelength", &m_photonInfo.wavelength, "wavelength/D");
    m_tree->Branch("parentID", &m_photonInfo.parentID, "parentID/I");
    m_tree->Branch("parentType", &m_photonInfo.parentType);
    m_tree->Branch("photonOrigin", &m_photonInfo.photonOrigin);
    m_tree->Branch("parentProcess", &m_photonInfo.parentProcess);
    m_tree->Branch("nPhotoelectrons", &m_photonInfo.nPhotoelectrons, "nPhotoelectrons/I");

    log_trace("ROOT tree created with all branches");
}

ROOTHitManager::~ROOTHitManager()
{
    if (m_file && !m_file->IsZombie()) {
        log_trace("Writing and closing ROOT file");
        m_file->cd();
        m_tree->Write();
        m_file->Close();
    }
    delete m_file;
    m_file = nullptr;
    m_tree = nullptr;  // Tree is owned by file, don't delete separately
    log_trace("ROOT file closed");
}

void ROOTHitManager::FillPhotonInfo(const PhotonInfoROOT& info)
{
    m_photonInfo = info;
    m_tree->Fill();
}
