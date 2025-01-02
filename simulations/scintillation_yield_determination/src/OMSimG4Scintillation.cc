// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
////////////////////////////////////////////////////////////////////////
// Scintillation Light Class Implementation
////////////////////////////////////////////////////////////////////////
//
// File:        G4Scintillation.cc
// Description: RestDiscrete Process - Generation of Scintillation Photons
// Version:     1.0
// Created:     1998-11-07
// Author:      Peter Gumplinger
// Updated:     2010-10-20 Allow the scintillation yield to be a function
//              of energy deposited by particle type
//              Thanks to Zach Hartwig (Department of Nuclear
//              Science and Engineeering - MIT)
//              2010-09-22 by Peter Gumplinger
//              > scintillation rise time included, thanks to
//              > Martin Goettlich/DESY
//              2005-08-17 by Peter Gumplinger
//              > change variable name MeanNumPhotons -> MeanNumberOfPhotons
//              2005-07-28 by Peter Gumplinger
//              > add G4ProcessType to constructor
//              2004-08-05 by Peter Gumplinger
//              > changed StronglyForced back to Forced in GetMeanLifeTime
//              2002-11-21 by Peter Gumplinger
//              > change to use G4Poisson for small MeanNumberOfPhotons
//              2002-11-07 by Peter Gumplinger
//              > now allow for fast and slow scintillation component
//              2002-11-05 by Peter Gumplinger
//              > now use scintillation constants from G4Material
//              2002-05-09 by Peter Gumplinger
//              > use only the PostStepPoint location for the origin of
//                scintillation photons when energy is lost to the medium
//                by a neutral particle
//              2000-09-18 by Peter Gumplinger
//              > change: aSecondaryPosition=x0+rand*aStep.GetDeltaPosition();
//                        aSecondaryTrack->SetTouchable(0);
//              2001-09-17, migration of Materials to pure STL (mma)
//              2003-06-03, V.Ivanchenko fix compilation warnings
//
////////////////////////////////////////////////////////////////////////

#include "OMSimG4Scintillation.hh"
#include "OMSimLogger.hh"

