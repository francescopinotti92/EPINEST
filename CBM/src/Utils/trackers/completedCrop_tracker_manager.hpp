//
//  completedCrop_tracker_manager.hpp
//  CBM
//
//

#ifndef completedCrop_tracker_manager_hpp
#define completedCrop_tracker_manager_hpp

#include "BaseCompletedCropTracker.hpp"
#include "node.hpp"
#include "farm.hpp"
#include <memory.h>
#include <stdio.h>
#include <vector>

class BaseCompletedCropTracker;

class CompletedCropTrackerManager {
public:
    CompletedCropTrackerManager();
    void addTracker( BaseCompletedCropTracker* tracker );
    void processCompletedCrop( NodeFarm& nf, Crop* crop, const Clock& clock );
    void processCompletedCrop( Farm& nf, Crop* crop, const Clock& clock );
    void reset( bool updateStreams = true );
private:
    std::vector<BaseCompletedCropTracker*> trackers;
};

#endif /* completedCrop_tracker_manager_hpp */
