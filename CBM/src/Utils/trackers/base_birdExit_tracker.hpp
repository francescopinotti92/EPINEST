//
//  base_birdExit_tracker.hpp
//  CBM
//
//

#ifndef base_birdExit_tracker_hpp
#define base_birdExit_tracker_hpp

#include "named_ofstream.hpp"
#include "clock.hpp"
#include "bird.hpp"
#include "types.hpp"
#include "useful_functions.hpp"

class BaseBirdExitTracker {
public:
    BaseBirdExitTracker( NamedOfstream outFile );
    virtual ~BaseBirdExitTracker() = default;
    virtual void addBird( const pBird& bird, const Clock& clock ) = 0;
    virtual void reset( bool updateStream = true ) = 0;
protected:
    NamedOfstream outFile;
};

#endif /* base_birdExit_tracker_hpp */
