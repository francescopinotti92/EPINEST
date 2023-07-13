//
//  ChickenPool.hpp
//  CBM
//
//

#ifndef ChickenPool_hpp
#define ChickenPool_hpp

#include "ElementPool.hpp"
#include "bird.hpp"
#include "constants.hpp"
#include "types.hpp"


/*
 This class acts as a pool of bird pointers; these birds are then recycled throughout
 a simulation and between simulations.
 Birds must be returned to this pool between simulations.
 */
class ChickenPool {
public:
    ChickenPool( const uint& capacity );
    pBird request_bird();
    void return_bird( pBird&& bird );
private:
    pBird make_dummy_bird();
    ElementPool<pBird> birds;
};

#endif /* ChickenPool_hpp */
