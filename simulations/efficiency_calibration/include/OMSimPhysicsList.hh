#pragma once
#include <G4VUserPhysicsList.hh>

class G4VPhysicsConstructor;
class G4ProductionCuts;

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
