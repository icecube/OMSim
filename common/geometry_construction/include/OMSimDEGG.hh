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
class DEGG : public OMSimOpticalModule
{
public:
    DEGG(InputDataManager *pData, G4bool pPlaceHarness = true);
    ~DEGG(){};

    void construction();
    double getPressureVesselWeight() { return 18.0 * kg; };
    int getNumberOfPMTs() { return 2; };

    G4String getName()
    {
        std::stringstream ss;
        ss << "/DEGG/" << mIndex;
        return ss.str();
    }

private:
    DEggHarness *mHarness;


    void appendPMTs();
    void appendInternalComponentsFromCAD();
    void appendPressureVesselFromCAD();

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