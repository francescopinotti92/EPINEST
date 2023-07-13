//
//  lineage_tracker.hpp
//  CBM
//
//

#ifndef lineage_tracker_hpp
#define lineage_tracker_hpp

#include <stdio.h>

#include "base_cross_sectional_tracker.hpp"
#include "base_infection_tracer.hpp"

struct LineageData {
    LineageData() ;
    LineageData( const uint& lineageID, Clock& clock ) ;
    uint lineageID ;
    uint dayIntro ;
    uint size ; // number of secondary chickens infected
};


/*
 This is a special tracker that tracks which lineages are present at the market every day.
 A lineage is defined as a new infection that enters the market but is not associated to
 any other lineage. It also checks which lineages went extinct and measures how many days it
 persisted in the market system.
 
 Notes:
 
 1) Requires own WIW tracker defined below: do not specify a WIW tracker if
 you want to analyse lineage persistence.
 
 2) Does not work with multiple strains.
 
 3) Does not work with environmental transmission.
 
 
 */
class NewMarketLineagesTracker: public BaseCrossSectionalTracker {
public:
    NewMarketLineagesTracker( NamedOfstream outFile, const uint& tmin, const uint& tmax, const uint& minClusterSize = 0 );
    void collectInfo( WorldSimulator* simulator ) override ;
    void reset( bool updateStream = true  ) override ;
    void addInfectionEvent( Bird& infector, Bird& infectee, const Clock& clock ) ;
private:
    std::unordered_map<BirdID, uint, hashBirdID> bird2lineage ;
    std::unordered_map<uint, LineageData> existingLineages ;
    uint nCreatedLineages;
    std::vector<uint> lineageDurationHist ; // holds results (no restriction)
    std::vector<uint> lineageDurationMinSizeHist ; // holds results (restricted to min size of clusters)

    bool initialised;
    uint minClusterSize;
};

// Tracks new infections in markets and checks


class NewMarketLineagesWIWHelperTracker: public BaseInfectionTracker {
    
public:
    NewMarketLineagesWIWHelperTracker( std::shared_ptr<NewMarketLineagesTracker> lngTracker );
    ~NewMarketLineagesWIWHelperTracker() override {};
    void reset( bool updateStream = true ) override;
    void addInfectionEvent( Bird& infector, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addEnvironmentInfectionEvent( const BirdID& infectorID, Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    void addExternalInfectionEvent( Bird& infectee, const StrainIdType& strain, const Clock& clock ) override;
    
private:
    
    std::shared_ptr<NewMarketLineagesTracker> lngTracker ; // contains all info and eventually process data
    
    
} ;



#endif /* lineage_tracker_hpp */
