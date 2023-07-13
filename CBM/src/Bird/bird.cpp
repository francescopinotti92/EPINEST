//
//  bird.cpp
//  CBM
//
//

#include <stdio.h>

#include "bird.hpp"

//=== Constructor ===//
Bird::Bird(const uint& index_, const BirdType& birdType_, const BirdIdType& id_, const uint& t_birth_, const double& hazard_, Crop* cropPtr_, HoldsBirds* owner_): index(index_), birdType(birdType_), id(id_), t_birth(t_birth_), t_market(0), hazard(hazard_), owner(owner_), cropPtr(cropPtr_), immState(0), infState(0), pendingInfections(0) {}; // creates a susceptible bird


void Bird::setBird( const uint& index_, const BirdType& birdType_, const BirdIdType& id_, const uint& t_birth_, const double& hazard_, Crop* cropPtr_, HoldsBirds* owner_ ) {
    
    index    = index_;
    birdType = birdType_;
    id       = id_;
    t_birth  = t_birth_;
    t_market = 0;
    hazard   = hazard_;
    cropPtr  = cropPtr_;
    owner    = owner_;
    immState = 0;
    infState = 0;
    pendingInfections = 0;
    movements.reset();

}
