/**
 * @file
 * @brief Modified version of the original Geant4 RadioactiveDecay class to stop decay chain early. 
 * @ingroup radioactive
 */

#pragma once

#include "OMSimG4VRadioactiveDecay.hh"

#include <vector>
#include <map>
#include <CLHEP/Units/SystemOfUnits.h>

#include "G4ios.hh"
#include "globals.hh"
#include "G4VRestDiscreteProcess.hh"
#include "OMSimG4ParticleChangeForRadDecay.hh"

#include "G4NucleusLimits.hh"
#include "G4RadioactiveDecayRatesToDaughter.hh"
#include "G4RadioactiveDecayChainsFromParent.hh"
#include "G4RadioactivityTable.hh"
#include "G4ThreeVector.hh"
#include "G4Threading.hh"

class G4Fragment;
class G4RadioactivationMessenger;

typedef std::vector<G4RadioactiveDecayChainsFromParent> G4RadioactiveDecayParentChainTable;
typedef std::vector<G4RadioactiveDecayRatesToDaughter> G4RadioactiveDecayRates;
typedef std::map<G4String, G4DecayTable*> DecayTableMap;

class G4RadioactiveDecay : public G4VRadioactiveDecay
{
  public: // with description

    G4RadioactiveDecay(const G4String& processName="RadioactiveDecay",
                      const G4double timeThresholdForRadioactiveDecays=-1.0);
    ~G4RadioactiveDecay() override;

    G4VParticleChange* DecayIt(const G4Track& theTrack,
                               const G4Step&  theStep) override;

    void ProcessDescription(std::ostream& outFile) const override;

    // Set the decay biasing scheme using the data in "filename"
    void SetDecayBias(const G4String& filename);

    // Set the half-life threshold for isomer production
    void SetHLThreshold(G4double hl) {halflifethreshold = hl;}

    void SetSourceTimeProfile(const G4String& filename);
    // Set source exposure function using histograms in "filename"

    G4bool IsRateTableReady(const G4ParticleDefinition &);
    // Returns true if the coefficient and decay time table for all the
    // descendants of the specified isotope are ready.
    // used in VR decay mode only

    void CalculateChainsFromParent(const G4ParticleDefinition&);
    // Calculates the coefficient and decay time table for all the descendents
    // of the specified isotope.  Adds the calculated table to the private data
    // member "theParentChainTable".
    // used in VR decay mode only 

    void GetChainsFromParent(const G4ParticleDefinition&);
    // Used to retrieve the coefficient and decay time table for all the
    // descendants of the specified isotope from "theParentChainTable"
    // and place it in "chainsFromParent".
    // used in VR decay mode only 

    void SetDecayRate(G4int,G4int,G4double, G4int, std::vector<G4double>&,
                      std::vector<G4double>&);
    // Sets "theDecayRate" with data supplied in the arguements.
    // used in VR decay mode only 

    std::vector<G4RadioactivityTable*>& GetTheRadioactivityTables()
       {return theRadioactivityTables;}
    // Return vector of G4Radioactivity map - should be used in VR mode only


    // Controls whether G4RadioactiveDecay runs in analogue mode or
    // variance reduction mode.  SetBRBias, SetSplitNuclei and
    // SetSourceTimeProfile all turn off analogue mode and use VR mode
    inline void SetAnalogueMonteCarlo (G4bool r) {
      AnalogueMC = r;
    }

    // Returns true if the simulation is an analogue Monte Carlo, and false if
    // any of the biassing schemes have been selected.
    inline G4bool IsAnalogueMonteCarlo () {return AnalogueMC;}

     // Sets whether branching ration bias scheme applies.
    inline void SetBRBias(G4bool r) {
      BRBias = r;
      AnalogueMC = false;
    }

    // Sets the number of times a nucleus will decay when biased
    inline void SetSplitNuclei(G4int r) {
      NSplit = r;
      AnalogueMC = false;
    }

    //  Returns the nuclear splitting number
    inline G4int GetSplitNuclei () {return NSplit;}

    G4RadioactiveDecay(const G4RadioactiveDecay& right) = delete;
    G4RadioactiveDecay& operator=(const G4RadioactiveDecay& right) = delete;

  protected:

    G4double ConvolveSourceTimeProfile(const G4double, const G4double);
    G4double GetDecayTime();
    G4int GetDecayTimeBin(const G4double aDecayTime);

    G4double GetMeanLifeTime(const G4Track& theTrack,
                             G4ForceCondition* condition) override;

    //Add gamma,Xray,conversion,and auger electrons for bias mode
    void AddDeexcitationSpectrumForBiasMode(G4ParticleDefinition* apartDef,
                                            G4double weight,
                                            G4double currenTime,
                                            std::vector<double>& weights_v,
                                            std::vector<double>& times_v,
                                            std::vector<G4DynamicParticle*>& secondaries_v);

  private:

    G4RadioactivationMessenger* theRadioactivationMessenger;
    G4bool AnalogueMC;
    G4bool BRBias;
    G4int NSplit;

    G4double halflifethreshold;

    G4int NSourceBin;
    G4double SBin[100];
    G4double SProfile[100];
    G4int NDecayBin;
    G4double DBin[100];
    G4double DProfile[100];

    G4RadioactiveDecayRatesToDaughter ratesToDaughter;
    G4RadioactiveDecayRates theDecayRateVector;
    G4RadioactiveDecayChainsFromParent chainsFromParent;
    G4RadioactiveDecayParentChainTable theParentChainTable;

    // for the radioactivity tables
    std::vector<G4RadioactivityTable*> theRadioactivityTables;
    G4int decayWindows[100];
};
