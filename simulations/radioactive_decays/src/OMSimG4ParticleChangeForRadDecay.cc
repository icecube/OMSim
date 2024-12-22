
#include "OMSimG4ParticleChangeForRadDecay.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4DynamicParticle.hh"
#include "G4ExceptionSeverity.hh"

// --------------------------------------------------------------------
G4ParticleChangeForRadDecay::G4ParticleChangeForRadDecay()
{}

// --------------------------------------------------------------------
void G4ParticleChangeForRadDecay::Initialize(const G4Track& track)
{
  // use base class's method at first
  G4VParticleChange::Initialize(track);

  // set TimeChange equal to local time of the parent track
  theLocalTime0 = theTimeChange = track.GetLocalTime();

  // set initial Local/Global time of the parent track
  theGlobalTime0 = track.GetGlobalTime();

  // set the Polarization equal to those of the parent track
  thePolarizationChange = track.GetDynamicParticle()->GetPolarization();
}

// --------------------------------------------------------------------
G4Step* G4ParticleChangeForRadDecay::UpdateStepForPostStep(G4Step* pStep)
{
  G4StepPoint* pPostStepPoint = pStep->GetPostStepPoint();

  if(isParentWeightProposed)
  {
    pPostStepPoint->SetWeight(theParentWeight);
  }
  // update polarization
  pPostStepPoint->SetPolarization(thePolarizationChange);

  // update the G4Step specific attributes
  return UpdateStepInfo(pStep);
}

// --------------------------------------------------------------------
G4Step* G4ParticleChangeForRadDecay::UpdateStepForAtRest(G4Step* pStep)
{
  // A physics process always calculates the final state of the particle

  G4StepPoint* pPostStepPoint = pStep->GetPostStepPoint();

  // update polarization
  pPostStepPoint->SetPolarization(thePolarizationChange);

  // update time
  pPostStepPoint->SetGlobalTime(GetGlobalTime());
  pPostStepPoint->SetLocalTime(theTimeChange);
  pPostStepPoint->AddProperTime(theTimeChange - theLocalTime0);

#ifdef G4VERBOSE
  if(debugFlag) { CheckIt(*theCurrentTrack); }
#endif

  if(isParentWeightProposed)
    pPostStepPoint->SetWeight(theParentWeight);

  //  Update the G4Step specific attributes
  return UpdateStepInfo(pStep);
}

// --------------------------------------------------------------------
void G4ParticleChangeForRadDecay::DumpInfo() const
{
  // Show header
  G4VParticleChange::DumpInfo();

  G4long oldprc = G4cout.precision(8);
  G4cout << "      -----------------------------------------------" << G4endl;
  G4cout << "    G4ParticleChangeForRadDecay proposes: " << G4endl;
  G4cout << "    Proposed local Time (ns): " << std::setw(20)
         << theTimeChange / ns << G4endl;
  G4cout << "    Initial local Time (ns) : " << std::setw(20)
         << theLocalTime0 / ns << G4endl;
  G4cout << "    Initial global Time (ns): " << std::setw(20)
         << theGlobalTime0 / ns << G4endl;
  G4cout << "    Current global Time (ns): " << std::setw(20)
         << theCurrentTrack->GetGlobalTime() / ns << G4endl;
  G4cout.precision(oldprc);
}

// --------------------------------------------------------------------
G4bool G4ParticleChangeForRadDecay::CheckIt(const G4Track& aTrack)
{
  // local time should not go back
  G4bool itsOK = true;
  if(theTimeChange < theLocalTime0)
  {
    itsOK         = false;
    ++nError;
#ifdef G4VERBOSE
    if(nError < maxError)
    {
      G4cout << "  G4ParticleChangeForRadDecay::CheckIt    : ";
      G4cout << "the local time goes back  !!"
	     << "  Difference:  " << (theTimeChange - theLocalTime0) / ns 
             << "[ns] " << G4endl;
      G4cout << "initial local time " << theLocalTime0 / ns << "[ns] "
	     << "initial global time " << theGlobalTime0 / ns << "[ns] "
	     << G4endl;
    }
#endif
    theTimeChange = theLocalTime0;
  }

  if(!itsOK)
  {
    if(nError < maxError)
    {
#ifdef G4VERBOSE
      // dump out information of this particle change
      DumpInfo();
#endif
      G4Exception("G4ParticleChangeForRadDecay::CheckIt()", "TRACK005",
                  JustWarning, "time is illegal");
    }
  }

  return (itsOK) && G4VParticleChange::CheckIt(aTrack);
}



