//
//  cross_sectional_tracker.hpp
//  CBM
//
//

#ifndef cross_sectional_tracker_hpp
#define cross_sectional_tracker_hpp

#include "sampling.hpp"
#include "LineageTree.hpp"
#include "base_cross_sectional_tracker.hpp"

//====== Cross-sectional trackers ======//

//=== breaks up chicken by setting type (farms, middlemen, markets, vendors)
class AggregateBirdCountSettingTracker: public BaseCrossSectionalTracker {
public:
    AggregateBirdCountSettingTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~AggregateBirdCountSettingTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::unordered_map<NodeType, uint> countBySetting;
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
};


//=== breaks up total prevalence by setting type (farms, middlemen, markets, vendors)
class AggregatePrevalenceSettingTracker: public BaseCrossSectionalTracker {
public:
    AggregatePrevalenceSettingTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~AggregatePrevalenceSettingTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::unordered_map<NodeType, uint> countBySetting; // infected chickens by setting
    std::unordered_map<NodeType, uint> countTotBySetting; // total chickens by setting
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
};

//=== Prints IDs of infected farms
class InfectedFarmsTracker: public BaseCrossSectionalTracker {
public:
    InfectedFarmsTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const std::string& sep = "," );
    ~InfectedFarmsTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
    ElementPool<uint> farmIdxs;
    std::string sep;
};

//=== Prints susceptibility to various strains by setting
class AggregateImmunitySettingTracker: public BaseCrossSectionalTracker {
public:
    AggregateImmunitySettingTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~AggregateImmunitySettingTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::map<NodeType, std::map<StateType,uint>> countByStateBySetting;
    std::map<NodeType, int> countChickensBySetting;

    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
    
};

//=== computes multiplicity of infections (mono-, bi-, tri-infected and so on..) by setting type
class AggregateInfectionMultiplicityBySettingTracker: public BaseCrossSectionalTracker {
public:
    AggregateInfectionMultiplicityBySettingTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~AggregateInfectionMultiplicityBySettingTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::map<NodeType, std::map<uint,uint>> countByMultiplicityBySetting;

    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
    
};

//=== computes richness in markets, aggregated over all markets
// uses infected chickens only
class AggregateRichnessMarkets: public BaseCrossSectionalTracker {
public:
    AggregateRichnessMarkets( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~AggregateRichnessMarkets() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::unordered_set<StrainIdType> strainsFound ;
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
};

//=== computes Simpson's diversity index in markets, aggregated over all markets
// uses infected chickens only
class AggregateSimpsonDiversityMarkets: public BaseCrossSectionalTracker {
public:
    AggregateSimpsonDiversityMarkets( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~AggregateSimpsonDiversityMarkets() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::unordered_map<StrainIdType, uint> strainAbundance ;
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
};

//=== computes richness in individual markets
// uses infected chickens only
class RichnessSingleMarkets: public BaseCrossSectionalTracker {
public:
    RichnessSingleMarkets( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~RichnessSingleMarkets() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::unordered_map<uint, std::unordered_set<StrainIdType>> strainsFoundByMarket ;
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
};

//=== computes richness in individual areas
// uses infected chickens only
class RichnessSingleAreas: public BaseCrossSectionalTracker {
public:
    RichnessSingleAreas( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {0}, const uint& tmin = 0 );
    ~RichnessSingleAreas() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    std::unordered_map<uint, std::unordered_set<StrainIdType>> strainsFoundByArea ;
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
};



//=== Prints bird counts per market by farm
class BirdMixingMarketTracker: public BaseCrossSectionalTracker {
public:
    BirdMixingMarketTracker( NamedOfstream outFile, bool infectedOnly, const uint& deltaTdays = 1, const std::vector<uint>& hs = {12}, const uint& tmin = 0 );
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
    std::function< void( NodeMarket* ) > getMixing;
    void getMixingByFarmAll( NodeMarket* market );
    void getMixingByFarmInfected( NodeMarket* market );
    std::vector<std::map<uint,uint>> birdCts;
    bool infectedOnly;
};

//=== Prints strain counts per market
class StrainMixingMarketTracker: public BaseCrossSectionalTracker {
public:
    StrainMixingMarketTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {12}, const uint& tmin = 0 );
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    void doNothing( Clock& clock ) {};
    void flushWithPrint( Clock& clock );
    void getMixing( NodeMarket* market, BaseCompartmentalModel& epiModel );
    std::vector<std::map<uint,StrainIdType>> strainCts;
};



//=== tracks farm rollout duration
// N.b.1) works only for farm types that actually perform a rollout
// N.b.2) asks to farms if they have emptied their batch,
// hence must be called right before farms are about to be checked for empty status
class FarmRolloutTimeTracker: public BaseCrossSectionalTracker {
public:
    FarmRolloutTimeTracker( NamedOfstream outFile );
    ~FarmRolloutTimeTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    ElementPool<std::pair<std::string, uint>> buffer;
    void doNothing( Clock& clock );
    void flushWithPrint( Clock& clock );
};

//=== measures number of birds unsold (middlemen)
class UnsoldMmanStockTracker: public BaseCrossSectionalTracker {
public:
    UnsoldMmanStockTracker( NamedOfstream outFile, const uint& deltaTdays = 1 );
    ~UnsoldMmanStockTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    ElementPool<std::pair<std::string, uint>> buffer;
    void doNothing( Clock& clock );
    void flushWithPrint( Clock& clock );
};

//=== measures number of birds unsold (vendors)
class UnsoldVendorStockTracker: public BaseCrossSectionalTracker {
public:
    UnsoldVendorStockTracker( NamedOfstream outFile, const uint& deltaTdays = 1 );
    ~UnsoldVendorStockTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
    std::function< void ( Clock& ) > flush;
private:
    ElementPool<std::pair<std::string, uint>> buffer;
    void doNothing( Clock& clock );
    void flushWithPrint( Clock& clock );
};

//=== computes cumulative number of birds that went unsold
class DiscardedBirdsTracker: public BaseCrossSectionalTracker {
public:
    DiscardedBirdsTracker( NamedOfstream outFile, const uint& deltaTdays = 1 );
    ~DiscardedBirdsTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
};

//=== samples distribution of times since a bird enters a market
class TimeSinceMarketingTracker: public BaseCrossSectionalTracker {
public:
    TimeSinceMarketingTracker( NamedOfstream outFile, const uint& dtMax, const uint& deltaTdays = 1 );
    ~TimeSinceMarketingTracker() {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true ) override;
private:
    uint dtMax;
};

//=== Prints counts of infected birds bought by each middleman daily
class FarmTradedBirdsInfectedTracker: public BaseCrossSectionalTracker {
public:
    FarmTradedBirdsInfectedTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {MARKET_OPENING_HOUR + 1}, const uint& tmin = 0 );
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
};



//=== Prints counts of birds bought in each market daily, broken down by origin farm
class MmanTradedBirdsByFarmTracker: public BaseCrossSectionalTracker {
public:
    MmanTradedBirdsByFarmTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {MARKET_OPENING_HOUR + 1}, const uint& tmin = 0 );
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
};

