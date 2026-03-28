/**
 * @file OMSimEventAction.hh
 * @brief Event action for WavePID simulation.
 */
#pragma once

#include "G4UserEventAction.hh"
#include <string>
#include "G4Types.hh"

class G4Event;

class OMSimEventAction : public G4UserEventAction
{
public:
    OMSimEventAction() = default;
    ~OMSimEventAction() = default;

    void BeginOfEventAction(const G4Event* event) override;
    void EndOfEventAction(const G4Event* event) override;
};
