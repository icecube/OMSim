#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimSNAnalysis.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <OMSimInputData.hh>

void setupDistribution(DistributionSampler &sampler,
                       const std::vector<G4double> &xData,
                       const std::vector<G4double> &yData,
                       const G4String &name,
                       G4double xUnit,
                       G4double yUnit)
{
    sampler.setData(xData, yData, name);
    sampler.setUnits(xUnit, yUnit);
    sampler.makeInterpolator();
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                     Base Class
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

SNBaseParticleGenerator::SNBaseParticleGenerator(G4ParticleGun *gun)
    : mParticleGun(gun)
{
}

void SNBaseParticleGenerator::initialiseDistribution(int pColumnIndex, int pNrOfTargets)
{

    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false)
    {
        std::pair<std::string, std::string> lFluxNames = mSNToolBox.getFileNames(OMSimCommandArgsTable::getInstance().get<G4int>("SNtype"));
        std::string lFluxName;
        if (pColumnIndex == 1)
        {
            lFluxName = lFluxNames.first;
        }
        else
        {
            lFluxName = lFluxNames.second;
        }

        std::vector<std::vector<double>> lData = InputDataManager::loadtxt(lFluxName, true, 0, '\t');
        setupDistribution(mTimeDistribution, lData[0], lData[1], "SNLuminosityVsTime", 1 * s, 1);
        setupDistribution(mMeanEnergyDistribution, lData[0], lData[2], "SNMeanEnergyVsTime", 1 * s, 1 * MeV);
        setupDistribution(mMeanEnergySquaredDistribution, lData[0], lData[3], "SNMeanEnergySquaredVsTime", 1 * s, 1 * MeV * MeV);
    }
    mNrTargets = mSNToolBox.numberOfTargets(pNrOfTargets);
}

void SNBaseParticleGenerator::GeneratePrimaries(G4Event *pEvent)
{
    initialiseParticle();
    OMSimSNAnalysis &lAnalysisManager = OMSimSNAnalysis::getInstance();
    OMSimSNAnalysis::getInstance().initEventStat();
    G4double lNeutrinoEnergy = calculateNeutrinoEnergy();

    G4ThreeVector lMomentumDirection = calculateMomentumDirection(lNeutrinoEnergy);
    mParticleGun->SetParticleMomentumDirection(lMomentumDirection);

    G4double lParticleEnergy = calculateSecondaryParticleEnergy(lNeutrinoEnergy, lMomentumDirection.getZ());
    mParticleGun->SetParticleEnergy(lParticleEnergy);

    G4double lWeight = calculateWeight(lNeutrinoEnergy);

    
    lAnalysisManager.mEventStat->weight = lWeight;
    lAnalysisManager.mEventStat->primary_energy = lParticleEnergy;

    mParticleGun->GeneratePrimaryVertex(pEvent);
}

G4ThreeVector SNBaseParticleGenerator::calculateMomentumDirection(G4double pNuEnergy)
{
    G4double lCosTheta = angularDistribution(pNuEnergy).sampleFromDistribution();
    G4double lSinTheta = std::sqrt(1. - lCosTheta * lCosTheta);
    G4double phi = twopi * G4UniformRand();
    G4double zdir = -lCosTheta;
    G4double xdir = -lSinTheta * std::cos(phi);
    G4double ydir = -lSinTheta * std::sin(phi);
    OMSimSNAnalysis::getInstance().mEventStat->cos_theta = lCosTheta;
    return G4ThreeVector(xdir, ydir, zdir);
}

G4double SNBaseParticleGenerator::calculateWeight(G4double pNuEnergy)
{
    G4double lSigma = totalCrossSection(pNuEnergy);
    return mSNToolBox.calculateWeight(lSigma, mNrTargets);
}

