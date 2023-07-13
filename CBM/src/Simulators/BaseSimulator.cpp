//
//  BaseSimulator.cpp
//  CBM
//
//

#include "BaseSimulator.hpp"

// constructor
BaseSimulator::BaseSimulator() {
    clock = Clock();
    placeVec  =         nullptr;
    compModel =         nullptr;
    tracker   =         nullptr;
}

// counts total number of birds
uint BaseSimulator::getTotalNumberBirds() {
    uint nBirds = 0;
    for ( auto& place: *placeVec ) {
        nBirds += place->getSizeBirds();
    }
    return nBirds;
}

// counts total number of infected birds
uint BaseSimulator::getTotalNumberInfectedBirds() {
    uint nBirds = 0;
    for ( auto& place: *placeVec ) {
        nBirds += place->getSizeInfectedBirds();
    }
    return nBirds;
}
