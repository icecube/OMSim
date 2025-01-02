
#pragma once

#include "globals.hh"
#include "G4ios.hh"
#include "G4ThreeVector.hh"
#include "G4VParticleChange.hh"

class G4DynamicParticle;

class G4ParticleChangeForRadDecay final : public G4VParticleChange
{
  public:

    G4ParticleChangeForRadDecay();

    ~G4ParticleChangeForRadDecay() override = default;

    G4ParticleChangeForRadDecay(const G4ParticleChangeForRadDecay& right) = delete;
    G4ParticleChangeForRadDecay& operator=(const G4ParticleChangeForRadDecay& right) = delete;

  // --- the following methods are for updating G4Step -----
  // Return the pointer to the G4Step after updating the step information
  // by using final state information of the track given by a physics process
  // !!! No effect for  AlongSteyp

    G4Step* UpdateStepForAtRest(G4Step* Step) final;
    G4Step* UpdateStepForPostStep(G4Step* Step) final;

    void Initialize(const G4Track&) final;
      // Initialize all properties by using G4Track information

    void ProposeGlobalTime(G4double t);
    void ProposeLocalTime(G4double t);
      // Get/Propose the final global/local time
      // NOTE: DO NOT INVOKE both methods in a step
      //       Each method affects both local and global time

    G4double GetGlobalTime(G4double timeDelay = 0.0) const;
    G4double GetLocalTime(G4double timeDelay = 0.0) const;
      // Convert the time delay to the glocbal/local time.
      // Can get  the final global/local time without argument

    const G4ThreeVector* GetPolarization() const;
    void ProposePolarization(G4double Px, G4double Py, G4double Pz);
    void ProposePolarization(const G4ThreeVector& finalPoralization);
    void AddSecondary(G4Track* aTrack);
      // Get/Propose the final Polarization vector

  // --- Dump and debug methods ---

    void DumpInfo() const final;

    G4bool CheckIt(const G4Track&) final;
    

  private:

    G4double theGlobalTime0 = 0.0;
      // The global time at Initial
    G4double theLocalTime0 = 0.0;
      // The local time at Initial

    G4double theTimeChange = 0.0;
      // The change of local time of a given particle

    G4ThreeVector thePolarizationChange;
      // The changed (final) polarization of a given track

    G4bool CheckSecondary(G4Track&);
    
};

// ----------------------
// Inline methods
// ----------------------

inline
void G4ParticleChangeForRadDecay::ProposeGlobalTime(G4double t)
{
  theTimeChange = (t - theGlobalTime0) + theLocalTime0;
}

inline
G4double G4ParticleChangeForRadDecay::GetGlobalTime(G4double timeDelay) const
{
  //  Convert the time delay to the global time.
  return theGlobalTime0 + (theTimeChange - theLocalTime0) + timeDelay;
}

inline
void G4ParticleChangeForRadDecay::ProposeLocalTime(G4double t)
{
  theTimeChange = t;
}

inline
G4double G4ParticleChangeForRadDecay::GetLocalTime(G4double timeDelay) const
{
  //  Convert the time delay to the local time.
  return theTimeChange + timeDelay;
}

inline
const G4ThreeVector* G4ParticleChangeForRadDecay::GetPolarization() const
{
  return &thePolarizationChange;
}

inline
void G4ParticleChangeForRadDecay::ProposePolarization(
  const G4ThreeVector& finalPoralization)
{
  thePolarizationChange = finalPoralization;
}

inline
void G4ParticleChangeForRadDecay::ProposePolarization(G4double Px,
                                                   G4double Py,
                                                   G4double Pz)
{
  thePolarizationChange.setX(Px);
  thePolarizationChange.setY(Py);
  thePolarizationChange.setZ(Pz);
}