G4double SNBaseParticleGenerator::calculateNeutrinoEnergy()
{
    G4double lThresholdEnergy, lTimeOfSpectrum, lMeanEnergy, lMeanEnergySquared, lAlpha, lNeutrinoEnergy;
    lThresholdEnergy = neutron_mass_c2 + electron_mass_c2 - proton_mass_c2 + 0.1 * MeV;
    bool lRetry = true;
    while (lRetry)
    {
        lRetry = false;
        lTimeOfSpectrum = -99.;
        lMeanEnergy = -99.;
        lMeanEnergySquared = -99.;
        lAlpha = -99.;
        lNeutrinoEnergy = -99.;

        if (!OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy"))
        {
            lTimeOfSpectrum = mTimeDistribution.sampleFromDistribution();
            lMeanEnergy = mMeanEnergyDistribution.interpolate(lTimeOfSpectrum);
            lMeanEnergySquared = mMeanEnergySquaredDistribution.interpolate(lTimeOfSpectrum);

            G4int count = 0;
            while (lNeutrinoEnergy <= lThresholdEnergy)
            {
                lNeutrinoEnergy = mSNToolBox.sampleEnergy(lMeanEnergy, lMeanEnergySquared, lAlpha);
                count += 1;
                if (count > 10)
                {
                    lRetry = true;
                    break;
                }
            }
        }
        else
        {
            lTimeOfSpectrum = 0.0;
            lMeanEnergy = OMSimCommandArgsTable::getInstance().get<G4double>("SNmeanE") * MeV;
            lAlpha = OMSimCommandArgsTable::getInstance().get<G4double>("SNalpha");
            lMeanEnergySquared = lMeanEnergy * lMeanEnergy * (2 + lAlpha) / (1 + lAlpha);
            lNeutrinoEnergy = mSNToolBox.sampleEnergy(lMeanEnergy, lMeanEnergySquared, lAlpha);
            while (lNeutrinoEnergy <= lThresholdEnergy)
            {
                lNeutrinoEnergy = mSNToolBox.sampleEnergy(lMeanEnergy, lMeanEnergySquared, lAlpha);
            }
        }
    }

    OMSimSNAnalysis &lAnalysisManager = OMSimSNAnalysis::getInstance();
    lAnalysisManager.mEventStat->neutrino_time = lTimeOfSpectrum;
    lAnalysisManager.mEventStat->mean_energy = lMeanEnergy;
    lAnalysisManager.mEventStat->neutrino_energy = lNeutrinoEnergy;
    return lNeutrinoEnergy;
}

/*
 * %%%%%%%%%%%%%%%% ENES %%%%%%%%%%%%%%%%
 */

/**
 * @brief Constructor of OMSimENES class
 * @param gun Pointer to the particle gun object used for simulation.
 */
OMSimENES::OMSimENES(G4ParticleGun *gun)
    : SNBaseParticleGenerator(gun)
{
    initialiseDistribution(1, 10); // 10 electrons per molecule
}

void OMSimENES::initialiseParticle()
{
    G4ParticleDefinition *lParticle = G4ParticleTable::GetParticleTable()->FindParticle("e-");
    mParticleGun->SetParticleDefinition(lParticle);
    G4ThreeVector lParticlePosition = mSNToolBox.randomPosition();
    mParticleGun->SetParticlePosition(lParticlePosition);
}

/**
 * @brief Computes the angular distribution for electron scattering based on incident neutrino energy.
 *
 * This function calculates the angular distribution for electrons scattered by neutrinos. The foundation for this
 * computation is taken from the reference:
 * "Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press, Chapter 5, eq. 5.29".
 *
 * @param Enu The energy of the incident neutrino.
 */
DistributionSampler OMSimENES::angularDistribution(G4double Enu)
{
    const int lAngularDistributionSize = 500;
    const G4double g1 = 0.73;
    const G4double g2 = 0.23;
    std::vector<G4double> lAngularDistribution_x, lAngularDistribution_y;

    G4double lSigma0 = 2. * pow(mGf, 2) * pow(electron_mass_c2, 2) / pi * pow(197.326e-15, 2);

    lAngularDistribution_x.resize(lAngularDistributionSize);
    lAngularDistribution_y.resize(lAngularDistributionSize);

    const double lMin = 0.;
    const double lMax = 1.;
    G4double delta = (lMax - lMin) / G4double(lAngularDistributionSize - 1);

    for (G4int i = 0; i < lAngularDistributionSize; i++)
    {
        lAngularDistribution_x[i] = lMin + i * delta; // costheta
    }

    for (G4int j = 0; j < lAngularDistributionSize; j++)
    {
        G4double dem = pow((pow((electron_mass_c2 + Enu), 2) - pow(Enu, 2) * pow(lAngularDistribution_x[j], 2)), 2);
        G4double factor1 = 4. * pow(Enu, 2) * pow((electron_mass_c2 + Enu), 2) * lAngularDistribution_x[j] / dem;
        G4double factor2 = 2. * electron_mass_c2 * Enu * pow(lAngularDistribution_x[j], 2) / dem;
        G4double factor3 = 2. * pow(electron_mass_c2, 2) * pow(lAngularDistribution_x[j], 2) / dem;
        lAngularDistribution_y[j] = lSigma0 * factor1 * (pow(g1, 2) + pow(g2, 2) * pow((1. - factor2), 2) - g1 * g2 * factor3); // dsigma/dcos(theta)
    }
    DistributionSampler lAngularDistribution;
    lAngularDistribution.setData(lAngularDistribution_x, lAngularDistribution_y, "AngularDistribution");
    lAngularDistribution.setUnits(1,1);
    return lAngularDistribution;
}

/**
 * @brief Computes the electron energy from elastic scattering based on incident neutrino energy and scatter angle.
 *
 * The computation utilizes the formula for the energy of the electron as a function of incident neutrino energy
 * and the scatter angle, based on the formula provided in the reference:
 * "Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press, Chapter 5, eq. 5.27".
 *
 * @param pNuEnergy The energy of the incident neutrino.
 * @param pCosTheta The cosine of the scattering angle.
 * @return Calculated energy value of the electron post scattering.
 */
G4double OMSimENES::calculateSecondaryParticleEnergy(G4double pNuEnergy, G4double pCosTheta)
{
    G4double lEnergy = 2 * electron_mass_c2 * pow(pNuEnergy, 2) * pow(pCosTheta, 2) / (pow((electron_mass_c2 + pNuEnergy), 2) - pow(pNuEnergy, 2) * pow(pCosTheta, 2));
    return lEnergy;
}

/**
 * @brief Computes the total cross section for ENES based on incident neutrino energy.
 *
 * This function determines the total cross-section for the scattering of neutrinos with electrons given a specific
 * incident neutrino energy. The computational foundation is derived from:
 * "M. Buchkremer, Electroweak Interactions: Neutral currents in neutrino-lepton elastic scattering experiments,
 * Université Catholique de Louvain /CP3, 2011."
 *
 * @param pEnergy The energy of the incident neutrino.
 * @return Calculated total cross-section value for the given energy.
 */
G4double OMSimENES::totalCrossSection(G4double pEnergy)
{
    double lSin2ThetaW = 0.231;
    G4double lSigma = pow(mGf, 2) * electron_mass_c2 * pEnergy / (2. * pi) * (1. + 4. * lSin2ThetaW + 16. / 3. * pow(lSin2ThetaW, 2)) * pow(197.326e-15, 2) * m * m;
    return lSigma;
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                     Derived Classes
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

/*
 * %%%%%%%%%%%%%%%% IBD %%%%%%%%%%%%%%%%
 */

/**
 * @brief Constructor of OMSimIBD class
 * @param gun Pointer to the particle gun object used for simulation.
 */
OMSimIBD::OMSimIBD(G4ParticleGun *gun)
    : SNBaseParticleGenerator(gun)
{
    initialiseDistribution(2, 2); // 2 protons (hydrogen) per molecule
}

void OMSimIBD::initialiseParticle()
{
    G4ParticleDefinition *lParticle = G4ParticleTable::GetParticleTable()->FindParticle("e+");
    mParticleGun->SetParticleDefinition(lParticle);
    G4ThreeVector lParticlePosition = mSNToolBox.randomPosition();
    mParticleGun->SetParticlePosition(lParticlePosition);
}

/**
 * @brief Computes the angular distribution of the reaction \( \bar{\nu}_e + p \rightarrow e^+ + n \).
 *
 * This function evaluates the angular distribution of the positron based on the energy of the incident electronic antineutrino.
 * The calculation follows the theoretical approach outlined in "The angular distribution of the reaction \( \bar{\nu}_e + p \rightarrow e^+ + n \)"
 * by P. Vogel and J. F. Beacom (1999), specifically referencing Eq. 14.
 *
 * @param Enubar Energy of the incoming electronic antineutrino.
 * @note This function populates the lAngularDistribution_x and lAngularDistribution_y vectors, which later can be used in other parts of the simulation.
 */
DistributionSampler OMSimIBD::angularDistribution(G4double Enubar)
{
    std::vector<G4double> lAngularDistribution_x, lAngularDistribution_y;
    const int lAngularDistributionSize = 300; // probably more than enough
    const G4double costhetac = 0.974;
    const G4double consf = 1.;
    const G4double consf2 = 3.706;

    G4double sigma0 = pow(mGf, 2) * pow(costhetac, 2) / pi * 1.024 * pow(197.326e-15, 2);

    lAngularDistribution_x.resize(lAngularDistributionSize);
    lAngularDistribution_y.resize(lAngularDistributionSize);

    G4double min = -1.;
    G4double max = 1.;
    G4double delta = (max - min) / G4double(lAngularDistributionSize - 1);
    for (G4int i = 0; i < lAngularDistributionSize; i++)
    {
        lAngularDistribution_x[i] = min + i * delta; // costheta
    }

    G4double Ee0 = Enubar - mDeltaMass;

    G4double pe0 = pow((pow(Ee0, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
    G4double ve0 = pe0 / Ee0;

    for (G4int j = 0; j < lAngularDistributionSize; j++)
    {
        G4double Ee1 = Ee0 * (1. - Enubar / proton_mass_c2 * (1. - ve0 * lAngularDistribution_x[j])) - mMassSquaredDifference / proton_mass_c2;
        G4double pe1 = pow((pow(Ee1, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
        G4double ve1 = pe1 / Ee1;
        G4double part1 = 2. * (consf + consf2) * mConsg * ((2. * Ee0 + mDeltaMass) * (1. - ve0 * lAngularDistribution_x[j]) - pow(electron_mass_c2, 2) / Ee0);
        G4double part2 = (pow(consf, 2) + pow(mConsg, 2)) * (mDeltaMass * (1. + ve0 * lAngularDistribution_x[j]) + pow(electron_mass_c2, 2) / Ee0);
        G4double part3 = (pow(consf, 2) + 3. * pow(mConsg, 2)) * ((Ee0 + mDeltaMass) * (1. - lAngularDistribution_x[j] / ve0) - mDeltaMass);
        G4double part4 = (pow(consf, 2) + pow(mConsg, 2)) * ((Ee0 + mDeltaMass) * (1. - lAngularDistribution_x[j] / ve0) - mDeltaMass) * ve0 * lAngularDistribution_x[j];
        G4double Tau = part1 + part2 + part3 + part4;

        lAngularDistribution_y[j] = sigma0 / 2. * ((pow(consf, 2) + 3. * pow(mConsg, 2)) + (pow(consf, 2) - pow(mConsg, 2)) * ve1 * lAngularDistribution_x[j]) * Ee1 * pe1 - sigma0 / 2. * (Tau / proton_mass_c2) * Ee0 * pe0; // dsigma/dcos(theta)
    }
    DistributionSampler lAngularDistribution;
    lAngularDistribution.setData(lAngularDistribution_x, lAngularDistribution_y, "AngularDistribution");
    lAngularDistribution.setUnits(1,1);
    return lAngularDistribution;
}

/**
 * @brief Computes the energy of a positron resulting from the inverse beta decay.
 *
 * Given the energy of the incident electronic antineutrino and the scatter angle, this function calculates the
 * energy of the emitted positron following inverse beta decay. The theoretical basis for this calculation is
 * given in "The angular distribution of the reaction \( \nu_e + p \rightarrow e^+ + n \)" by P. Vogel and J. F. Beacom (1999), specifically referencing Eq. 13.
 *
 * @param pEnubar Energy of the incoming electronic antineutrino.
 * @param pCostheta Cosine of the scatter angle between the direction of the antineutrino's momentum and the positron's momentum.
 *
 * @return Energy of the emitted positron as a result of the inverse beta decay process.
 * @reference P. Vogel, J. F. Beacom. (1999). The angular distribution of the reaction \( \nu_e + p \rightarrow e^+ + n \). Phys.Rev., D60, 053003
 */
G4double OMSimIBD::calculateSecondaryParticleEnergy(G4double pEnubar, G4double pCostheta)
{
    G4double lEe0 = pEnubar - mDeltaMass;
    G4double lPe0 = pow((pow(lEe0, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
    G4double lVe0 = lPe0 / lEe0;
    G4double lEnergy = lEe0 * (1. - pEnubar / proton_mass_c2 * (1. - lVe0 * pCostheta)) - mMassSquaredDifference / proton_mass_c2;

    return lEnergy;
}

/**
 * @brief Calculates the total cross-section of the inverse beta decay reaction for a given energy.
 *
 * This function estimates the total cross-section of the inverse beta decay, which can be used
 * to weigh each event. The theoretical basis for this calculation is presented in "Future detection
 * of supernova neutrino burst and explosion mechanism"
 * by T. Totani, K. Sato, H. E. Dalhed, and J. R. Wilson (1998), specifically referencing Equation 9.
 *
 * @param pEnergy Energy of the incoming electronic antineutrino.
 *
 * @return Total cross-section for the given energy.
 * @reference T. Totani, K. Sato, H. E. Dalhed, J. R. Wilson, "Future detection of supernova neutrino burst
 * and explosion mechanism", Astrophys. J. 496, 1998, 216–225, Preprint astro-ph/9710203, 1998.
 */
G4double OMSimIBD::totalCrossSection(G4double pEnergy)
{
    G4double lHbar = 6.58211899e-16 * 1e-6 * MeV * s;
    G4double lC = 3e8 * m / s;
    G4double lSigma0 = pow(2. * mGf * electron_mass_c2 * lHbar * lC, 2) / pi;
    G4double lConstante = -0.00325 / (MeV * MeV);
    G4double lDeltaWM = lConstante * (pEnergy - (neutron_mass_c2 - proton_mass_c2) / 2.);
    G4double lEe = pEnergy + proton_mass_c2 - neutron_mass_c2;
    G4double lPec = pow((pow(lEe, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
    G4double lSigma = 1. / 4. * lSigma0 * (1. + 3. * pow(mConsg, 2)) * (1. + lDeltaWM) * lEe * lPec / pow(electron_mass_c2, 2);
    return lSigma;
}


