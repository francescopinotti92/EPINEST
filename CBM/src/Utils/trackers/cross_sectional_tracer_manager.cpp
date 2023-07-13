//
//  cross_sectional_tracer_manager.cpp
//  CBM
//
//

#include "cross_sectional_tracer_manager.hpp"

//====== CrossSectionalTrackerManager ======//

// Constructor

CrossSectionalTrackerManager::CrossSectionalTrackerManager() {
    
    trackerVecPreMovements = {} ;
    trackerVecPostMovements = {} ;
    oneShotT0TrackerVec = {} ;
    
}

// add a new tracker

void CrossSectionalTrackerManager::addTracker( pTrackerCS tracker, bool preMovements ) {
    
    if ( preMovements )
        trackerVecPreMovements.push_back( tracker ) ;
    else
        trackerVecPostMovements.push_back( tracker ) ;
    
}

// add a new tracker (works only once for each batch of simulations)

void CrossSectionalTrackerManager::addTrackerT0( pTrackerCS tracker ) {
    
    oneShotT0TrackerVec.push_back( tracker ) ;
    
}

// make each tracker do its job

void CrossSectionalTrackerManager::collectInfoPreMovements ( WorldSimulator* simulator ) {
    
    for ( pTrackerCS& tracker : trackerVecPreMovements )
        tracker->collectInfo( simulator ) ;
    
}

// make each tracker do its job (before movements are resolved)

void CrossSectionalTrackerManager::collectInfoPostMovements ( WorldSimulator* simulator ) {
    
    for ( pTrackerCS& tracker : trackerVecPostMovements )
        tracker->collectInfo( simulator ) ;
    
}

// make each tracker do its job (after movements are resolved)

void CrossSectionalTrackerManager::collectInfoT0( WorldSimulator* simulator ) {
    
    for ( pTrackerCS& tracker : oneShotT0TrackerVec )
        tracker->collectInfo( simulator ) ;
    
}

// reset everything, eventually opening new files for new simulations
void CrossSectionalTrackerManager::reset( bool updateStreams ) {
    
    for ( pTrackerCS& tracker : trackerVecPreMovements )
        tracker->reset( updateStreams ) ;
    
    for ( pTrackerCS& tracker : trackerVecPostMovements )
        tracker->reset( updateStreams ) ;
    
    for ( pTrackerCS& tracker : oneShotT0TrackerVec )
        tracker->reset( updateStreams ) ;
    
}
