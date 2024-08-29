#pragma once
#include <G4VUserPhysicsList.hh>

class G4VPhysicsConstructor;
class G4ProductionCuts;

/**
 * @class OMSimPhysicsList
 * @brief Custom physics list for optical photon processes.
 *
 * This physics list initializes the optical photon processes including 
 * absorption, boundary interactions, Rayleigh scattering, and Mie scattering.
 *
 * @ingroup EffectiveArea
 */
class OMSimPhysicsList: public G4VUserPhysicsList
{
	public:
		OMSimPhysicsList();
		~OMSimPhysicsList(){};

	protected:
		void ConstructParticle();
		void ConstructProcess();
		void SetCuts();

	private:

};
