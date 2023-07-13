//
//  bird_counter.hpp
//  CBM_ECS
//
//

#ifndef bird_counter_h
#define bird_counter_h

#include <unordered_map>
#include "types.hpp"
#include "constants.hpp"

/*
 This is just counting how many birds per each type are stored in a place
 */
struct BirdCounter {
    uint mcounter[constants::nBirdTypes];
    
    BirdCounter() {
        for (uint i = 0; i < constants::nBirdTypes; i++)
            mcounter[i] = 0;
    };

    uint& operator[](const BirdType& birdType) {
        return mcounter[static_cast<UnderlyingBirdType>(birdType)];
    }
    
};

#endif /* bird_counter_h */