#include "globals.hh"
#include "G4DynamicParticle.hh"
#include "G4EmProcessSubType.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4MaterialPropertyVector.hh"
#include "G4OpticalParameters.hh"
#include "G4ParticleMomentum.hh"
#include "G4ParticleTypes.hh"
#include "G4PhysicalConstants.hh"
#include "G4PhysicsFreeVector.hh"
#include "G4PhysicsTable.hh"
#include "G4Poisson.hh"
#include "G4ScintillationTrackInformation.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "Randomize.hh"
#include "G4PhysicsModelCatalog.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
OMSimG4Scintillation::OMSimG4Scintillation(const G4String &processName,
                                 G4ProcessType type)
    : G4VRestDiscreteProcess(processName, type), fIntegralTable1(nullptr), fIntegralTable2(nullptr), fIntegralTable3(nullptr), fEmSaturation(nullptr), fNumPhotons(0)
{
  secID = G4PhysicsModelCatalog::GetModelID("model_Scintillation");
  SetProcessSubType(fScintillation);

#ifdef G4DEBUG_SCINTILLATION
  ScintTrackEDep = 0.;
  ScintTrackYield = 0.;
#endif

  if (verboseLevel > 0)
  {
    G4cout << GetProcessName() << " is created " << G4endl;
  }
  Initialise();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
OMSimG4Scintillation::~OMSimG4Scintillation()
{
  if (fIntegralTable1 != nullptr)
  {
    fIntegralTable1->clearAndDestroy();
    delete fIntegralTable1;
  }
  if (fIntegralTable2 != nullptr)
  {
    fIntegralTable2->clearAndDestroy();
    delete fIntegralTable2;
  }
  if (fIntegralTable3 != nullptr)
  {
    fIntegralTable3->clearAndDestroy();
    delete fIntegralTable3;
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::ProcessDescription(std::ostream &out) const
{
  out << "Scintillation simulates production of optical photons produced\n"
         "by a high energy particle traversing matter.\n"
         "Various material properties need to be defined.\n";
  G4VRestDiscreteProcess::DumpInfo();

  G4OpticalParameters *params = G4OpticalParameters::Instance();
  out << "Track secondaries first: " << params->GetScintTrackSecondariesFirst();
  out << "Finite rise time: " << params->GetScintFiniteRiseTime();
  out << "Scintillation by particle type: " << params->GetScintByParticleType();
  out << "Save track information: " << params->GetScintTrackInfo();
  out << "Stack photons: " << params->GetScintStackPhotons();
  out << "Verbose level: " << params->GetScintVerboseLevel();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4bool OMSimG4Scintillation::IsApplicable(const G4ParticleDefinition &aParticleType)
{
  if (aParticleType.GetParticleName() == "opticalphoton")
    return false;
  if (aParticleType.IsShortLived())
    return false;
  return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::PreparePhysicsTable(const G4ParticleDefinition &)
{
  Initialise();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::Initialise()
{
  G4OpticalParameters *params = G4OpticalParameters::Instance();
  SetTrackSecondariesFirst(params->GetScintTrackSecondariesFirst());
  SetFiniteRiseTime(params->GetScintFiniteRiseTime());
  SetScintillationByParticleType(params->GetScintByParticleType());
  SetScintillationTrackInfo(params->GetScintTrackInfo());
  SetStackPhotons(params->GetScintStackPhotons());
  SetVerboseLevel(params->GetScintVerboseLevel());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::BuildPhysicsTable(const G4ParticleDefinition &)
{
  if (fIntegralTable1 != nullptr)
  {
    fIntegralTable1->clearAndDestroy();
    delete fIntegralTable1;
    fIntegralTable1 = nullptr;
  }
  if (fIntegralTable2 != nullptr)
  {
    fIntegralTable2->clearAndDestroy();
    delete fIntegralTable2;
    fIntegralTable2 = nullptr;
  }
  if (fIntegralTable3 != nullptr)
  {
    fIntegralTable3->clearAndDestroy();
    delete fIntegralTable3;
    fIntegralTable3 = nullptr;
  }

  const G4MaterialTable *materialTable = G4Material::GetMaterialTable();
  std::size_t numOfMaterials = G4Material::GetNumberOfMaterials();

  // create new physics table
  if (!fIntegralTable1)
    fIntegralTable1 = new G4PhysicsTable(numOfMaterials);
  if (!fIntegralTable2)
    fIntegralTable2 = new G4PhysicsTable(numOfMaterials);
  if (!fIntegralTable3)
    fIntegralTable3 = new G4PhysicsTable(numOfMaterials);

  for (std::size_t i = 0; i < numOfMaterials; ++i)
  {
    auto vector1 = new G4PhysicsFreeVector();
    auto vector2 = new G4PhysicsFreeVector();
    auto vector3 = new G4PhysicsFreeVector();

    // Retrieve vector of scintillation wavelength intensity for
    // the material from the material's optical properties table.
    G4MaterialPropertiesTable *MPT =
        ((*materialTable)[i])->GetMaterialPropertiesTable();

    if (MPT)
    {
      G4MaterialPropertyVector *MPV =
          MPT->GetProperty(kSCINTILLATIONCOMPONENT1);
      if (MPV)
      {

        // Retrieve the first intensity point in vector
        // of (photon energy, intensity) pairs
        G4double currentIN = (*MPV)[0];
        if (currentIN >= 0.0)
        {
          // Create first (photon energy, Scintillation Integral pair
          G4double currentPM = MPV->Energy(0);
          G4double currentCII = 0.0;
          vector1->InsertValues(currentPM, currentCII);

          // Set previous values to current ones prior to loop
          G4double prevPM = currentPM;
          G4double prevCII = currentCII;
          G4double prevIN = currentIN;

          // loop over all (photon energy, intensity)
          // pairs stored for this material
          for (std::size_t ii = 1; ii < MPV->GetVectorLength(); ++ii)
          {
            currentPM = MPV->Energy(ii);
            currentIN = (*MPV)[ii];
            currentCII =
                prevCII + 0.5 * (currentPM - prevPM) * (prevIN + currentIN);

            vector1->InsertValues(currentPM, currentCII);

            prevPM = currentPM;
            prevCII = currentCII;
            prevIN = currentIN;
          }
        }
      }

      MPV = MPT->GetProperty(kSCINTILLATIONCOMPONENT2);
      if (MPV)
      {
        // Retrieve the first intensity point in vector
        // of (photon energy, intensity) pairs
        G4double currentIN = (*MPV)[0];
        if (currentIN >= 0.0)
        {
          // Create first (photon energy, Scintillation Integral pair
          G4double currentPM = MPV->Energy(0);
          G4double currentCII = 0.0;
          vector2->InsertValues(currentPM, currentCII);

          // Set previous values to current ones prior to loop
          G4double prevPM = currentPM;
          G4double prevCII = currentCII;
          G4double prevIN = currentIN;

          // loop over all (photon energy, intensity)
          // pairs stored for this material
          for (std::size_t ii = 1; ii < MPV->GetVectorLength(); ++ii)
          {
            currentPM = MPV->Energy(ii);
            currentIN = (*MPV)[ii];
            currentCII =
                prevCII + 0.5 * (currentPM - prevPM) * (prevIN + currentIN);

            vector2->InsertValues(currentPM, currentCII);

            prevPM = currentPM;
            prevCII = currentCII;
            prevIN = currentIN;
          }
        }
      }
      MPV = MPT->GetProperty(kSCINTILLATIONCOMPONENT3);
      if (MPV)
      {
        // Retrieve the first intensity point in vector
        // of (photon energy, intensity) pairs
        G4double currentIN = (*MPV)[0];
        if (currentIN >= 0.0)
        {
          // Create first (photon energy, Scintillation Integral pair
          G4double currentPM = MPV->Energy(0);
          G4double currentCII = 0.0;
          vector3->InsertValues(currentPM, currentCII);

          // Set previous values to current ones prior to loop
          G4double prevPM = currentPM;
          G4double prevCII = currentCII;
          G4double prevIN = currentIN;

          // loop over all (photon energy, intensity)
          // pairs stored for this material
          for (std::size_t ii = 1; ii < MPV->GetVectorLength(); ++ii)
          {
            currentPM = MPV->Energy(ii);
            currentIN = (*MPV)[ii];
            currentCII =
                prevCII + 0.5 * (currentPM - prevPM) * (prevIN + currentIN);

            vector3->InsertValues(currentPM, currentCII);

            prevPM = currentPM;
            prevCII = currentCII;
            prevIN = currentIN;
          }
        }
      }
    }
    fIntegralTable1->insertAt(i, vector1);
    fIntegralTable2->insertAt(i, vector2);
    fIntegralTable3->insertAt(i, vector3);
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4VParticleChange *OMSimG4Scintillation::AtRestDoIt(const G4Track &aTrack,
                                               const G4Step &aStep)
// This routine simply calls the equivalent PostStepDoIt since all the
// necessary information resides in aStep.GetTotalEnergyDeposit()
{
  return OMSimG4Scintillation::PostStepDoIt(aTrack, aStep);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4VParticleChange *OMSimG4Scintillation::PostStepDoIt(const G4Track &aTrack,
                                                 const G4Step &aStep)
// This routine is called for each tracking step of a charged particle
// in a scintillator. A Poisson/Gauss-distributed number of photons is
// generated according to the scintillation yield formula, distributed
// evenly along the track segment and uniformly into 4pi.
{
  aParticleChange.Initialize(aTrack);
  fNumPhotons = 0;

  const G4DynamicParticle *aParticle = aTrack.GetDynamicParticle();
  const G4Material *aMaterial = aTrack.GetMaterial();

  G4StepPoint *pPreStepPoint = aStep.GetPreStepPoint();
  G4StepPoint *pPostStepPoint = aStep.GetPostStepPoint();

  G4ThreeVector x0 = pPreStepPoint->GetPosition();
  G4ThreeVector p0 = aStep.GetDeltaPosition().unit();
  G4double t0 = pPreStepPoint->GetGlobalTime();

  G4double TotalEnergyDeposit = aStep.GetTotalEnergyDeposit();

  G4MaterialPropertiesTable *MPT = aMaterial->GetMaterialPropertiesTable();
  if (!MPT)
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);

  G4MaterialPropertyVector *lScintSpectrum = MPT->GetProperty(kSCINTILLATIONCOMPONENT1);

  G4MaterialPropertyVector *lScintComponents = MPT->GetProperty("FRACTIONLIFETIMES");

  G4int nscnt = 0;
  if (!lScintSpectrum)
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);

  if (lScintSpectrum)
    nscnt = lScintComponents->GetVectorLength();

  G4double ResolutionScale = MPT->GetConstProperty("RESOLUTIONSCALE");

  G4double ScintillationYield = MPT->GetConstProperty("SCINTILLATIONYIELD");

  G4ParticleDefinition *pDef = aParticle->GetDefinition();

  if (pDef == G4Electron::ElectronDefinition() || pDef == G4Positron::PositronDefinition() || pDef == G4Gamma::GammaDefinition())
  {
    ScintillationYield = MPT->GetConstProperty("SCINTILLATIONYIELDELECTRONS");
  }

  G4double MeanNumberOfPhotons = ScintillationYield * TotalEnergyDeposit;
  
  G4int NumPhotons;
  if (MeanNumberOfPhotons > 10.)
  {
    G4double sigma = ResolutionScale * std::sqrt(MeanNumberOfPhotons);
    NumPhotons = G4int(G4RandGauss::shoot(MeanNumberOfPhotons, sigma) + 0.5);
  }
  else
  {
    NumPhotons = G4int(G4Poisson(MeanNumberOfPhotons));
  }
 
  if (NumPhotons <= 0 || !fStackingFlag)
  {
    // return unchanged particle and no secondaries
    aParticleChange.SetNumberOfSecondaries(0);
    return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
  }

  aParticleChange.SetNumberOfSecondaries(NumPhotons);

  if (fTrackSecondariesFirst)
  {
    if (aTrack.GetTrackStatus() == fAlive)
      aParticleChange.ProposeTrackStatus(fSuspend);
  }

  G4int materialIndex = (G4int)aMaterial->GetIndex();

  std::vector<G4double> lFractions;
  std::vector<G4double> lTimes;

  G4double lFractionNormalization = 0;
  for (G4int scnt = 0; scnt < nscnt; scnt++)
  {
    G4double lCurrentFraction = lScintComponents->Energy(scnt); // don't get confused about the energy here, it is a fraction... I wanted to use the G4 vectors, and this is the nomenclature
    G4double lCurrentTime = lScintComponents->Value(lCurrentFraction);
    lFractionNormalization += lCurrentFraction;
    lTimes.push_back(lCurrentTime);
    lFractions.push_back(lCurrentFraction);
  }

  
std::vector<G4int> Nums(nscnt, 0);
G4int lTotalNr = 0;

for (G4int scnt = 0; scnt < nscnt; scnt++)
{
    lFractions[scnt] /= lFractionNormalization;
    Nums[scnt] = G4int(lFractions[scnt] * NumPhotons);
    lTotalNr += Nums[scnt];
}

G4double difference = lTotalNr - NumPhotons;

if (difference != 0)
{
    // Prepare a list of the fractional parts
    std::vector<std::pair<G4double, G4int>> photonFractions;
    G4double totalFractionalSum = 0.0;
    for (G4int scnt = 0; scnt < nscnt; scnt++) 
    {
        G4double fractionalPart = lFractions[scnt] * NumPhotons - Nums[scnt];
        photonFractions.push_back(std::make_pair(fractionalPart, scnt));
        totalFractionalSum += fractionalPart;
    }

    // Distribute the leftover photons
    for (G4int i = 0; i < abs(difference); i++) 
    {
        G4double randValue = G4UniformRand() * totalFractionalSum;
        G4double cumulativeSum = 0;

        for (auto& p : photonFractions)
        {
            cumulativeSum += p.first;
            if (randValue < cumulativeSum)
            {
                if (difference < 0)
                    Nums[p.second]++;
                else
                    Nums[p.second]--;
                totalFractionalSum -= p.first;
                p.first = 0; // Ensure this bin is not selected again
                break;
            }
        }
    }
}

lTotalNr =0;
for (G4int scnt = 0; scnt < nscnt; scnt++)
{

    lTotalNr += Nums[scnt];
}
if (lTotalNr != NumPhotons){
  G4cout << lTotalNr << " " << NumPhotons << G4endl;
}

  for (G4int scnt = 1; scnt <= nscnt; scnt++)
  {

    G4double ScintillationRiseTime = 0. * ns;

    G4int Num = Nums[scnt - 1];

    G4double ScintillationTime = lTimes[scnt - 1];
    G4PhysicsFreeVector *ScintillationIntegral = (G4PhysicsFreeVector *)((*fIntegralTable1)(materialIndex));

    if (!ScintillationIntegral)
      continue;

    // Max Scintillation Integral

    G4double CIImax = ScintillationIntegral->GetMaxValue();

    for (G4int i = 0; i < Num; i++)
    {

      // Determine photon energy

      G4double CIIvalue = G4UniformRand() * CIImax;
      G4double sampledEnergy =
          ScintillationIntegral->GetEnergy(CIIvalue);

      if (verboseLevel > 1)
      {
        G4cout << "sampledEnergy = " << sampledEnergy << G4endl;
        G4cout << "CIIvalue =        " << CIIvalue << G4endl;
      }

      // Generate random photon direction

      G4double cost = 1. - 2. * G4UniformRand();
      G4double sint = std::sqrt((1. - cost) * (1. + cost));

      G4double phi = twopi * G4UniformRand();
      G4double sinp = std::sin(phi);
      G4double cosp = std::cos(phi);

      G4double px = sint * cosp;
      G4double py = sint * sinp;
      G4double pz = cost;

      // Create photon momentum direction vector

      G4ParticleMomentum photonMomentum(px, py, pz);

      // Determine polarization of new photon

      G4double sx = cost * cosp;
      G4double sy = cost * sinp;
      G4double sz = -sint;

      G4ThreeVector photonPolarization(sx, sy, sz);

      G4ThreeVector perp = photonMomentum.cross(photonPolarization);
      // G4cout << photonMomentum.x() << " " << photonMomentum.y() << " " << photonMomentum.z() << G4endl;
      phi = twopi * G4UniformRand();
      sinp = std::sin(phi);
      cosp = std::cos(phi);

      photonPolarization = cosp * photonPolarization + sinp * perp;

      photonPolarization = photonPolarization.unit();

      // Generate a new photon:

      G4DynamicParticle *aScintillationPhoton =
          new G4DynamicParticle(G4OpticalPhoton::OpticalPhoton(),
                                photonMomentum);
      aScintillationPhoton->SetPolarization(photonPolarization.x(),
                                            photonPolarization.y(),
                                            photonPolarization.z());

      aScintillationPhoton->SetKineticEnergy(sampledEnergy);

      // Generate new G4Track object:

      G4double rand;

      if (aParticle->GetDefinition()->GetPDGCharge() != 0)
      {
        rand = G4UniformRand();
      }
      else
      {
        rand = 1.0;
      }

      G4double delta = rand * aStep.GetStepLength();
      G4double deltaTime = delta /
                           ((pPreStepPoint->GetVelocity() +
                             pPostStepPoint->GetVelocity()) /
                            2.);

      // emission time distribution
      if (ScintillationRiseTime == 0.0)
      {
        deltaTime = deltaTime -
                    ScintillationTime * std::log(G4UniformRand());
      }
      else
      {
        deltaTime = deltaTime +
                    sample_time(ScintillationRiseTime, ScintillationTime);
      }

      G4double aSecondaryTime = t0 + deltaTime;

      // G4cout << deltaTime << " " << t0 << " " <<aSecondaryTime<<  G4endl;

      G4ThreeVector aSecondaryPosition =
          x0 + rand * aStep.GetDeltaPosition();

      G4Track *aSecondaryTrack =
          new G4Track(aScintillationPhoton, aSecondaryTime, aSecondaryPosition);

      aSecondaryTrack->SetTouchableHandle(
          aStep.GetPreStepPoint()->GetTouchableHandle());
      // aSecondaryTrack->SetTouchableHandle((G4VTouchable*)0);

      aSecondaryTrack->SetParentID(aTrack.GetTrackID());
      aSecondaryTrack->SetCreatorModelID(secID);
      aParticleChange.AddSecondary(aSecondaryTrack);
    }
  }

  if (verboseLevel > 1)
  {
    G4cout << "\n Exiting from OMSimG4Scintillation::DoIt -- NumberOfSecondaries = "
           << aParticleChange.GetNumberOfSecondaries() << G4endl;
  }

  return G4VRestDiscreteProcess::PostStepDoIt(aTrack, aStep);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4double OMSimG4Scintillation::GetMeanFreePath(const G4Track &, G4double,
                                          G4ForceCondition *condition)
{
  *condition = StronglyForced;
  return DBL_MAX;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4double OMSimG4Scintillation::GetMeanLifeTime(const G4Track &,
                                          G4ForceCondition *condition)
{
  *condition = Forced;
  return DBL_MAX;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4double OMSimG4Scintillation::sample_time(G4double tau1, G4double tau2)
{
  // tau1: rise time and tau2: decay time
  // Loop checking, 07-Aug-2015, Vladimir Ivanchenko
  while (true)
  {
    G4double ran1 = G4UniformRand();
    G4double ran2 = G4UniformRand();

    // exponential distribution as envelope function: very efficient
    G4double d = (tau1 + tau2) / tau2;
    // make sure the envelope function is
    // always larger than the bi-exponential
    G4double t = -1.0 * tau2 * std::log(1. - ran1);
    G4double gg = d * single_exp(t, tau2);
    if (ran2 <= bi_exp(t, tau1, tau2) / gg)
      return t;
  }
  return -1.0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
G4double OMSimG4Scintillation::GetScintillationYieldByParticleType(
    const G4Track &aTrack, const G4Step &aStep, G4double &yield1,
    G4double &yield2, G4double &yield3)
{
  // new in 10.7, allow multiple time constants with ScintByParticleType
  // Get the G4MaterialPropertyVector containing the scintillation
  // yield as a function of the energy deposited and particle type

  G4ParticleDefinition *pDef = aTrack.GetDynamicParticle()->GetDefinition();
  G4MaterialPropertyVector *yieldVector = nullptr;
  G4MaterialPropertiesTable *MPT =
      aTrack.GetMaterial()->GetMaterialPropertiesTable();

  // Protons
  if (pDef == G4Proton::ProtonDefinition())
  {
    yieldVector = MPT->GetProperty(kPROTONSCINTILLATIONYIELD);
    yield1 = MPT->ConstPropertyExists(kPROTONSCINTILLATIONYIELD1)
                 ? MPT->GetConstProperty(kPROTONSCINTILLATIONYIELD1)
                 : 1.;
    yield2 = MPT->ConstPropertyExists(kPROTONSCINTILLATIONYIELD2)
                 ? MPT->GetConstProperty(kPROTONSCINTILLATIONYIELD2)
                 : 0.;
    yield3 = MPT->ConstPropertyExists(kPROTONSCINTILLATIONYIELD3)
                 ? MPT->GetConstProperty(kPROTONSCINTILLATIONYIELD3)
                 : 0.;
  }

  // Deuterons
  else if (pDef == G4Deuteron::DeuteronDefinition())
  {
    yieldVector = MPT->GetProperty(kDEUTERONSCINTILLATIONYIELD);
    yield1 = MPT->ConstPropertyExists(kDEUTERONSCINTILLATIONYIELD1)
                 ? MPT->GetConstProperty(kDEUTERONSCINTILLATIONYIELD1)
                 : 1.;
    yield2 = MPT->ConstPropertyExists(kDEUTERONSCINTILLATIONYIELD2)
                 ? MPT->GetConstProperty(kDEUTERONSCINTILLATIONYIELD2)
                 : 0.;
    yield3 = MPT->ConstPropertyExists(kDEUTERONSCINTILLATIONYIELD3)
                 ? MPT->GetConstProperty(kDEUTERONSCINTILLATIONYIELD3)
                 : 0.;
  }

  // Tritons
  else if (pDef == G4Triton::TritonDefinition())
  {
    yieldVector = MPT->GetProperty(kTRITONSCINTILLATIONYIELD);
    yield1 = MPT->ConstPropertyExists(kTRITONSCINTILLATIONYIELD1)
                 ? MPT->GetConstProperty(kTRITONSCINTILLATIONYIELD1)
                 : 1.;
    yield2 = MPT->ConstPropertyExists(kTRITONSCINTILLATIONYIELD2)
                 ? MPT->GetConstProperty(kTRITONSCINTILLATIONYIELD2)
                 : 0.;
    yield3 = MPT->ConstPropertyExists(kTRITONSCINTILLATIONYIELD3)
                 ? MPT->GetConstProperty(kTRITONSCINTILLATIONYIELD3)
                 : 0.;
  }

  // Alphas
  else if (pDef == G4Alpha::AlphaDefinition())
  {
    yieldVector = MPT->GetProperty(kALPHASCINTILLATIONYIELD);
    yield1 = MPT->ConstPropertyExists(kALPHASCINTILLATIONYIELD1)
                 ? MPT->GetConstProperty(kALPHASCINTILLATIONYIELD1)
                 : 1.;
    yield2 = MPT->ConstPropertyExists(kALPHASCINTILLATIONYIELD2)
                 ? MPT->GetConstProperty(kALPHASCINTILLATIONYIELD2)
                 : 0.;
    yield3 = MPT->ConstPropertyExists(kALPHASCINTILLATIONYIELD3)
                 ? MPT->GetConstProperty(kALPHASCINTILLATIONYIELD3)
                 : 0.;
  }

  // Ions (particles derived from G4VIon and G4Ions) and recoil ions
  // below the production cut from neutrons after hElastic
  else if (pDef->GetParticleType() == "nucleus" ||
           pDef == G4Neutron::NeutronDefinition())
  {
    yieldVector = MPT->GetProperty(kIONSCINTILLATIONYIELD);
    yield1 = MPT->ConstPropertyExists(kIONSCINTILLATIONYIELD1)
                 ? MPT->GetConstProperty(kIONSCINTILLATIONYIELD1)
                 : 1.;
    yield2 = MPT->ConstPropertyExists(kIONSCINTILLATIONYIELD2)
                 ? MPT->GetConstProperty(kIONSCINTILLATIONYIELD2)
                 : 0.;
    yield3 = MPT->ConstPropertyExists(kIONSCINTILLATIONYIELD3)
                 ? MPT->GetConstProperty(kIONSCINTILLATIONYIELD3)
                 : 0.;
  }

  // Electrons (must also account for shell-binding energy
  // attributed to gamma from standard photoelectric effect)
  // and, default for particles not enumerated/listed above
  else
  {
    yieldVector = MPT->GetProperty(kELECTRONSCINTILLATIONYIELD);
    yield1 = MPT->ConstPropertyExists(kELECTRONSCINTILLATIONYIELD1)
                 ? MPT->GetConstProperty(kELECTRONSCINTILLATIONYIELD1)
                 : 1.;
    yield2 = MPT->ConstPropertyExists(kELECTRONSCINTILLATIONYIELD2)
                 ? MPT->GetConstProperty(kELECTRONSCINTILLATIONYIELD2)
                 : 0.;
    yield3 = MPT->ConstPropertyExists(kELECTRONSCINTILLATIONYIELD3)
                 ? MPT->GetConstProperty(kELECTRONSCINTILLATIONYIELD3)
                 : 0.;
  }

  // Throw an exception if no scintillation yield vector is found
  if (!yieldVector)
  {
    G4ExceptionDescription ed;
    ed << "\nG4Scintillation::PostStepDoIt(): "
       << "Request for scintillation yield for energy deposit and particle\n"
       << "type without correct entry in MaterialPropertiesTable.\n"
       << "ScintillationByParticleType requires at minimum that \n"
       << "ELECTRONSCINTILLATIONYIELD is set by the user\n"
       << G4endl;
    G4String comments = "Missing MaterialPropertiesTable entry - No correct "
                        "entry in MaterialPropertiesTable";
    G4Exception("OMSimG4Scintillation::PostStepDoIt", "Scint01", FatalException, ed,
                comments);
  }

  ///////////////////////////////////////
  // Calculate the scintillation light //
  ///////////////////////////////////////
  // To account for potential nonlinearity and scintillation photon
  // density along the track, light (L) is produced according to:
  // L_currentStep = L(PreStepKE) - L(PreStepKE - EDep)

  G4double ScintillationYield = 0.;
  G4double StepEnergyDeposit = aStep.GetTotalEnergyDeposit();
  G4double PreStepKineticEnergy = aStep.GetPreStepPoint()->GetKineticEnergy();

  if (PreStepKineticEnergy <= yieldVector->GetMaxEnergy())
  {
    // G4double Yield1 = yieldVector->Value(PreStepKineticEnergy);
    // G4double Yield2 = yieldVector->Value(PreStepKineticEnergy -
    // StepEnergyDeposit); ScintillationYield = Yield1 - Yield2;
    ScintillationYield =
        yieldVector->Value(PreStepKineticEnergy) -
        yieldVector->Value(PreStepKineticEnergy - StepEnergyDeposit);
  }
  else
  {
    G4ExceptionDescription ed;
    ed << "\nG4Scintillation::GetScintillationYieldByParticleType(): Request\n"
       << "for scintillation light yield above the available energy range\n"
       << "specified in G4MaterialPropertiesTable. A linear interpolation\n"
       << "will be performed to compute the scintillation light yield using\n"
       << "(L_max / E_max) as the photon yield per unit energy." << G4endl;
    G4String cmt = "\nScintillation yield may be unphysical!\n";
    G4Exception("OMSimG4Scintillation::GetScintillationYieldByParticleType()",
                "Scint03", JustWarning, ed, cmt);

    // Units: [# scintillation photons]
    ScintillationYield = yieldVector->GetMaxValue() /
                         yieldVector->GetMaxEnergy() * StepEnergyDeposit;
  }

#ifdef G4DEBUG_SCINTILLATION
  // Increment track aggregators
  ScintTrackYield += ScintillationYield;
  ScintTrackEDep += StepEnergyDeposit;

  G4cout << "\n--- OMSimG4Scintillation::GetScintillationYieldByParticleType() ---\n"
         << "--\n"
         << "--  Name         =  "
         << aTrack.GetParticleDefinition()->GetParticleName() << "\n"
         << "--  TrackID      =  " << aTrack.GetTrackID() << "\n"
         << "--  ParentID     =  " << aTrack.GetParentID() << "\n"
         << "--  Current KE   =  " << aTrack.GetKineticEnergy() / MeV
         << " MeV\n"
         << "--  Step EDep    =  " << aStep.GetTotalEnergyDeposit() / MeV
         << " MeV\n"
         << "--  Track EDep   =  " << ScintTrackEDep / MeV << " MeV\n"
         << "--  Vertex KE    =  " << aTrack.GetVertexKineticEnergy() / MeV
         << " MeV\n"
         << "--  Step yield   =  " << ScintillationYield << " photons\n"
         << "--  Track yield  =  " << ScintTrackYield << " photons\n"
         << G4endl;

  // The track has terminated within or has left the scintillator volume
  if ((aTrack.GetTrackStatus() == fStopButAlive) or
      (aStep.GetPostStepPoint()->GetStepStatus() == fGeomBoundary))
  {
    // Reset aggregators for the next track
    ScintTrackEDep = 0.;
    ScintTrackYield = 0.;
  }
#endif

  return ScintillationYield;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::DumpPhysicsTable() const
{
  if (fIntegralTable1)
  {
    for (std::size_t i = 0; i < fIntegralTable1->entries(); ++i)
    {
      ((G4PhysicsFreeVector *)(*fIntegralTable1)[i])->DumpValues();
    }
  }
  if (fIntegralTable2)
  {
    for (std::size_t i = 0; i < fIntegralTable2->entries(); ++i)
    {
      ((G4PhysicsFreeVector *)(*fIntegralTable2)[i])->DumpValues();
    }
  }
  if (fIntegralTable3)
  {
    for (std::size_t i = 0; i < fIntegralTable3->entries(); ++i)
    {
      ((G4PhysicsFreeVector *)(*fIntegralTable3)[i])->DumpValues();
    }
  }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::SetTrackSecondariesFirst(const G4bool state)
{
  fTrackSecondariesFirst = state;
  G4OpticalParameters::Instance()->SetScintTrackSecondariesFirst(
      fTrackSecondariesFirst);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::SetFiniteRiseTime(const G4bool state)
{
  fFiniteRiseTime = state;
  G4OpticalParameters::Instance()->SetScintFiniteRiseTime(fFiniteRiseTime);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::SetScintillationByParticleType(const G4bool scintType)
{
  if (fEmSaturation && scintType)
  {
    G4Exception("OMSimG4Scintillation::SetScintillationByParticleType", "Scint02",
                JustWarning,
                "Redefinition: Birks Saturation is replaced by "
                "ScintillationByParticleType!");
    RemoveSaturation();
  }
  fScintillationByParticleType = scintType;
  G4OpticalParameters::Instance()->SetScintByParticleType(
      fScintillationByParticleType);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::SetScintillationTrackInfo(const G4bool trackType)
{
  fScintillationTrackInfo = trackType;
  G4OpticalParameters::Instance()->SetScintTrackInfo(fScintillationTrackInfo);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::SetStackPhotons(const G4bool stackingFlag)
{
  fStackingFlag = stackingFlag;
  G4OpticalParameters::Instance()->SetScintStackPhotons(fStackingFlag);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
void OMSimG4Scintillation::SetVerboseLevel(G4int verbose)
{
  verboseLevel = verbose;
  G4OpticalParameters::Instance()->SetScintVerboseLevel(verboseLevel);
}
