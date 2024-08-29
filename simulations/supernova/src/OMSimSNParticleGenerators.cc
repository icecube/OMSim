#include "OMSimPrimaryGeneratorAction.hh"
#include "OMSimSNAnalysis.hh"
#include "OMSimCommandArgsTable.hh"

#include <G4ParticleGun.hh>
#include <G4ParticleTable.hh>
#include <OMSimTools.hh>

void setupDistribution(DistributionSampler &p_sampler,
                       const std::vector<G4double> &p_x,
                       const std::vector<G4double> &p_y,
                       const G4String &p_name,
                       G4double p_unit_x,
                       G4double p_unit_y)
{
    p_sampler.setData(p_x, p_y, p_name);
    p_sampler.setUnits(p_unit_x, p_unit_y);
    p_sampler.makeInterpolator();
}

/*
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 *                                     Base Class
 * %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
 */

SNBaseParticleGenerator::SNBaseParticleGenerator(G4ParticleGun *p_gun)
    : m_particleGun(p_gun)
{
}

void SNBaseParticleGenerator::initialiseDistribution(int p_columnIndex, int p_NrOfTargets)
{

    if (OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy") == false)
    {
        std::pair<std::string, std::string> fluxNames = m_SNToolBox.getFileNames(OMSimCommandArgsTable::getInstance().get<G4int>("SNtype"));
        std::string fluxName;
        if (p_columnIndex == 1)
        {
            fluxName = fluxNames.first;
        }
        else
        {
            fluxName = fluxNames.second;
        }

        std::vector<std::vector<double>> data = Tools::loadtxt(fluxName, true, 0, '\t');
        setupDistribution(m_timeDistribution, data[0], data[1], "SNLuminosityVsTime", 1 * s, 1);
        setupDistribution(m_meanEnergyDistribution, data[0], data[2], "SNMeanEnergyVsTime", 1 * s, 1 * MeV);
        setupDistribution(m_meanEnergySquaredDistribution, data[0], data[3], "SNMeanEnergySquaredVsTime", 1 * s, 1 * MeV * MeV);
    }
    m_NrTargets = m_SNToolBox.numberOfTargets(p_NrOfTargets);
}

void SNBaseParticleGenerator::GeneratePrimaries(G4Event *p_event)
{
    initialiseParticle();
    OMSimSNAnalysis &analysisManager = OMSimSNAnalysis::getInstance();
    OMSimSNAnalysis::getInstance().initEventStat();
    G4double neutrinoEnergy = calculateNeutrinoEnergy();

    G4ThreeVector momentumDirection = calculateMomentumDirection(neutrinoEnergy);
    m_particleGun->SetParticleMomentumDirection(momentumDirection);

    G4double particleEnergy = calculateSecondaryParticleEnergy(neutrinoEnergy, momentumDirection.getZ());
    m_particleGun->SetParticleEnergy(particleEnergy);

    analysisManager.m_eventStat->weight = calculateWeight(neutrinoEnergy);
    analysisManager.m_eventStat->primaryEnergy = particleEnergy;

    m_particleGun->GeneratePrimaryVertex(p_event);
}

G4ThreeVector SNBaseParticleGenerator::calculateMomentumDirection(G4double p_NuEnergy)
{
    G4double cosTheta = angularDistribution(p_NuEnergy).sampleFromDistribution();
    G4double sinTheta = std::sqrt(1. - cosTheta * cosTheta);
    G4double phi = twopi * G4UniformRand();
    G4double zdir = -cosTheta;
    G4double xdir = -sinTheta * std::cos(phi);
    G4double ydir = -sinTheta * std::sin(phi);
    OMSimSNAnalysis::getInstance().m_eventStat->cosTheta = cosTheta;
    return G4ThreeVector(xdir, ydir, zdir);
}

G4double SNBaseParticleGenerator::calculateWeight(G4double p_NuEnergy)
{
    G4double sigma = totalCrossSection(p_NuEnergy);
    return m_SNToolBox.calculateWeight(sigma, m_NrTargets);
}

G4double SNBaseParticleGenerator::calculateNeutrinoEnergy()
{
    G4double thresholdEnergy, timeOfSpectrum, meanEnergy, meanEnergySquared, alpha, neutrinoEnergy;
    thresholdEnergy = neutron_mass_c2 + electron_mass_c2 - proton_mass_c2 + 0.1 * MeV;
    bool retry = true;
    while (retry)
    {
        retry = false;
        timeOfSpectrum = -99.;
        meanEnergy = -99.;
        meanEnergySquared = -99.;
        alpha = -99.;
        neutrinoEnergy = -99.;

        if (!OMSimCommandArgsTable::getInstance().get<bool>("SNfixEnergy"))
        {
            timeOfSpectrum = m_timeDistribution.sampleFromDistribution();
            meanEnergy = m_meanEnergyDistribution.interpolate(timeOfSpectrum);
            meanEnergySquared = m_meanEnergySquaredDistribution.interpolate(timeOfSpectrum);

            G4int count = 0;
            while (neutrinoEnergy <= thresholdEnergy)
            {
                neutrinoEnergy = m_SNToolBox.sampleEnergy(meanEnergy, meanEnergySquared, alpha);
                count += 1;
                if (count > 10)
                {
                    retry = true;
                    break;
                }
            }
        }
        else
        {
            timeOfSpectrum = 0.0;
            meanEnergy = OMSimCommandArgsTable::getInstance().get<G4double>("SNmeanE") * MeV;
            alpha = OMSimCommandArgsTable::getInstance().get<G4double>("SNalpha");
            meanEnergySquared = meanEnergy * meanEnergy * (2 + alpha) / (1 + alpha);
            neutrinoEnergy = m_SNToolBox.sampleEnergy(meanEnergy, meanEnergySquared, alpha);
            while (neutrinoEnergy <= thresholdEnergy)
            {
                neutrinoEnergy = m_SNToolBox.sampleEnergy(meanEnergy, meanEnergySquared, alpha);
            }
        }
    }

    OMSimSNAnalysis &analysisManager = OMSimSNAnalysis::getInstance();
    analysisManager.m_eventStat->neutrinoTime = timeOfSpectrum;
    analysisManager.m_eventStat->meanEnergy = meanEnergy;
    analysisManager.m_eventStat->neutrinoEnergy = neutrinoEnergy;
    return neutrinoEnergy;
}

/*
 * %%%%%%%%%%%%%%%% ENES %%%%%%%%%%%%%%%%
 */

/**
 * @brief Constructor of OMSimENES class
 * @param p_gun Pointer to the particle p_gun object used for simulation.
 */
OMSimENES::OMSimENES(G4ParticleGun *p_gun)
    : SNBaseParticleGenerator(p_gun)
{
    initialiseDistribution(1, 10); // 10 electrons per molecule
}

void OMSimENES::initialiseParticle()
{
    G4ParticleDefinition *particle = G4ParticleTable::GetParticleTable()->FindParticle("e-");
    m_particleGun->SetParticleDefinition(particle);
    G4ThreeVector lParticlePosition = m_SNToolBox.randomPosition();
    m_particleGun->SetParticlePosition(lParticlePosition);
}

/**
 * @brief Computes the angular distribution for electron scattering based on incident neutrino energy.
 *
 * This function calculates the angular distribution for electrons scattered by neutrinos. The foundation for this
 * computation is taken from the reference:
 * "Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press, Chapter 5, eq. 5.29".
 *
 * @param p_energy The energy of the incident neutrino.
 */
DistributionSampler OMSimENES::angularDistribution(G4double p_energy)
{
    const int angularDistributionSize = 500;
    const G4double g1 = 0.73;
    const G4double g2 = 0.23;
    std::vector<G4double> xAngularDistribution, yAngularDistribution;

    G4double sigma0 = 2. * pow(m_Gf, 2) * pow(electron_mass_c2, 2) / pi * pow(197.326e-15, 2);

    xAngularDistribution.resize(angularDistributionSize);
    yAngularDistribution.resize(angularDistributionSize);

    const double min = 0.;
    const double max = 1.;
    G4double delta = (max - min) / G4double(angularDistributionSize - 1);

    for (G4int i = 0; i < angularDistributionSize; i++)
    {
        xAngularDistribution[i] = min + i * delta; // costheta
    }

    for (G4int j = 0; j < angularDistributionSize; j++)
    {
        G4double dem = pow((pow((electron_mass_c2 + p_energy), 2) - pow(p_energy, 2) * pow(xAngularDistribution[j], 2)), 2);
        G4double factor1 = 4. * pow(p_energy, 2) * pow((electron_mass_c2 + p_energy), 2) * xAngularDistribution[j] / dem;
        G4double factor2 = 2. * electron_mass_c2 * p_energy * pow(xAngularDistribution[j], 2) / dem;
        G4double factor3 = 2. * pow(electron_mass_c2, 2) * pow(xAngularDistribution[j], 2) / dem;
        yAngularDistribution[j] = sigma0 * factor1 * (pow(g1, 2) + pow(g2, 2) * pow((1. - factor2), 2) - g1 * g2 * factor3); // dsigma/dcos(theta)
    }
    DistributionSampler angularDistribution;
    angularDistribution.setData(xAngularDistribution, yAngularDistribution, "AngularDistribution");
    angularDistribution.setUnits(1,1);
    return angularDistribution;
}

/**
 * @brief Computes the electron energy from elastic scattering based on incident neutrino energy and scatter angle.
 *
 * The computation utilizes the formula for the energy of the electron as a function of incident neutrino energy
 * and the scatter angle, based on the formula provided in the reference:
 * "Carlo Giunti and Chung W.Kim (2007), Fundamentals of Neutrino Physics and Astrophysics, Oxford University Press, Chapter 5, eq. 5.27".
 *
 * @param p_NuEnergy The energy of the incident neutrino.
 * @param p_cosTheta The cosine of the scattering angle.
 * @return Calculated energy value of the electron post scattering.
 */
G4double OMSimENES::calculateSecondaryParticleEnergy(G4double p_NuEnergy, G4double p_cosTheta)
{
    G4double energy = 2 * electron_mass_c2 * pow(p_NuEnergy, 2) * pow(p_cosTheta, 2) / (pow((electron_mass_c2 + p_NuEnergy), 2) - pow(p_NuEnergy, 2) * pow(p_cosTheta, 2));
    return energy;
}

/**
 * @brief Computes the total cross section for ENES based on incident neutrino energy.
 *
 * This function determines the total cross-section for the scattering of neutrinos with electrons given a specific
 * incident neutrino energy. The computational foundation is derived from:
 * "M. Buchkremer, Electroweak Interactions: Neutral currents in neutrino-lepton elastic scattering experiments,
 * Université Catholique de Louvain /CP3, 2011."
 *
 * @param p_energy The energy of the incident neutrino.
 * @return Calculated total cross-section value for the given energy.
 */
G4double OMSimENES::totalCrossSection(G4double p_energy)
{
    double sin2ThetaW = 0.231;
    G4double sigma = pow(m_Gf, 2) * electron_mass_c2 * p_energy / (2. * pi) * (1. + 4. * sin2ThetaW + 16. / 3. * pow(sin2ThetaW, 2)) * pow(197.326e-15, 2) * m * m;
    return sigma;
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
 * @param p_gun Pointer to the particle p_gun object used for simulation.
 */
OMSimIBD::OMSimIBD(G4ParticleGun *p_gun)
    : SNBaseParticleGenerator(p_gun)
{
    initialiseDistribution(2, 2); // 2 protons (hydrogen) per molecule
}

void OMSimIBD::initialiseParticle()
{
    G4ParticleDefinition *particle = G4ParticleTable::GetParticleTable()->FindParticle("e+");
    m_particleGun->SetParticleDefinition(particle);
    G4ThreeVector lParticlePosition = m_SNToolBox.randomPosition();
    m_particleGun->SetParticlePosition(lParticlePosition);
}

/**
 * @brief Computes the angular distribution of the reaction \( \bar{\nu}_e + p \rightarrow e^+ + n \).
 *
 * This function evaluates the angular distribution of the positron based on the energy of the incident electronic antineutrino.
 * The calculation follows the theoretical approach outlined in "The angular distribution of the reaction \( \bar{\nu}_e + p \rightarrow e^+ + n \)"
 * by P. Vogel and J. F. Beacom (1999), specifically referencing Eq. 14.
 *
 * @param p_energy Energy of the incoming electronic antineutrino.
 * @note This function populates the xAngularDistribution and yAngularDistribution vectors, which later can be used in other parts of the simulation.
 */
DistributionSampler OMSimIBD::angularDistribution(G4double p_energy)
{
    std::vector<G4double> xAngularDistribution, yAngularDistribution;
    const int angularDistributionSize = 300; // probably more than enough
    const G4double costhetac = 0.974;
    const G4double consf = 1.;
    const G4double consf2 = 3.706;

    G4double sigma0 = pow(m_Gf, 2) * pow(costhetac, 2) / pi * 1.024 * pow(197.326e-15, 2);

    xAngularDistribution.resize(angularDistributionSize);
    yAngularDistribution.resize(angularDistributionSize);

    G4double min = -1.;
    G4double max = 1.;
    G4double delta = (max - min) / G4double(angularDistributionSize - 1);
    for (G4int i = 0; i < angularDistributionSize; i++)
    {
        xAngularDistribution[i] = min + i * delta; // costheta
    }

    G4double Ee0 = p_energy - m_deltaMass;

    G4double electronMomentum0 = pow((pow(Ee0, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
    G4double ve0 = electronMomentum0 / Ee0;

    for (G4int j = 0; j < angularDistributionSize; j++)
    {
        G4double Ee1 = Ee0 * (1. - p_energy / proton_mass_c2 * (1. - ve0 * xAngularDistribution[j])) - m_massSquaredDifference / proton_mass_c2;
        G4double pe1 = pow((pow(Ee1, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
        G4double ve1 = pe1 / Ee1;
        G4double part1 = 2. * (consf + consf2) * m_Consg * ((2. * Ee0 + m_deltaMass) * (1. - ve0 * xAngularDistribution[j]) - pow(electron_mass_c2, 2) / Ee0);
        G4double part2 = (pow(consf, 2) + pow(m_Consg, 2)) * (m_deltaMass * (1. + ve0 * xAngularDistribution[j]) + pow(electron_mass_c2, 2) / Ee0);
        G4double part3 = (pow(consf, 2) + 3. * pow(m_Consg, 2)) * ((Ee0 + m_deltaMass) * (1. - xAngularDistribution[j] / ve0) - m_deltaMass);
        G4double part4 = (pow(consf, 2) + pow(m_Consg, 2)) * ((Ee0 + m_deltaMass) * (1. - xAngularDistribution[j] / ve0) - m_deltaMass) * ve0 * xAngularDistribution[j];
        G4double tau = part1 + part2 + part3 + part4;

        yAngularDistribution[j] = sigma0 / 2. * ((pow(consf, 2) + 3. * pow(m_Consg, 2)) + (pow(consf, 2) - pow(m_Consg, 2)) * ve1 * xAngularDistribution[j]) * Ee1 * pe1 - sigma0 / 2. * (tau / proton_mass_c2) * Ee0 * electronMomentum0; // dsigma/dcos(theta)
    }
    DistributionSampler angularDistribution;
    angularDistribution.setData(xAngularDistribution, yAngularDistribution, "AngularDistribution");
    angularDistribution.setUnits(1,1);
    return angularDistribution;
}

/**
 * @brief Computes the energy of a positron resulting from the inverse beta decay.
 *
 * Given the energy of the incident electronic antineutrino and the scatter angle, this function calculates the
 * energy of the emitted positron following inverse beta decay. The theoretical basis for this calculation is
 * given in "The angular distribution of the reaction \( \nu_e + p \rightarrow e^+ + n \)" by P. Vogel and J. F. Beacom (1999), specifically referencing Eq. 13.
 *
 * @param p_energy Energy of the incoming electronic antineutrino.
 * @param p_cosTheta Cosine of the scatter angle between the direction of the antineutrino's momentum and the positron's momentum.
 *
 * @return Energy of the emitted positron as a result of the inverse beta decay process.
 * @reference P. Vogel, J. F. Beacom. (1999). The angular distribution of the reaction \( \nu_e + p \rightarrow e^+ + n \). Phys.Rev., D60, 053003
 */
G4double OMSimIBD::calculateSecondaryParticleEnergy(G4double p_energy, G4double p_cosTheta)
{
    G4double electronEnergy0 = p_energy - m_deltaMass;
    G4double electronMomentum0 = pow((pow(electronEnergy0, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
    G4double velocity0 = electronMomentum0 / electronEnergy0;
    G4double energy = electronEnergy0 * (1. - p_energy / proton_mass_c2 * (1. - velocity0 * p_cosTheta)) - m_massSquaredDifference / proton_mass_c2;

    return energy;
}

/**
 * @brief Calculates the total cross-section of the inverse beta decay reaction for a given energy.
 *
 * This function estimates the total cross-section of the inverse beta decay, which can be used
 * to weigh each event. The theoretical basis for this calculation is presented in "Future detection
 * of supernova neutrino burst and explosion mechanism"
 * by T. Totani, K. Sato, H. E. Dalhed, and J. R. Wilson (1998), specifically referencing Equation 9.
 *
 * @param p_energy Energy of the incoming electronic antineutrino.
 *
 * @return Total cross-section for the given energy.
 * @reference T. Totani, K. Sato, H. E. Dalhed, J. R. Wilson, "Future detection of supernova neutrino burst
 * and explosion mechanism", Astrophys. J. 496, 1998, 216–225, Preprint astro-ph/9710203, 1998.
 */
G4double OMSimIBD::totalCrossSection(G4double p_energy)
{
    G4double hbar = 6.58211899e-16 * 1e-6 * MeV * s;
    G4double lC = 2.99792458e8 * m / s;
    G4double sigma0 = pow(2. * m_Gf * electron_mass_c2 * hbar * lC, 2) / pi;
    G4double constant = -0.00325 / (MeV * MeV);
    G4double deltaWM = constant * (p_energy - (neutron_mass_c2 - proton_mass_c2) / 2.);
    G4double electronEnergy = p_energy + proton_mass_c2 - neutron_mass_c2;
    G4double momentume = pow((pow(electronEnergy, 2) - pow(electron_mass_c2, 2)), 1. / 2.);
    G4double sigma = 1. / 4. * sigma0 * (1. + 3. * pow(m_Consg, 2)) * (1. + deltaWM) * electronEnergy * momentume / pow(electron_mass_c2, 2);
    return sigma;
}


