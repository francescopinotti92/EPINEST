//
//  crop.hpp
//  CBM
//
//

#ifndef crop_hpp
#define crop_hpp

#include "types.hpp"
#include "ElementPool.hpp"
#include "clock.hpp"
#include <stdio.h>


struct Crop {
    
    Crop( const uint& size );
    Crop( const uint& size, const Clock& clock );
    void insert( const uint& id );
    void remove( const uint& id, const bool& dead = false, const bool& forced = false );
    bool empty();
    uint size();
    uint removeLast( const bool& dead = false, const bool& forced = false );
    uint dayCreate;
    uint tCreate;
    uint nBirdsInitial;         // initial crop size
    uint nInfectionsCumulative; // number cumulative infections
    uint nBirdsDead;            // number dead birds
    uint nBirdsForceRemove;     // number birds removed forcefully
    ElementPool<uint> pool;
    
};

#endif /* crop_hpp */