G4bool G4ParticleChangeForRadDecay::CheckSecondary(G4Track& aTrack)
{
  G4bool isOK = true;

  // MomentumDirection should be unit vector
  G4double ekin = aTrack.GetKineticEnergy();
  auto dir = aTrack.GetMomentumDirection();
  G4double accuracy = std::abs(dir.mag2() - 1.0);
  if(accuracy > accuracyForWarning)
  {
    isOK = false;
    ++nError;
#ifdef G4VERBOSE
    if(nError < maxError)
    {
      G4String mname = aTrack.GetCreatorModelName();
      G4cout << " G4VParticleChange::CheckSecondary : " << G4endl;
      G4cout << " the momentum direction " << dir
	     << " is not unit vector !!" << G4endl;
      G4cout << " Difference=" << accuracy 
	     << " Ekin(MeV)=" << ekin/MeV 
	     << "  " << aTrack.GetParticleDefinition()->GetParticleName()
	     << " created by " << mname
             << G4endl;
    }
#endif
    aTrack.SetMomentumDirection(dir.unit());
  }

  // Kinetic Energy should not be negative
  if(ekin < 0.0)
  {
    isOK = false;
    ++nError;
#ifdef G4VERBOSE
    if(nError < maxError)
    {
      G4String mname = aTrack.GetCreatorModelName();
      G4cout << " G4VParticleChange::CheckSecondary : " << G4endl;
      G4cout << " Ekin(MeV)=" << ekin << " is negative !!  "
	     << aTrack.GetParticleDefinition()->GetParticleName()
	     << " created by " << mname
             << G4endl;
    }
#endif
    aTrack.SetKineticEnergy(0.0);
  }
    ////OMSIM//////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////
  // Check timing of secondaries
//   G4double time = aTrack.GetGlobalTime();
//   if(time < theParentGlobalTime)
//   {
//     isOK = false;
//     ++nError;
// #ifdef G4VERBOSE
//     if(nError < maxError)
//     {
//       G4String mname = aTrack.GetCreatorModelName();
//       G4cout << " G4VParticleChange::CheckSecondary : " << G4endl;
//       G4cout << " The global time of secondary goes back compared to the parent !!" << G4endl;
//       G4cout << " ParentTime(ns)=" << theParentGlobalTime/ns
// 	     << " SecondaryTime(ns)= " << time/ns
// 	     << " Difference(ns)=" << (theParentGlobalTime - time)/ns
// 	     << G4endl;
//       G4cout << " Ekin(MeV)=" << ekin
// 	     << aTrack.GetParticleDefinition()->GetParticleName()
// 	     << " created by " << mname << G4endl;
//     }
// #endif
//     aTrack.SetGlobalTime(theParentGlobalTime);
//   }

    //////////////////////////////////////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////

  // Exit with error
  if(!isOK)
  {
    if(nError < maxError)
    {
#ifdef G4VERBOSE
      DumpInfo();
#endif
      G4Exception("G4VParticleChange::CheckSecondary()", "TRACK001",
		  JustWarning, "Secondary with illegal time and/or energy and/or momentum");
    }
  }
  return isOK;
}

void G4ParticleChangeForRadDecay::AddSecondary(G4Track* aTrack)
{
  if(debugFlag)
    CheckSecondary(*aTrack);

  if(!fSetSecondaryWeightByProcess)
    aTrack->SetWeight(theParentWeight);

  // add a secondary after size check
  if(theSizeOftheListOfSecondaries > theNumberOfSecondaries)
  {
    theListOfSecondaries[theNumberOfSecondaries] = aTrack;
  }
  else
  {
    theListOfSecondaries.push_back(aTrack);
    ++theSizeOftheListOfSecondaries;
  }
  ++theNumberOfSecondaries;
}
