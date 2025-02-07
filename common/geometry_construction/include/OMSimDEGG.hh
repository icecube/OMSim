/**
 * @file OMSimDEGG.hh
 * @brief Construction of the DEGG class. From DOUMEKI
 * @ingroup common
 */

#pragma once

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"
class DEggHarness;

/**
 * @class DEGG
 * @brief Construction of the DEGG detector geometry.
 *
 * The DEGG class is responsible for constructing the D-Egg detector geometry. It creates the pressure
 * vessel, the inner volume, and places the PMTs inside the logical volume of the gel. The class is derived
 * from the `OMSimDetectorComponent` base class.
 * @ingroup common
 */
class DEGG : public OMSimOpticalModule
{
public:
    DEGG(G4bool p_placeHarness = true);
    ~DEGG(){};

    void construction();
    double getPressureVesselWeight() { return 18.0 * kg; };
    int getNumberOfPMTs() { return 2; };

    G4String getName()
    {
        std::stringstream ss;
        ss << "/DEGG/" << m_index;
        return ss.str();
    }

private:
    DEggHarness *m_harness;
    G4bool m_placeHarness = true;
    G4SubtractionSolid *substractHarnessPCA(G4VSolid *pSolid);

    void appendPMTs();
    G4VSolid* placePMTs(G4LogicalVolume *p_innerVolume, G4VSolid* p_gelLayer);
    void appendInternalComponentsFromCAD();

    std::stringstream m_converter;
    
    G4VSolid *createEggSolid(G4int p_segments_1,
                             G4double pSphereRmax,
                             G4double pSpheredTheta,
                             G4double pSphereTransformZ,
                             G4double pTorus1R,
                             G4double pCenterOfTorus1R,
                             G4int pSegments_2,
                             G4double pTorus2R,
                             G4double pCenterOfTorus2R,
                             G4double pCenterOfTorus2_z,
                             G4double pTorus2_Zmin,
                             G4double pTorus2_Zmax,
                             G4double pTorus2_Z0,
                             G4double pTorus1TransformZ);
};

