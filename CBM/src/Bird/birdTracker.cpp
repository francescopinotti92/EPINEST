//
//  birdTracker.cpp
//  CBM
//
//

#include "birdTracker.hpp"

BirdMovements::BirdMovements() {
    Mcount = 0;
    Vcount = 0;
    RepurposeCount = 0;
}

std::string BirdMovements::formatData() {
    return fmt::format( "{}-{}-{}", Mcount, Vcount, RepurposeCount );
}
