//
//  base_infection_tracer.hpp
//  CBM
//
//

#ifndef base_infection_tracer_hpp
#define base_infection_tracer_hpp

#include <vector>
#include <fstream>
#include <iostream>
#include "useful_functions.hpp"
#include "holdsbirds.hpp"
#include "named_ofstream.hpp"
#include "bird.hpp"
#include "types.hpp"

enum class InfectionEventType: short {
    Direct = 0,
    Environment = 1,
    External = 2,
};

//====== BaseInfectionTracker ======//
/*
    Interface for output objects tracking individual infection events
*/

class BaseInfectionTracker {
public:
    BaseInfectionTracker( NamedOfstream outFile );
    virtual ~BaseInfectionTracker() = default;
    virtual void addInfectionEvent( Bird& infector, Bird& infectee, const StrainIdType& strain, const Clock& clock ) = 0;
    virtual void addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) = 0;
    virtual void addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) = 0;

    virtual void reset( bool updateStream = true ) = 0;
    void setTrackingWindow( const uint& tStart, const uint& tStop );
protected:
    NamedOfstream outFile;
    uint tStartTracking; // in days
    uint tStopTracking; // in days
};


#endif /* base_infection_tracer_hpp */


