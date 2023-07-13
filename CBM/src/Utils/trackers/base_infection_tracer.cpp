//
//  base_infection_tracer.cpp
//  CBM
//
//

#include "base_infection_tracer.hpp"

//====== BaseInfectionTracker ======//
BaseInfectionTracker::BaseInfectionTracker( NamedOfstream outFile_ ): outFile( outFile_ ), tStartTracking( 0 ), tStopTracking( 10000000 ) {};

void BaseInfectionTracker::setTrackingWindow( const uint& tStart, const uint& tStop ) {
    tStartTracking = tStart;
    tStopTracking = tStop;
}
