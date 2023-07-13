//
//  bird_urn.hpp
//  CBM
//
//

#ifndef bird_urn_hpp
#define bird_urn_hpp

#include "bird.hpp"
#include "types.hpp"
#include "constants.hpp"

#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/*
 Contains sparse index of birds by species.
 Follows a FIFO structure, i.e. allows to retrieve chickens in the same
 order as they are added to this container.
 
 Careful:
 - does not check duplicate indexes.
 - returns an INVALID value if empty
 */

class BirdUrn {
public:
    BirdUrn();
    void insertBird( const uint& index );
    void insertBird( const Bird& bird );
    void insertBird( const pBird& bird );
    uint pop_front();
    void markInvalidBird( const uint& index );
    void markInvalidBird( const Bird& bird  ) { markInvalidBird( bird.index ); }
    void markInvalidBird( const pBird& bird ) { markInvalidBird( bird->index ); }

    //void removeBird( const Bird& bird );
    //void removeBird( const pBird& bird );
    void clear();
    uint size() { return _size; }
    
    const static uint INVALID = std::numeric_limits<uint>::max();

private:
    std::deque<uint> _indexPool;
    uint _size;
};

struct BirdUrnMngr {
    
    //====== Constructor ======//
    BirdUrnMngr( const std::vector<BirdType>& birdTypes );
    
    BirdUrn& getBirdUrn( const BirdType& bt, const uint& marketIndex ) { return urnList[bt][marketIndex] ; }
    
   
    //== iterators

    
    void clear();
    void clear( const BirdType& birdType );
    
    uint size( const BirdType& birdType );
    uint size( const BirdType& birdType, const uint& marketIndex );

    std::vector<uint>* getNonEmptyUrnIds( const BirdType& bt );

    std::unordered_map<BirdType, std::unordered_map<uint, BirdUrn > > urnList;

};



#endif /* bird_urn_hpp */
