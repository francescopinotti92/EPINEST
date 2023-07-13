//
//  constants.hpp
//  CBM_ECS
//
//

#ifndef constants_h
#define constants_h

#include "types.hpp"
#include <utility>


//====== Constant MACROS ======//

#ifndef M_E
    #define M_E        2.71828182845904523536028747135      /* e */
#endif

#ifndef M_PI
    #define M_PI       3.14159265358979323846264338328      /* pi */
#endif

#ifndef HOURS_DAY
    #define HOURS_DAY 24
#endif

#ifndef MARKET_NDAYS_CYCLE
    #define MARKET_NDAYS_CYCLE 10   // hour at which markets open (7 means from 6 to 7 a.m.)
#endif

#ifndef MARKET_OPENING_HOUR
    #define MARKET_OPENING_HOUR 6   // hour at which markets open (7 means from 6 to 7 a.m.)
#endif

#ifndef MARKET_CLOSING_HOUR
    #define MARKET_CLOSING_HOUR 23   // hour at which markets close (7 means from 6 to 7 a.m.)
#endif

#ifndef FARM_DAILY_CHECK_HOUR
    #define FARM_DAILY_CHECK_HOUR 12
#endif

/*
 namespace "constants" contains constant variables used throughout the code
*/

namespace constants {

static constexpr StateType ONE64 = 1;
static const uint nBirdTypes = 2; // number of bird types to allow for

//static std::unordered_map<BirdType, const uint> harvestTimes;
extern std::unordered_map<BirdType, const uint> harvestTimes;

static std::vector<BirdType> allBirdTypes = { BirdType::null, BirdType::broiler, BirdType::deshi };

std::string getStringFromBirdType( const BirdType& bt );
BirdType getBirdTypeFromString( const std::string& birdTypeString );
NodeType getNodeTypeFromString( const std::string& nodeTypeString );
ActorType getActorTypeFromString( const std::string& actorTypeString );
std::string getStringFromNodeType( const NodeType& nt );


} // end constants

#endif /* constants_h */
