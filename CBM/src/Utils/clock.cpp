//
//  clock.cpp
//  CBM
//
//

#include "clock.hpp"

//=== Constructor ===//

Clock::Clock(const uint& day_, const uint& dt_): day(day_), hour(0), dt(dt_), currTime(0) {};

// increase time by one timestep
void Clock::advance() {
    if (hour + dt >= HOURS_DAY)
        day++;
    hour = (hour + dt) % HOURS_DAY;
    currTime++;
}

// reset clock
void Clock::reset(uint day_) {
    day = day_;
    hour = 0;
    currTime = 0;
}

//=== getter functions ===//

uint Clock::getCurrDay()    const {return day;}
uint Clock::getCurrHour()   const {return hour;}
uint Clock::getCurrTime()   const {return currTime;}
uint Clock::getDt()         const {return dt;}

uint Clock::computeTimingHourDayAfter(const uint& h) const { return h + (day + 1) * HOURS_DAY; }


//=== print functions ===//

void Clock::printDay() {std::cout << day << std::endl;}
void Clock::printHour() {std::cout << hour << std::endl;}

//=== free functions ===//

bool checkTimePeriodic( const Clock& clock, const uint& dt, const uint& t0 ) {
    
    uint t = clock.getCurrTime();
    
    if ( t < t0 )
        return false;
    
    if ( ( t - t0 ) % dt == 0 )
        return true;
    else
        return false;
    
}

