
#ifndef OMSimYieldGPS_h
#define OMSimYieldGPS_h 1
#include "OMSimOpticalModule.hh"
#include <globals.hh>

class OMSimYieldGPS
{
public:

  static OMSimYieldGPS &getInstance()
  {
    static OMSimYieldGPS instance;
    return instance;
  }

  void setEmitterVolume(OMSimDetectorComponent *pEmitterVolume) { mEmitterVolume = pEmitterVolume; };
  void setProductionRadius(G4double pProductionRadius) {mProductionRadius = pProductionRadius;};
  void limitThetaEmission(G4double pThetaMin, G4double pThetaMax);
  void configureGammaEmitter(G4double pEnergy, G4String pVolumeName);
  void configureAm241Emitter(G4String pVolumeName);
  void configureAm241EmitterForActivity(G4String pVolumeName);
  
private:

  OMSimDetectorComponent *mEmitterVolume;
  G4double mProductionRadius;

  OMSimYieldGPS() = default;
  ~OMSimYieldGPS() = default;
  OMSimYieldGPS(const OMSimYieldGPS &) = delete;
  OMSimYieldGPS &operator=(const OMSimYieldGPS &) = delete;
};
#endif
