//
//  vendor.cpp
//  CBM
//
//

#include "vendor.hpp"

//=== Constructor ===//

Vendor::Vendor(const uint& id_,
               const uint& initialCapacityBirds,
               const std::vector<BirdType>& birdTypes_):
               HoldsBirds( birdTypes_, initialCapacityBirds ), id(id_) {
                   newBirdsMarketIndex.clear();
                   newBirdsMarketIndex.reserve( initialCapacityBirds );
                   repurposedBirdsMarketIndex.clear();
                   repurposedBirdsMarketIndex.reserve( initialCapacityBirds );
};


//=== HoldsBirds functions ===//

Bird& Vendor::insertBird(pBird&& bird) {
    bird->owner = this;
    return birds.insert( std::move(bird) );
}

pBird Vendor::removeBird(const uint& index) {
    pBird bird = birds.remove(index);
    return bird;
}

pBird Vendor::removeDeadBird(const uint& index) {
    pBird bird = birds.remove(index);
    return bird;
}

void Vendor::notifySuccessfulExposure( Bird& bird ) {};


std::string Vendor::getStringID() {
    std::string res = "V";
    res += std::to_string( id );
    return res;
}

NodeType Vendor::getSettingType() {
    return NodeType::vendor;
}

//=== Vendor Specific functions ===//

void Vendor::reset( ChickenPool* birdMngr ) {
    newBirdsMarketIndex.clear(); // does not care where birds are found
    repurposedBirdsMarketIndex.clear();
    birds.clear( birdMngr );
    contEnv->clear();
}

// transfer birds from own stock to market
void Vendor::transferBirds( Market* market ) {
    
    if ( market == nullptr )
        return;
    
    if ( birds.empty() )
        return;
    
    uint idx;
    while ( birds.size() > 0 ) { // push all birds to market, one by one
        pBird birdTmp = birds.removeLast();
        birdTmp->movements.increaseRepurposeCount();
        idx = ( market->insertBird( std::move( birdTmp ) ) ).index;
        repurposedBirdsMarketIndex.push_back( idx ); // get bird index back
    }
}

// to be called when a vendor buys a new bird in markets
void Vendor::addNewBirdIndex( const uint &index ) {
    newBirdsMarketIndex.push_back( index );
}

bool Vendor::removeNewBirdIndex( const uint &index ) {
    for ( auto it = newBirdsMarketIndex.begin(), end = newBirdsMarketIndex.end(); it < end; ) {
        if (*it == index) {
            std::iter_swap( it, newBirdsMarketIndex.end() - 1 );
            newBirdsMarketIndex.pop_back();
            return true;
        }
        ++it;
    }
    return false;
}

bool Vendor::removeRepurposedBirdIndex( const uint &index ) {
    for ( auto it = repurposedBirdsMarketIndex.begin(), end = repurposedBirdsMarketIndex.end(); it < end; ) {
        if (*it == index) {
            std::iter_swap( it, repurposedBirdsMarketIndex.end() - 1 );
            repurposedBirdsMarketIndex.pop_back();
            return true;
        }
        ++it;
    }
    return false;
}

void Vendor::removeBirdIndex( const uint& index ) {
    
    // look for bird in repurposed bird index container first
    if ( removeRepurposedBirdIndex( index ) )
        return;
    else // check among new birds
        removeNewBirdIndex( index );
}

uint Vendor::popNewBirdIndex() {
    uint idx = newBirdsMarketIndex.back();
    newBirdsMarketIndex.pop_back();
    return idx;
}

uint Vendor::popRepurposedBirdIndex() {
    uint idx = repurposedBirdsMarketIndex.back();
    repurposedBirdsMarketIndex.pop_back();
    return idx;
}

// getters
std::vector<uint>& Vendor::getMarketedRepurposedBirdsIndexes()     {
    return repurposedBirdsMarketIndex;
}
std::vector<uint>& Vendor::getMarketedNewBirdsIndexes()     {
    return newBirdsMarketIndex;
}

uint Vendor::getSizeBirdsMarket() const {
    return static_cast<uint>( newBirdsMarketIndex.size() + repurposedBirdsMarketIndex.size() );
}

uint Vendor::getSizeRepurposedBirdsMarket() const {
    return static_cast<uint>( repurposedBirdsMarketIndex.size() );
}

uint Vendor::getSizeNewBirdsMarket() const {
    return static_cast<uint>( newBirdsMarketIndex.size() );
}


uint Vendor::getId() {
    return id;
}
