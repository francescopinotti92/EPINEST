//
//  birdExit_tracker_manager.hpp
//  CBM
//
//

#ifndef birdExit_tracker_manager_hpp
#define birdExit_tracker_manager_hpp

#include "base_birdExit_tracker.hpp"
#include <memory>

class BirdExitTrackerManager {
public:
    BirdExitTrackerManager();
    void addFarmRemovalTracker( std::shared_ptr<BaseBirdExitTracker> tracker ) ;
    void addMarketVendorTracker( std::shared_ptr<BaseBirdExitTracker> tracker ) ;
    void birdRemovedFromFarm( const pBird& bird, const Clock& ) ;
    void birdExitsFromMarketOrVendor( const pBird& bird, const Clock& clock ) ;
    void reset( bool updateStreams = true );
private:
    std::vector< std::shared_ptr<BaseBirdExitTracker> > trackersAll;
    std::vector< std::shared_ptr<BaseBirdExitTracker> > trackersFarmRemoval;
    std::vector< std::shared_ptr<BaseBirdExitTracker> > trackersMarketVendor; // chickens removed through sale in markets and vendors
};

#endif /* birdExit_tracker_manager_hpp */
