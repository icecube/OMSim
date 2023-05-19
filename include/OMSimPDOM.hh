#ifndef OMSimPDOM_h
#define OMSimPDOM_h 1
#include "abcDetectorComponent.hh"
#include "OMSimPMTConstruction.hh"

class pDOM : public abcDetectorComponent
    {
    public:
        pDOM(InputDataManager* pData, G4bool pPlaceHarness = true);
        ~pDOM();
        void construction();
        G4bool mPlaceHarness;

    private:
        OMSimPMTConstruction *mPMTManager;
        
    };

#endif