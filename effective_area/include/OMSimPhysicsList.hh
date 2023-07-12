#ifndef OMSimPhysicsList_h
#define OMSimPhysicsList_h 1

#include "G4VUserPhysicsList.hh"
#include "globals.hh"

class G4VPhysicsConstructor;
class G4ProductionCuts;

class OMSimPhysicsList: public G4VUserPhysicsList
{
	public:
		OMSimPhysicsList();
		~OMSimPhysicsList();

	protected:
		void ConstructParticle();
		void ConstructProcess();
		void SetCuts();

	private:

};
#endif
