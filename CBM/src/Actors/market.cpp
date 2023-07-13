//
//  market.cpp
//  CBM
//
//

#include "market.hpp"

//=== Constructor ===//

Market::Market(const uint& id_, const std::vector<BirdType>& birdTypes_, const uint& initialCapacityBirds ): id(id_), HoldsBirds( birdTypes_, initialCapacityBirds ) {
        
    //vendorList.clear();
    //vendorList.reserve(initialCapacityVendors);
    
}

//=== HoldsBirds specific functions ===//

Bird& Market::insertBird( pBird&& bird ) {
    bird->owner = this;
    return birds.insert(std::move(bird));
}

pBird Market::removeBird( const uint& index ) {
    pBird bird = birds.remove( index );
    return bird;
}

pBird Market::removeDeadBird(const uint& index) {
    pBird bird = birds.remove(index);
    bird->vendorPtr->removeBirdIndex(bird->index); // remove corresponding index from vendor
    return bird;
}

void Market::notifySuccessfulExposure( Bird& bird ) {};

std::string Market::getStringID() {
    std::string res = "M";
    res += std::to_string( id );
    return res;
}

NodeType Market::getSettingType() {
    return NodeType::market;
}

// market-specialized inserter
// takes care of updating a bird's vendorPtr field and notifies vendor
Bird& Market::insertNewBird(pBird&& bird, Vendor* vendor) {
    bird->owner = this;
    bird->vendorPtr = vendor;
    Bird& birdRef = birds.insert( std::move(bird) );
    vendor->addNewBirdIndex( birdRef.index );
    return birdRef;
}

void Market::reset( ChickenPool* birdMngr ) {
    birds.clear( birdMngr );
    contEnv->clear();
}
