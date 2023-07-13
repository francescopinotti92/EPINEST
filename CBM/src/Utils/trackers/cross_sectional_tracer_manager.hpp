//
//  cross_sectional_tracer_manager.hpp
//  CBM
//
//

#ifndef cross_sectional_tracer_manager_hpp
#define cross_sectional_tracer_manager_hpp

#include "base_cross_sectional_tracker.hpp"
#include "WorldSimulator.hpp"

class BaseCrossSectionalTracker;
class WorldSimulator;

//====== CrossSectionalTrackerManager ======//
class CrossSectionalTrackerManager {
    typedef std::shared_ptr<BaseCrossSectionalTracker> pTrackerCS;
public:
    CrossSectionalTrackerManager();
    void addTracker( pTrackerCS tracker, bool preMovements = true );
    void addTrackerT0( pTrackerCS tracker );

    void collectInfoPreMovements( WorldSimulator* simulator );
    void collectInfoPostMovements( WorldSimulator* simulator );

    void collectInfoT0( WorldSimulator* simulator );
    void reset( bool updateStreams = true );
private:
    std::vector<pTrackerCS> trackerVecPreMovements;
    std::vector<pTrackerCS> trackerVecPostMovements;

    std::vector<pTrackerCS> oneShotT0TrackerVec;
};

#endif /* cross_sectional_tracer_manager_hpp */
