//
//  fieldexperimenter.cpp
//  CBM
//
//

#include "fieldexperimenter.hpp"


//=== Constructor ===//

FieldExperimenter::FieldExperimenter( const BirdType& birdType_, const uint& initialCapacityBirds ): HoldsBirds( {birdType_}, initialCapacityBirds ) {
    
    birdsMarketIndex = {} ;
    experimentGroup = {} ;
    
} ;

//=== HoldsBirds specific functions ===//

Bird& FieldExperimenter::insertBird( pBird&& bird ) {
    bird->owner = this ;
    return birds.insert( std::move( bird ) ) ;
}

pBird FieldExperimenter::removeBird( const uint& index ) {
    pBird bird = birds.remove( index ) ;
    return bird ;
}

pBird FieldExperimenter::removeDeadBird( const uint& index ) {
    
    pBird bird = birds.remove(index);
    //bird->vendorPtr->removeBirdIndex(bird->index); // remove corresponding index from vendor
    return bird;

}

void FieldExperimenter::notifySuccessfulExposure( Bird& bird ) {};

std::string FieldExperimenter::getStringID() {
    std::string res = "FE" ;
    return res ;
}

NodeType FieldExperimenter::getSettingType() {
    return NodeType::fieldexperimenter ;
}

std::unordered_set<uint>& FieldExperimenter::getBirdsMarketIndex() {
    return birdsMarketIndex ;
}

void FieldExperimenter::registerFarmRecruitedBird( Bird& bird ) {
    BirdID birdID = bird.getID() ;
    experimentGroup[birdID] = 1 ;
}

void FieldExperimenter::registerMarketRecruitedBird( Bird& bird ) {
    BirdID birdID = bird.getID() ;
    experimentGroup[birdID] = 0 ;
}


// market-specialized inserter
// takes care of updating a bird's vendorPtr field and notifies vendor
/*
Bird& Market::insertNewBird(pBird&& bird, Vendor* vendor) {
    bird->owner = this;
    bird->vendorPtr = vendor;
    Bird& birdRef = birds.insert( std::move(bird) );
    vendor->addNewBirdIndex( birdRef.index );
    return birdRef;
}*/

void FieldExperimenter::reset( ChickenPool* birdMngr ) {
    
    birds.clear( birdMngr ) ;
    contEnv->clear() ;
    birdsMarketIndex.clear() ;
    experimentGroup.clear() ;
    
}
