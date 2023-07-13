//
//  infection_tracer.cpp
//  CBM
//
//

#include "infection_tracer.hpp"

//====== OutputInfectionEvent ======//

OutputInfectionEvent::OutputInfectionEvent(): infectorID( 0, 0 ), infecteeID( 0, 0 ), eventType( InfectionEventType::Direct ), strain( 0 ), t( 0 ), placeName( "NA" ) {};

OutputInfectionEvent::OutputInfectionEvent( const BirdID& infectorID_, const BirdID& infecteeID_, const InfectionEventType& eventType_, const StrainIdType& strain_, const uint& t_, const std::string placeName_ ): infectorID( infectorID_ ), infecteeID( infecteeID_ ), eventType( eventType_ ), strain( strain_ ), t( t_ ), placeName( placeName_ ) {};

//====== InfectionEventTree ======//

/*
InfectionEventTree::InfectionEventTree() {
    
    events = {};
    roots.clear();
    leaves.clear();
    roots.reserve( 10000 );
    leaves.reserve( 10000 );

    nEvents = 0;
    
}

void InfectionEventTree::addEvent( const BirdID& infectorID,
                                   const BirdID& infecteeID,
                                   const InfectionEventType& eventType,
                                   const StrainIdType& strain,
                                   const uint& t,
                                   const std::string placeName ) {
    
    // check if infectee is a root
    // simple check: infector is not in tree
    if ( events.count( infectorID ) == 0 ) {
        
        roots.insert( infecteeID ); // add root
        
    }
    else { // infector is in tree
        
        // if infector is leaf, now it should not be anymore
        if ( isLeaf( infectorID ) ) {
            
            leaves.erase( infectorID ); // remove lead
            
        }
        
    }
    leaves.insert( infecteeID ); // add infectee to leaves
    
    OutputInfectionEvent event( infectorID, infecteeID, eventType, strain, t, placeName );
    events[infecteeID] = event;
    ++nEvents;
    
    
}

void InfectionEventTree::reset() {
    
    
    events = {};
    roots.clear();
    leaves.clear();

    nEvents = 0;
    
}

bool InfectionEventTree::isRoot( const BirdID& birdID ) {
    
    return ( roots.find( birdID ) != roots.end() ) ? true : false;
    
}


bool InfectionEventTree::isLeaf( const BirdID& birdID ) {
    
    return ( leaves.find( birdID ) != leaves.end() ) ? true : false;
    
}

*/

//====== WIWTracer class ======//

WIWTracker::WIWTracker( NamedOfstream outFile, const uint& bufferCapacity ): BaseInfectionTracker( outFile ), eventBuffer( bufferCapacity ) {
    
    if ( outFile.isClosed() )
        flushEvents = std::bind( &WIWTracker::flushEventsWithoutPrint, this );
    else
        flushEvents = std::bind( &WIWTracker::flushEventsWithPrint, this );
};

void WIWTracker::addInfectionEvent( Bird &infector, Bird &infectee, const StrainIdType &strain, const Clock& clock ) {
    
    // check if event within time slice of interest
    uint tCurr = clock.getCurrDay();
    if ( tCurr < tStartTracking )
        return;
    if ( tCurr > tStopTracking )
        return;
    
    // check if event buffer is full
    if ( eventBuffer.isFull() )
        flushEvents();
    
    // add event to buffer
    std::string where = (infectee.owner)->getStringID();
    eventBuffer.insert( OutputInfectionEvent( infector.id, infectee.id, InfectionEventType::Direct, strain, clock.getCurrTime(), where ) );
}

void WIWTracker::addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) {
    
    // check if event within time slice of interest
    uint tCurr = clock.getCurrDay();
    if ( tCurr < tStartTracking )
        return;
    if ( tCurr > tStopTracking )
        return;
    
    // check if event buffer is full
    if ( eventBuffer.isFull() )
        flushEvents();
    
    // add event to buffer
    std::string where = (infectee.owner)->getStringID();
    eventBuffer.insert( OutputInfectionEvent( infectorID, infectee.id, InfectionEventType::Environment, strain, clock.getCurrTime(), where ) );
}

void WIWTracker::addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) {
    
    uint tCurr = clock.getCurrDay();
    if ( tCurr < tStartTracking )
        return;
    if ( tCurr > tStopTracking )
        return;
    
    // check if event buffer is full
    if ( eventBuffer.isFull() )
        flushEvents();
    
    // add event to buffer
    std::string where = (infectee.owner)->getStringID();
    eventBuffer.insert( OutputInfectionEvent( BirdID(0,0), infectee.id, InfectionEventType::External, strain, clock.getCurrTime(), where ) );
}



