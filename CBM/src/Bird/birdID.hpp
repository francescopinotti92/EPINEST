//
//  birdID.hpp
//  CBM
//
//

#ifndef birdID_h
#define birdID_h

#include "types.hpp"
#include <string>

/*
 Unique ID for each bird.
 Each bird is characterized by:
 1) The ID of the farm that instantiated it
 2) An additional integer ID
 */

struct BirdID {
    BirdID(uint idFarm_ = std::numeric_limits<uint>::max(), uint idBird_ = std::numeric_limits<uint>::max() ): idFarm(idFarm_), idBird(idBird_) {};
    std::string toString() const;
    uint idFarm;
    uint idBird;
};

bool operator==(const BirdID& lhs, const BirdID& rhs);

struct hashBirdID {
    
    std::size_t operator()( const BirdID& id ) const;
            
};


#endif /* birdID_h */
