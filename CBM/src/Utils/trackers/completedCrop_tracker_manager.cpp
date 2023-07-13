//
//  completedCrop_tracker_manager.cpp
//  CBM
//
//

#include "completedCrop_tracker_manager.hpp"



CompletedCropTrackerManager::CompletedCropTrackerManager() {
    
    trackers = {} ;
    
}

void CompletedCropTrackerManager::addTracker( BaseCompletedCropTracker* tracker ) {
    
    trackers.push_back( tracker ) ;
    
}

void CompletedCropTrackerManager::processCompletedCrop( NodeFarm& nf, Crop* crop, const Clock& clock ) {
    
    for ( auto& tracker : trackers ) {
        
        tracker->processCompletedCrop( &nf, crop, clock ) ;
        
    }
    
}

void CompletedCropTrackerManager::processCompletedCrop( Farm& farm, Crop* crop, const Clock& clock ) {
    
    for ( auto& tracker : trackers ) {
        
        tracker->processCompletedCrop( &farm, crop, clock ) ;
        
    }
    
}


void CompletedCropTrackerManager::reset( bool updateStreams ) {
    
    for ( auto& tracker : trackers ) {
        
        tracker->reset( updateStreams ) ;
        
    }
    
}
