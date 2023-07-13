//
//  birdTracker.hpp
//  CBM
//
//

#ifndef birdTracker_hpp
#define birdTracker_hpp

#include "types.hpp"

// this object tracks several stats about number of consecutive mark-ups
struct BirdMovements {
    BirdMovements();
    std::string formatData();
    inline void reset() {
        
        Mcount = 0 ;
        Vcount = 0 ;
        RepurposeCount = 0 ;
        
    }
    inline void increaseMcount() {
        Mcount++;
    };
    inline void increaseVcount() {
        Vcount++;
    }
    inline void increaseRepurposeCount() {
        RepurposeCount++;
    }
    unsigned short Mcount;
    unsigned short Vcount;
    unsigned short RepurposeCount;
};



#endif /* birdTracker_hpp */
