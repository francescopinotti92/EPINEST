//
//  holdsbirds.cpp
//  CBM
//
//

#include <stdio.h>

#include "holdsbirds.hpp"

//====== Constructors ======//

HoldsBirds::HoldsBirds(): baseWeight( 1. ) {};

HoldsBirds::HoldsBirds(const std::vector<BirdType>& birdTypes_, const uint& initialCapacityBirds_ ): birdTypes(birdTypes_), birds(initialCapacityBirds_), contEnv(nullptr), baseWeight( 1. ) {};

//====== Virtual functions ======//

// Inserts single bird and returns reference to it
Bird& HoldsBirds::insertBird(pBird&& bird) {
    bird->owner = this;
    return birds.insert(std::move(bird));
}

// Non-virtual functions
void HoldsBirds::swapToInfected(const uint& index) {
    birds.swapToInfected(index);
}
void HoldsBirds::swapToSusceptible(const uint& index) {
    birds.swapToSusceptible(index);
}


// check if trades a specific type of birdss
/*
bool HoldsBirds::trades(const BirdType& bt) {
    if ( std::find( birdTypes.begin(), birdTypes.end(), bt ) != birdTypes.end() )
        return true;
    else
        return false;
}
 */

// get a random bird
pBird& HoldsBirds::getRandomBird(Random &randGen) {
    return birds.getBirdDense( randGen.getUniInt( birds.size() - 1 ) );
}

/*
std::vector<BirdType>& HoldsBirds::getTradedBirdTypes() const {
    return birdTypes;
}*/

