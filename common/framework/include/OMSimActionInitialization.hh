#pragma once
#include "G4VUserActionInitialization.hh"

class OMSimActionInitialization : public G4VUserActionInitialization
{
public:
    OMSimActionInitialization();
    virtual ~OMSimActionInitialization();

    virtual void BuildForMaster() const;
    virtual void Build() const;
};
