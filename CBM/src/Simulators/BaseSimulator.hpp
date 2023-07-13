//
//  BaseSimulator.hpp
//  CBM
//
//

#ifndef BaseSimulator_hpp
#define BaseSimulator_hpp

#include <stdio.h>
#include "epidemic_dynamics.hpp"
#include "basecompartmentalmodel.hpp"
#include "birdTransferManager.hpp"
#include "base_infection_tracer.hpp"
#include "baseNode.hpp"
#include "holdsbirds.hpp"
#include "clock.hpp"
#include "types.hpp"
#include <vector>

class BaseSimulator {
public:
    BaseSimulator();
    virtual ~BaseSimulator() = default;
        
    // general output functions
    uint getTotalNumberBirds();
    uint getTotalNumberInfectedBirds();

    // setters
    virtual void setEpiModel( BaseCompartmentalModel* compModel_ )          { compModel = compModel_; }
    
    // getters
    std::vector<BaseNode*>& getPlaces()                             { return *placeVec; }
    BaseCompartmentalModel& getCompModel()                          { return *compModel; }
    Clock& getClock()                                               { return clock; }
    
    
protected:
    Clock clock;
    BaseCompartmentalModel* compModel;
    BaseInfectionTracker* tracker;
    std::vector<BaseNode*>* placeVec;
    
};

#endif /* BaseSimulator_hpp */
