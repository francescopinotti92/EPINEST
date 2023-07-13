//
//  base_cross_sectional_tracker.hpp
//  CBM
//
//

#ifndef base_cross_sectional_tracker_hpp
#define base_cross_sectional_tracker_hpp

#include "WorldSimulator.hpp"
#include "named_ofstream.hpp"
#include "clock.hpp"
#include "types.hpp"
#include "checks.hpp"
#include <string>
#include <limits>

class WorldSimulator;

//====== BaseCrossSectionalTracker ======//
/*
    Interface for output objects measuring cross-sectional properties of the system
    these trackers are triggered periodically every 'deltaTdays' at a particular hour 'h'.
    Please remember to call them every time step.
*/


class BaseCrossSectionalTracker {
public:
    BaseCrossSectionalTracker( NamedOfstream outFile, uint deltaTdays, std::vector<uint> hs, uint tmin = 0, uint tmax = std::numeric_limits<uint>::max() );
    virtual ~BaseCrossSectionalTracker() = default;
    virtual void collectInfo( WorldSimulator* simulator ) = 0;
    virtual void reset( bool updateStream = true ) = 0;
    std::function< bool ( Clock& ) > checkTime;
    bool checkTimePeriodic( Clock& clock ) ;
    bool checkTimeFixedWindow( Clock& clock ) ;
    bool checkTimeFixedTiming( Clock& clock, const uint& t0 ) ;
protected:
    NamedOfstream outFile;
    std::vector<bool> hsActive;
    uint tmin;
    uint tmax;
    uint deltaTdays;
    uint tLastOutputDay;

};

#endif /* base_cross_sectional_tracker_hpp */
