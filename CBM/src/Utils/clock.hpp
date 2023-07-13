//
//  clock.hpp
//  CBM_ECS
//
//

#ifndef clock_h
#define clock_h

#include <vector>
#include "types.hpp"
#include "constants.hpp"

/*
Clock measures the flow of time.
checkPts_ represents a list of user-defined timesteps (in hours)
Be sure that checkPts[i] < checkPts[j] for i < j and checkPts[-1] is not 0 or 24 if checkPts[0] = 0 or 24
Alse please use integers from 0 to 24.
*/


class Clock {
public:
    Clock(const uint& day_ = 0, const uint& dt_ = 1);
    void advance();
    void reset(uint day_ = 0);

    //== getter functions ==//
    
    uint getCurrDay()   const;
    uint getCurrHour()  const;
    uint getCurrTime()  const;
    uint getDt()        const;

    uint computeTimingHourDayAfter(const uint& h) const;
    //== print functions ==//
    
    void printDay();
    void printHour();

private:
    uint day;
    uint hour;
    uint dt;
    uint currTime;
};

/*
 Returns True if current timestep t is after t0 and dt divides t - t0
 */
bool checkTimePeriodic( const Clock& clock, const uint& dt = 1, const uint& t0 = 0 );

#endif /* clock_h */
