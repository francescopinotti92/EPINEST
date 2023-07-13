//
//  base_cross_sectional_tracker.cpp
//  CBM
//
//

#include "base_cross_sectional_tracker.hpp"

//====== BaseCrossSectionalTracker ======//

BaseCrossSectionalTracker::BaseCrossSectionalTracker( NamedOfstream outFile_, uint deltaTdays_, std::vector<uint> hs, uint tmin, uint tmax ): outFile( outFile_ ), deltaTdays( deltaTdays_ ), tLastOutputDay( 0 ), tmin( tmin ), tmax( tmax ) {
    
    hsActive = std::vector<bool>( HOURS_DAY, false );
    for ( uint h: hs ) {
        assert( h >= 0 and h < 24);
        hsActive[h] = true;
    }

}

/*
 
 Checks:

    (i)   t >= tmin
    (ii)  t <= tmax
    (iii) time of the day
    (iv)  periodicity
 
 */
bool BaseCrossSectionalTracker::checkTimePeriodic( Clock& clock ) {
    
    if ( clock.getCurrTime() < tmin )
        return false;
    
    if ( clock.getCurrTime() > tmax )
        return false;
    
    bool cond1 = hsActive[ clock.getCurrHour() ] ;
    bool cond2 = clock.getCurrDay() % deltaTdays == 0;
    return ( cond1 and cond2 ) ? true : false;
}

/*
 
 Checks:

    (i)   t >= tmin
    (ii)  t <= tmax
    (iii) time of the day
 
 */
bool BaseCrossSectionalTracker::checkTimeFixedWindow( Clock& clock ) {
    
    if ( clock.getCurrTime() < tmin )
        return false;
    
    if ( clock.getCurrTime() > tmax )
        return false;
    
    return hsActive[ clock.getCurrHour() ] ;
    
}

bool BaseCrossSectionalTracker::checkTimeFixedTiming( Clock& clock, const uint& t0 ) {
    
    return ( clock.getCurrTime() == t0 ) ? true : false ;
    
}

