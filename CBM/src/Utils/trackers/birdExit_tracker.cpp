//
//  birdExit_tracker.cpp
//  CBM
//
//

#include "birdExit_tracker.hpp"

//====== BirdMarketingDurationTracker ======//

BirdMarketingDurationTracker::BirdMarketingDurationTracker( NamedOfstream outFile, const uint& dtMax_ ): BaseBirdExitTracker( outFile ), dtMax( dtMax_ ) {
    
    counter = std::vector<uint>( dtMax, 0 );
    
};

void BirdMarketingDurationTracker::addBird( const pBird& bird, const Clock& clock ) {
   
    uint t = clock.getCurrTime() - bird->t_market;
    if ( t >= dtMax )
        counter.resize( t + 1 ); // extend counter
    ++counter[t];
}
void BirdMarketingDurationTracker::reset( bool updateStream ) {
    if ( !outFile.isClosed() ) {
        outFile.getStream() << stringify_vec( counter );
        counter = std::vector<uint>( dtMax, 0 );
        
    }
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

//====== LineageBirdRemovalTracker ======//


LineageBirdRemovalTracker::LineageBirdRemovalTracker( std::shared_ptr<LineageTrackerManager> lngTrackerMngr_ ): BaseBirdExitTracker( NamedOfstream() ), lngTrackerMngr( lngTrackerMngr_ ) {} ;

void LineageBirdRemovalTracker::addBird( const pBird &bird, const Clock &clock ) {
    
    lngTrackerMngr->addRemovalEvent( *bird ) ;
    
}
