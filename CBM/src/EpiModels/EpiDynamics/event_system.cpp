//
//  event_system.cpp
//  CBM
//
//

#include "event_system.hpp"

//====== EventQueue ======//

// Queue of events (of a single type).
// It is organized in time slots. Each time slot stores events to occur at the same time

// N.B. past slots are recycled in order to save memory.
//      There is a maximum of 'maxQueueLength' time slots available/
//      If an event has duration > maxQueueLength, the code will throw an error.


// constructor
template <typename EventType>
EventQueue<EventType>::EventQueue(const uint& maxQueueLength_, const uint& initialCapacityEvents_): maxQueueLength(maxQueueLength_) {

    eventList.reserve(maxQueueLength);
    eventCount.reserve(maxQueueLength);

    for (uint i = 0; i < maxQueueLength; i++) {
        eventList.push_back({});
        eventList[i].reserve(initialCapacityEvents_);
        eventCount.push_back(0);
    }
    
    currStep = 0;
}

// push event inside the queue
template <typename EventType>
void EventQueue<EventType>::pushEvent(const uint& timeToEvent, EventType event) {
    
    assert( timeToEvent < maxQueueLength);

    uint eventStep = getEventStep( timeToEvent );
    
    //std::cout << "check " << eventCount[eventStep] << " " << eventList[eventStep].size() << std::endl;
    
    if ( eventCount[eventStep] >= eventList[eventStep].size() )
        eventList[eventStep].push_back( event );                        // push element
    else
        eventList[eventStep][ eventCount[eventStep] ] = event;          // use empty space
    
    eventCount[eventStep]++;
}

// clears events in the time slot currently in use and updates internal time
template <typename EventType>
void EventQueue<EventType>::advanceQueueTime() {
    eventCount[currStep] = 0;                           // reset index in current step
    currStep = ( currStep + 1 ) % maxQueueLength;       //
}

template <typename EventType>
void EventQueue<EventType>::resetQueue() {
    
    for (uint step = 0; step < maxQueueLength; ++step) {
        eventCount[step] = 0;
    }
    
    currStep = 0;
}


// Converts time to event into the appropriate time slot index within the queue
template <typename EventType>
uint EventQueue<EventType>::getEventStep(const uint& timeToEvent) {
    if ( timeToEvent < maxQueueLength )
        return ( currStep + timeToEvent ) % maxQueueLength ;
    else
        return ( currStep + maxQueueLength - 1 ) % maxQueueLength ;
}

// Iterators to current events
template <typename EventType>
typename std::vector<EventType>::iterator EventQueue<EventType>::begin() { return eventList[currStep].begin(); }

template <typename EventType>
typename std::vector<EventType>::iterator EventQueue<EventType>::end() { return eventList[currStep].begin() + eventCount[currStep]; }


//====== template instantiations (only these kinds of queues can be used) ======//
template class EventQueue<BirdStatusEvent>;
template class EventQueue<BirdDeathEvent>;




