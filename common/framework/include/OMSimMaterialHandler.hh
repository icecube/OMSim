/** @file
 *  @brief Definition of OMSimMaterialHandler and the namespaces IceProcessor and ScintillationProcessor
 * @ingroup common
 */

#pragma once

#include <G4NistManager.hh>
#include <G4SystemOfUnits.hh>
#include <G4OpticalSurface.hh>
#include <G4PhysicalConstants.hh>
#include <boost/property_tree/json_parser.hpp>
#include <OMSimCommandArgsTable.hh>

class ParameterTable;
using ProcessorFunction = void(ParameterTable *, const boost::property_tree::ptree &, G4MaterialPropertiesTable *);

/**
 * @class OMSimMaterialHandler
 * @brief Handles the creation and processing of materials from json files.
 *
 * This class is responsible for extracting data from json files, creating new materials,
 * processing optical surfaces, and handling special material types such as IceCube ice
 * and scintillators.
 * @ingroup common
 */
class OMSimMaterialHandler
{
public:
    OMSimMaterialHandler(const G4String &filename);
    ~OMSimMaterialHandler();

    void processMaterial();
    G4OpticalSurface *processSurface();
    void processExtraProperties();
    void processSpecial(ProcessorFunction *processor);
    G4String GetName() { return m_objectName; };

private:
    ParameterTable *m_fileData;
    G4double m_hc_eVnm = h_Planck * c_light / nm;
    G4String m_filePath;
    G4String m_objectName;
    boost::property_tree::ptree m_jsonTree;
    G4Material *m_material;
    G4MaterialPropertiesTable *m_MPT;
    G4OpticalSurface *m_opticalSurface;

    void readJsonFile();
    void createMaterial();
    void processProperties();
    void processConstProperties();
    void findMaterialPropertyTable(G4String p_name);
    void addProperty(const G4String &key, G4double energyUnit, G4double valueUnit, bool invertX, const boost::property_tree::ptree &propertyTree);

    // Helper functions for optical surface
    G4OpticalSurfaceFinish getOpticalSurfaceFinish(const std::string &finish);
    G4OpticalSurfaceModel getOpticalSurfaceModel(const std::string &model);
    G4SurfaceType getSurfaceType(const std::string &type);
    G4State getState(const std::string &type);
};

namespace ScintillationProcessor
{
    void process(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT);
    void extractSpectrum(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT);
    void getLifeTimeTemperatureRange(const boost::property_tree::ptree &p_jsonTree, double &pMinTemp, double &pMaxTemp);
    std::pair<std::vector<G4double>, std::vector<G4double>> extractLifeTimesForTemperature(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4String pTemperature);
    void weightLifeTimesAmplitudes(std::vector<G4double> &pAmp, double pT1, double pT2);
    void extractLifeTimes(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT, G4String pTemperature);
    void extractYield(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT,
                      G4String pTemperature, G4String pYieldPropertyName, G4String pArgKey, G4String pTreeKeyTemperature, G4String pTreeKeyYield);
    void extractYieldAlpha(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT, G4String pTemperature);
    void extractYieldElectron(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT, G4String pTemperature);
}

namespace IceProcessor
{
    void process(ParameterTable *p_dataFile, const boost::property_tree::ptree &p_jsonTree, G4MaterialPropertiesTable *p_MPT);
    G4double spiceTemperature(G4double p_depth);
    G4double spiceAbsorption(G4double p_lambda, const std::vector<G4double> &p_spicea400inv, const std::vector<G4double> &p_spiceDepth);
    G4double spiceRefraction(G4double p_lambda);
    G4double mieScattering(G4double p_lambda, const std::vector<G4double> &p_spicebe400inv);
}