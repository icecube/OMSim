/**
 * @file OMSimDEGG.hh
 * @brief Construction of the DEGG class.
 * @author Geometry from DOUMEKI parsed by Berit Schlüter
 * @ingroup common
 */

#ifndef DEGG_h
#define DEGG_h 1

#include "OMSimPMTConstruction.hh"
#include "OMSimOpticalModule.hh"
class DEggHarness;

/**
 * @class DEGG
 * @brief Construction of the DEGG detector geometry.
 *
 * The DEGG class is responsible for constructing the D-Egg detector geometry. It creates the pressure
 * vessel, the inner volume, and places the PMTs inside the logical volume of the gel. The class is derived
 * from the `abcDetectorComponent` base class.
 * @ingroup common
 */
class DEGG : public OpticalModule
{
public:
    DEGG(InputDataManager *pData, G4bool pPlaceHarness = true);
    ~DEGG(){};

    /**
     * @brief Construction of the whole DEGG. If you want to change any component, you have to change it at the specific function.
     */
    void construction();

    double getPressureVesselWeight() { return 18.0*kg; };

    int getNumberOfPMTs() { return 2; };

private:
    DEggHarness *mHarness;

    /**
     * @brief Append the PMTs in the DEGG components vector.
     */
    void appendPMTs();

    /**
     * @brief Appends internal components loaded from CAD files to the DEGG components vector.
     */
    void appendInternalComponentsFromCAD();

    /**
     * @brief Appends the pressure vessel from a CAD file to the DEGG geometry.
     */
    void appendPressureVesselFromCAD();

    /**
     * @brief Creates the solid shape for the DEGG pressure vessel.
     * @param pSegments_1 Number of segments for the G4Sphere representing the outer glass.
     * @param pSphereRmax Outer radius of the G4Sphere representing the outer glass.
     * @param pSpheredTheta Delta theta angle of the G4Sphere segment.
     * @param pSphereTransformZ Shift of the G4Sphere in the z-direction.
     * @param pTorus1R Radius of the small spindle torus sphere.
     * @param pCenterOfTorus1R Distance from the center of torus 1 to the z-axis.
     * @param pSegments_2 Number of segments for the large spindle torus sphere.
     * @param pTorus2R Radius of the large spindle torus sphere.
     * @param pCenterOfTorus2R Distance from the center of torus 2 to the z-axis (signed).
     * @param pCenterOfTorus2_z Distance from the center of torus 2 to the z-axis (signed).
     * @param pTorus2_Zmin Minimum z shift from z=0 in the positive z direction.
     * @param pTorus2_Zmax Maximum z shift from z=0 in the positive z direction.
     * @param pTorus2_Z0 G4double.
     * @param pTorus1TransformZ G4double.
     * @return The outer or inner shape of the glass vessel as a G4VSolid.
     */
    G4VSolid *createEggSolid(G4int pSegments_1,
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


#endif