//=== Prints counts of infected birds bought in each market daily
class MmanTradedBirdsInfectedTracker: public BaseCrossSectionalTracker {
public:
    MmanTradedBirdsInfectedTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const std::vector<uint>& hs = {MARKET_OPENING_HOUR + 1}, const uint& tmin = 0 );
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
};


//=== Prints counts of exposed and infectious birds
class InfectedTradedBirdsTracker: public BaseCrossSectionalTracker {
public:
    InfectedTradedBirdsTracker( NamedOfstream outFile, const uint& deltaTdays = 1, const uint& tmin = 0 );
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true  ) override;
};


//====== One-shot cross-sectional trackers ======//

// These trackers are called only once for each simulation

// Prints informations about vendors
// N.B. can be print only once per batch of simulations
// (since demography is the same for each simulation in the same batch)
class InitialVendorDemographyPrinter: public BaseCrossSectionalTracker {
public:
    InitialVendorDemographyPrinter( NamedOfstream outFile );
    ~InitialVendorDemographyPrinter() override {};
    void collectInfo( WorldSimulator* simulator ) override;
    void reset( bool updateStream = true ) override;
};


// Prints informations about vendors
// N.B. can be print only once per batch of simulations
// (since demography is the same for each simulation in the same batch)
class InitialMiddlemanDemographyPrinter: public BaseCrossSectionalTracker {
public:
    InitialMiddlemanDemographyPrinter( NamedOfstream outFile ) ;
    ~InitialMiddlemanDemographyPrinter() override {} ;
    void collectInfo( WorldSimulator* simulator ) override ;
    void reset( bool updateStream = true ) override ;
};

//=== Samples extant lineages in markets and prints phylogenetic tree
// N.B. does homochronous sampling only
class WritePhylogenyHomochronousMarketSampling: public BaseCrossSectionalTracker {
public:
    WritePhylogenyHomochronousMarketSampling( NamedOfstream outFile, uint tSample, Random* randGen, uint chickensSampledPerMarket, std::vector<uint> marketsSampledIdxs ) ;
    ~WritePhylogenyHomochronousMarketSampling() {} ;
    void collectInfo( WorldSimulator* simulator ) override ;
    void reset( bool updateStream = true ) override ;
private:
    Random* randGen ;
    uint chickensSampledPerMarket ;
    uint tSample ;
    std::vector<uint> marketsSampledIdxs ;
    
} ;


//=== Samples extant lineages in markets and prints phylogenetic tree
// N.B. allows heterochronous sampling

class WritePhylogenyHeterochronousMarketSampling: public BaseCrossSectionalTracker {
public:
    WritePhylogenyHeterochronousMarketSampling( NamedOfstream outFile, Random* randGen, std::vector<uint> samplingTimes, std::vector<uint> marketsSampledIdxs, std::vector<uint> birdSampledAmounts ) ;
    ~WritePhylogenyHeterochronousMarketSampling() {} ;
    void collectInfo( WorldSimulator* simulator ) override ;
    void reset( bool updateStream = true ) override ;
private:
    Random* randGen ;
    uint currentEventIdx ;
    uint nEvents ;
    std::vector<uint> samplingTimes ;
    std::vector<uint> marketsSampledIdxs ;
    std::vector<uint> birdSampledAmounts ;
    
} ;



#endif /* cross_sectional_tracker_hpp */
