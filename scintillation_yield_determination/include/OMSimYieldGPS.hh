
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

  void setEmitterVolume(abcDetectorComponent *pEmitterVolume) { mEmitterVolume = pEmitterVolume; };
  void setProductionRadius(G4double pProductionRadius);

  void configureGammaEmitter(G4double pEnergy, G4String pVolumeName);
  
private:

  abcDetectorComponent *mEmitterVolume;
  G4double mProductionRadius;

  OMSimYieldGPS() = default;
  ~OMSimYieldGPS() = default;
  OMSimYieldGPS(const OMSimYieldGPS &) = delete;
  OMSimYieldGPS &operator=(const OMSimYieldGPS &) = delete;
};
#endif
