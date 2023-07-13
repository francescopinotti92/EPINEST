//
//  birdExit_tracker_manager.cpp
//  CBM
//
//

#include "birdExit_tracker_manager.hpp"


BirdExitTrackerManager::BirdExitTrackerManager() {
    
    trackersAll = {} ;
    trackersFarmRemoval = {} ;
    trackersMarketVendor = {} ;
    

}

void BirdExitTrackerManager::addFarmRemovalTracker( std::shared_ptr<BaseBirdExitTracker> tracker ) {
    
    trackersAll.push_back( tracker ) ;
    trackersFarmRemoval.push_back( tracker ) ;

}

void BirdExitTrackerManager::addMarketVendorTracker( std::shared_ptr<BaseBirdExitTracker> tracker ) {
    
    trackersAll.push_back( tracker ) ;
    trackersMarketVendor.push_back( tracker ) ;

}

void BirdExitTrackerManager::birdRemovedFromFarm( const pBird& bird, const Clock& clock ) {
    
    for ( std::shared_ptr<BaseBirdExitTracker>& tracker: trackersFarmRemoval )
        tracker->addBird( bird, clock ) ;
    
}


void BirdExitTrackerManager::birdExitsFromMarketOrVendor( const pBird& bird, const Clock& clock ) {
    
    for ( std::shared_ptr<BaseBirdExitTracker>& tracker: trackersMarketVendor )
        tracker->addBird( bird, clock ) ;
    
}

void BirdExitTrackerManager::reset( bool updateStreams ) {
    
    for ( std::shared_ptr<BaseBirdExitTracker>& tracker: trackersAll )
        tracker->reset( updateStreams ) ;
    
}
