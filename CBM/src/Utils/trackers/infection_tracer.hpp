//
//  infection_tracer.hpp
//  CBM
//
//

#ifndef infection_tracer_hpp
#define infection_tracer_hpp

#include "base_infection_tracer.hpp"
#include "lineage_tracker_manager.hpp"
#include "ElementPool.hpp"
#include "birdID.hpp"
#include "clock.hpp"
#include "types.hpp"
#include <functional>
#include <memory>

//====== Single event objects ======//

struct OutputInfectionEvent {
    OutputInfectionEvent();
    OutputInfectionEvent( const BirdID& infectorID,
                          const BirdID& infecteeID,
                          const InfectionEventType& eventType,
                          const StrainIdType& strain,
                          const uint& t,
                          const std::string placeName );
    BirdID infectorID;
    BirdID infecteeID;
    InfectionEventType eventType;
    StrainIdType strain;
    uint t;
    std::string placeName;
};

//====== InfectionEventTree ======//

// stores events as tree that can be easily traversed backwards
// n.b. this is meant to work with one strain only

/*
class InfectionEventTree {
public:
    InfectionEventTree();
    ~InfectionEventTree() = default;
    void addEvent( const BirdID& infectorID,
                   const BirdID& infecteeID,
                   const InfectionEventType& eventType,
                   const StrainIdType& strain,
                   const uint& t,
                   const std::string placeName );
    void reset();
    
    std::unordered_set<BirdID>& getLeaves() { return leaves; }
    OutputInfectionEvent& getEvent( const BirdID& birdID ) { return events[birdID]; }
    
    bool isRoot( const BirdID& birdID );
    bool isLeaf( const BirdID& birdID );
    
private:
    std::map<BirdID,OutputInfectionEvent> events;
    std::unordered_set<BirdID> roots;
    std::unordered_set<BirdID> leaves;
    
    uint nEvents;
};*/



//====== Tracker objects ======//

// WIWTracer tracks individual infection events
// events are stored in a buffer that is flushed when at max capacity or if requested to

class WIWTracker: public BaseInfectionTracker {
public:
    WIWTracker( NamedOfstream outFile, const uint& bufferCapacity = 1000 );
    ~WIWTracker() override {};
    void reset( bool updateStream = true ) override;
    void addInfectionEvent( Bird& infector, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    
private:
    ElementPool<OutputInfectionEvent> eventBuffer;
    std::function< void () > flushEvents;
    void flushEventsWithPrint();
    void flushEventsWithoutPrint();
};


// Tracks new infection events by setting type
class AggregateSettingTypeIncidenceTracker: public BaseInfectionTracker {
public:
    AggregateSettingTypeIncidenceTracker( NamedOfstream outFile, const uint& deltaT = 24 );
    ~AggregateSettingTypeIncidenceTracker() override {};
    void reset( bool updateStream = true ) override;
    void addInfectionEvent( Bird& infector, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
private:
    uint deltaT;
    uint tOutput;
    std::unordered_map<StrainIdType, std::unordered_map<NodeType, uint>> incidence;
    std::function< void () > flushEvents;
    void flushEventsWithPrint();
    void flushEventsWithoutPrint();
};


/*
 
 Tracks infection events as a part of lineage tracking:
 each infection events leads to the birth of a new lineage
 
 
*/

class NewLineageTracker: public BaseInfectionTracker {
    
public:
    NewLineageTracker( const uint& S, const uint& maxStrains ) ;
    NewLineageTracker( std::shared_ptr<LineageTrackerManager> lngTrackerMngr ) ;
    ~NewLineageTracker() override {} ;
    void addInfectionEvent( Bird& infector, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override ;
    void addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override ;
    void addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) override ;
    void reset( bool updateStream = true ) override;
private:
    std::shared_ptr<LineageTrackerManager> lngTrackerMngr ;
    
} ;


// stores all infection events in a tree that can be traversed backwards
// at the end of a simulation prints subtree from a handful of leaf nodes
// n.b. not meant to be used with multiple strains

/*
class BackwardWIWTracker: public BaseInfectionTracker {
public:
    BackwardWIWTracker( NamedOfstream outFile, uint nLeavesSelect, uint dayMinSelect, uint dayMaxSelect );
    ~BackwardWIWTracker() override {};
    
    void reset( bool updateStream = true ) override;
    void addInfectionEvent( Bird& infector, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    
private:
    InfectionEventTree eventTree;
    //std::function< void () > flushEvents;
    //void flushEventsWithPrint();
    //void flushEventsWithoutPrint();
};
 */


#endif /* infection_tracer_hpp */
