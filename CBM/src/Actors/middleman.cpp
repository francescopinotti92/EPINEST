//
//  middleman.cpp
//  CBM
//
//

#include <stdio.h>

#include "middleman.hpp"

//====== Constructor ======//

Middleman::Middleman(const uint& id_, const std::vector<BirdType>& birdTypes_, const uint& initialCapacityBirds ): id( id_ ), HoldsBirds( birdTypes_, initialCapacityBirds ), birdUrnMngr( birdTypes_ ) {
    
    maxCapacity = initialCapacityBirds;
};

//====== Middleman specific functions ======//

void Middleman::reset( ChickenPool* birdMngr ) {
    birds.clear( birdMngr );
    birdUrnMngr.clear();
    contEnv->clear();
}

uint Middleman::getBirdSizeType(const BirdType& bt) {
    return birdUrnMngr.size( bt );
}

uint Middleman::getBirdSizeType( const BirdType& bt, const uint& marketId ) {
    return birdUrnMngr.size( bt, marketId );
}
  
//====== HoldsBirds functions ======//

Bird& Middleman::insertBird( pBird&& bird ) {
    bird->owner = this;
    Bird& bird_tmp = birds.insert( std::move( bird ) );
    //birdUrn.insertBird( bird_tmp );
    return bird_tmp;
}

pBird Middleman::removeBird( const uint& index ) {
    pBird bird = birds.remove( index );
    //auto& urn = birdUrnMngr.getBirdUrn( bird->birdType, bird->marketDestination );
    //urn.markInvalidBird( index );
    return bird;
}

pBird Middleman::removeDeadBird( const uint& index ) {
    pBird bird = birds.remove( index );
    //birdUrn.removeBird( bird );
    auto& urn = birdUrnMngr.getBirdUrn( bird->birdType, bird->marketDestination );
    urn.markInvalidBird( index );
    return bird;
}

void Middleman::notifySuccessfulExposure( Bird& bird ) {};

std::string Middleman::getStringID() {
    std::string res = "MM";
    res += std::to_string( id );
    return res;
}

NodeType Middleman::getSettingType() {
    return NodeType::middleman;
}