void WIWTracker::reset( bool updateStream ) {

    flushEvents(); // clear buffer and eventually print

    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

void WIWTracker::flushEventsWithPrint() {

    auto& stream = outFile.getStream();
    while ( !eventBuffer.empty() ) {
        OutputInfectionEvent& ev = eventBuffer.peekLast();
        stream << fmt::format("{},{},{},{},{},{}\n", ev.infectorID.toString(), ev.infecteeID.toString(), as_integer( ev.eventType ), ev.strain, ev.t, ev.placeName );
        eventBuffer.removeLast();
    }
}

void WIWTracker::flushEventsWithoutPrint() {
    eventBuffer.clear();
}

//====== AggregateSettingTypeIncidenceTracer ======//

AggregateSettingTypeIncidenceTracker::AggregateSettingTypeIncidenceTracker( NamedOfstream outFile, const uint& deltaT_ ): BaseInfectionTracker( outFile ),  deltaT( deltaT_ ) {
    
    assert( deltaT > 0 ); // deltaT should be a positive integer
    
    if ( outFile.isClosed() )
        flushEvents = std::bind( &AggregateSettingTypeIncidenceTracker::flushEventsWithoutPrint, this );
    else
        flushEvents = std::bind( &AggregateSettingTypeIncidenceTracker::flushEventsWithPrint, this );
}

void AggregateSettingTypeIncidenceTracker::addInfectionEvent( Bird &infector, Bird &infectee, const StrainIdType &strain, const Clock &clock ) {
    
    uint t = clock.getCurrTime();
    
    if ( t % deltaT == 0 ) {
        tOutput = t;
        flushEvents();
    }
    
    NodeType settingType = infectee.owner->getSettingType();
    incidence[ strain ][ settingType ]++;
    
}

void AggregateSettingTypeIncidenceTracker::addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) {
    
    uint t = clock.getCurrTime();
    
    if ( t % deltaT == 0 ) {
        tOutput = t;
        flushEvents();
    }
    
    NodeType settingType = infectee.owner->getSettingType();
    incidence[ strain ][ settingType ]++;
    
}

void AggregateSettingTypeIncidenceTracker::addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) {
    
    uint t = clock.getCurrTime();
    
    if ( t % deltaT == 0 ) {
        tOutput = t;
        flushEvents();
    }
    
    NodeType settingType = infectee.owner->getSettingType();
    incidence[ strain ][ settingType ]++;

}


void AggregateSettingTypeIncidenceTracker::reset( bool updateStream ) {
    incidence.clear();
    tOutput = 0;
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

void AggregateSettingTypeIncidenceTracker::flushEventsWithPrint() {
    for ( auto& outerElem: incidence ) {
        StrainIdType strain = outerElem.first;
        for ( auto& innerElem: outerElem.second ) {
            NodeType settingType = innerElem.first;
            uint inc = innerElem.second;
            outFile.getStream() << fmt::format( "{},{},{},{}\n", tOutput, strain, constants::getStringFromNodeType( settingType ), inc );
        }
    }
    incidence.clear();
}

void AggregateSettingTypeIncidenceTracker::flushEventsWithoutPrint() {
    incidence.clear();
}


//====== NewLineageTracker ======//

NewLineageTracker::NewLineageTracker( const uint& S, const uint& maxStrains ): BaseInfectionTracker( NamedOfstream() ) {
    
    lngTrackerMngr = std::make_shared<LineageTrackerManager>( S, maxStrains ) ;
    
}

NewLineageTracker::NewLineageTracker( std::shared_ptr<LineageTrackerManager> lngTrackerMngr_ ): BaseInfectionTracker( NamedOfstream() ) {
    
    lngTrackerMngr = lngTrackerMngr_ ;
    
}


void NewLineageTracker::addInfectionEvent( Bird& infector, Bird& infectee, const StrainIdType& strain, const Clock& clock ) {
    
    lngTrackerMngr->addInfectionEvent( infectee, infector, strain, clock ) ;
    
}


void NewLineageTracker::addEnvironmentInfectionEvent( const BirdID &infectorID, Bird &infectee, const StrainIdType &strain, const Clock &clock ) {
    
    // !!! redirect to Lineage Tree Manager
    
}

void NewLineageTracker::addExternalInfectionEvent( Bird &infectee, const StrainIdType &strain, const Clock &clock ) {
        
    lngTrackerMngr->addExternalInfectionEvent( infectee, strain, clock ) ;
    
}

void NewLineageTracker::reset( bool updateStream ) {
    
    // !!! implement
    ;
}

