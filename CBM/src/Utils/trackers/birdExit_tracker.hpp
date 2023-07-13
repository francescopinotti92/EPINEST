//
//  birdExit_tracker.hpp
//  CBM
//
//

#ifndef birdExit_tracker_hpp
#define birdExit_tracker_hpp

#include "base_birdExit_tracker.hpp"
#include "lineage_tracker_manager.hpp"
#include <memory>

class BirdMarketingDurationTracker: public BaseBirdExitTracker {
public:
    BirdMarketingDurationTracker( NamedOfstream outFile, const uint& dtMax );
    void addBird( const pBird& bird, const Clock& clock ) override;
    void reset( bool updateStream = true ) override;
private:
    std::vector<uint> counter;
    uint dtMax;
};

/*
 
 Tracks bird removal as part of lineage tracking
 
 */
class LineageBirdRemovalTracker: public BaseBirdExitTracker {
public:
    LineageBirdRemovalTracker( std::shared_ptr<LineageTrackerManager> lngTrackerMngr ) ;
    void addBird( const pBird& bird, const Clock& clock ) override;
    void reset( bool updateStream = true ) override {}; // does nothing
private:
    std::shared_ptr<LineageTrackerManager> lngTrackerMngr ;
    
} ;

#endif /* birdExit_tracker_hpp */
