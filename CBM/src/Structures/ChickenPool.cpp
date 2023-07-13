//
//  ChickenPool.cpp
//  CBM
//
//

#include "ChickenPool.hpp"

struct Bird;

ChickenPool::ChickenPool( const uint& capacity ): birds( capacity ) {};

pBird ChickenPool::request_bird() {
    
    if ( !birds.empty() ) {
        
        pBird bird = birds.removeLast();
        return bird;
        
    }
    
    return make_dummy_bird();
    
}

void ChickenPool::return_bird( pBird&& bird ) {
    
    bird->owner = nullptr; // this is sufficient to make bird invalid
    
    if ( birds.isFull() )
        birds.addCapacity( 100000 ); // increase size by fixed amount
    
    birds.insert( std::move( bird ) );
    
}


pBird ChickenPool::make_dummy_bird() {
    
    pBird bird = std::make_shared<Bird>( std::numeric_limits<uint>::max(), BirdType::null, BirdID( std::numeric_limits<uint>::max(), std::numeric_limits<uint>::max() ) , std::numeric_limits<uint>::max(), 0., nullptr, nullptr  );
    
    return bird;
    
}
