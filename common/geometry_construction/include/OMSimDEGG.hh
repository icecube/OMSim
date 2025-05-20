/**
 * @file OMSimDEGG.hh
 * @brief Construction of the DEGG class.
 * @ingroup common
 */

#pragma once

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"
class DEggHarness;

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
    

    /**
     * @brief Creates the egg-shaped pressure vessel solid
     * @details Constructs the D-Egg vessel shape using spheres and tori. The vessel 
     *          consists of two main parts:
     *          1. Upper/lower spherical sections
     *          2. Connecting torus sections forming the waist
     * 
     * @param p_segments1 Number of segments for the small spindle torus sphere
     * @param p_sphereRmax Maximum radius of the spherical sections
     * @param p_spheredTheta Opening angle of spherical sections
     * @param p_sphereTransformZ Z-axis translation of spherical sections
     * @param p_torus1R Radius of the small spindle torus sphere
     * @param p_centreOfTorus1R Distance from center of torus 1 to z-axis
     * @param p_segments2 Number of segments for the large spindle torus sphere
     * @param p_torus2R Radius of the large spindle torus sphere
     * @param p_centreOfTorus2R Distance from center of torus 2 to z-axis 
     * @param p_centreOfTorus2z Z-position of torus 2 center
     * @param p_torus2Zmin Minimum z-position of torus 2
     * @param p_torus2Zmax Maximum z-position of torus 2
     * @param p_torus2Z0 Z-offset for torus 2
     * @param p_torus1TransformZ Z-translation of torus 1
     * 
     * @return G4VSolid* The complete egg-shaped vessel solid
     */
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

