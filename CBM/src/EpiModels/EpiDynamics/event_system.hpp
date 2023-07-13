//
//  event_system.hpp
//  CBM
//
//

#ifndef event_system_hpp
#define event_system_hpp

#include "types.hpp"
#include "bird.hpp"
#include "clock.hpp"
#include <queue>

//====== Single events ======//

// contains info about strain that triggered the event
struct BirdStatusEvent {
    pBird targetBird;
    BirdID birdID; // bird id when event is created
    StrainIdType strain;
    uint t;
    
    // constructor uses shared_ptr to bird
    BirdStatusEvent( const pBird& targetBird_, const StrainIdType& strain_, const uint& t_ ): targetBird( targetBird_ ), birdID( targetBird_->id ), strain( strain_ ), t( t_ ) {};

};

struct BirdDeathEvent {
    pBird targetBird;
    BirdID birdID;
    uint t;
    BirdDeathEvent( pBird& targetBird_, const uint& t_ ): targetBird( targetBird_ ), birdID( targetBird_->id ), t( t_ ) {};
};

template <typename EventType>
class EventQueue {
    typedef typename std::vector<EventType>::iterator EventTypeIter;
public:
    EventQueue(const uint& maxQueueLength_, const uint& initialCapacityEvents_);
    void pushEvent(const uint& timeToEvent, EventType event);
    void advanceQueueTime();
    void resetQueue();
    
    EventTypeIter begin();
    EventTypeIter end();

private:
    uint getEventStep(const uint& timeToEvent);
    std::vector<std::vector<EventType> > eventList;
    std::vector<uint> eventCount;
    uint maxQueueLength;
    uint currStep;
};

//===== Priority queue implementation =====//

template <typename T>
struct EventComparator {
    EventComparator() {};
    inline bool operator() ( const T& left, const T& right ) { return left.t > right.t; }
};

template <typename T>
class EventPriorityQueue {
public:
    EventPriorityQueue() {};
    void pushEvent( const T& event ) {
        eventQ.push( event );
    }
    const T& top() {
        return eventQ.top();
    }
    void pop() {
        if ( !eventQ.empty() )
            eventQ.pop();
    }
    bool empty() {
        return ( eventQ.empty() ) ? true : false;
    }
    uint size() {
        return static_cast<uint>( eventQ.size() );
    }
    
    void resetQueue() {
        while ( !eventQ.empty() ) {
            eventQ.pop();
        }
    }
    
private:
    static bool compareTimeOrder( const T& left, const T& right ) { return left.t > right.t; }
    std::priority_queue<T, std::vector<T>, EventComparator<T>> eventQ;
};


#endif /* event_system_hpp */
