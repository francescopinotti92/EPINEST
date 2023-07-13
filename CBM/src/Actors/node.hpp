//
//  node.hpp
//  CBM
//
//

#ifndef node_hpp
#define node_hpp

#include "baseNode.hpp"
#include "AreaManager.hpp"
#include "birdTransferManager.hpp"
#include "edge_manager.hpp"
#include "weighted_set.hpp"
#include "farm.hpp"
#include "middleman.hpp"
#include "market.hpp"
#include "vendor.hpp"
#include "fieldexperimenter.hpp"
#include "clock.hpp"
#include "checks.hpp"

class BirdTransferManager;
class CompletedCropTrackerManager;
class AreaManager;
class Farm;


//====== forward declarations ======//

class StateFarm;
class StateMman;
class StateMarket;
class StateVendor;
class StateExperimenter;

class StateMmanAwaiting;
class StateVendorSell;
class StateWSWholesaleSell;

class NodeFarm;
class NodeMman;
class NodeMarket;
class NodeVendor;
class NodeFieldExperimenter;



// forward declare bird transferer class

//====== Declare transfer functions ======//

//void transferBirds(NodeFarm* nf, NodeMman* nmm, const BirdType& bt, const uint& nbirds, Random& randGen);
//void transferBirds(NodeMman* nmm, NodeMarket* nm, const BirdType& bt, const uint& nbirds, Random& randGen);
//void transferBirdsWithinMarket(NodeVendor* nv1, NodeVendor* nv2, NodeMarket* nm, uint nbirds, Random& randGen);
//void transferBirdsToCommunity(NodeVendor* nv, NodeMarket* nm, uint nbirds);


//====== Node Farm ======//

class NodeFarm: public BaseNode {
public:
    NodeFarm( Farm* farm_, StateFarm* state_, const Clock& clock );
    NodeFarm( Farm* farm_, StateFarm* state_, uint area_, Geom::Point pos, const Clock& clock );
    
    // handle external requests
    void refillRequest( const Clock& clock, Random& randGen, ChickenPool* birdMngr );
    void stageCropRequest( const Clock& clock, Random& randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr );
    //void sellRequest( const Clock& clock, Random& randGen );
    void scheduleTodayTradeRequest( const Clock& clock, Random& randGen, BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr );
    uint evalBatchSizeRequest( const BirdType& bt, const uint& amount );
    void isEmptyRequest( AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr );
    bool hasEndedRolloutRequest();
    void resetRequest( ChickenPool* birdMngr );
    
    
    bool isEmpty();
    bool doesRolloutTrade() const                               { return performsRolloutTrade; }
    //void addNbr(NodeMman* nbrMman, const BirdType& bt, const double& weight);
    //void addNbr(NodeMarket* nbrMarket);
    //void addNbrKernel(NodeFarm* nbrFarm, const double& weight);
    void setState( StateFarm* nextState)                         { state = nextState; }
    void reset( ChickenPool* birdMngr ) override;
    void setAverageFarmSize( const double& size )               { averageFarmSize = size; }
    void setRolloutBeginTime( const uint& day )                 { dayRolloutBegin = day; }
    void setNextCycleBeginTime( const uint& day )               { dayNextCycleBegin = day; }
    void setBaseWeight( double weight ) override                { farm->setBaseWeight( weight ); }
    void increaseTotalAvailBirdsToday( const BirdType& bt, const uint& amount );
    void reduceTotalAvailBirdsToday( const BirdType& bt, const uint& amount );
    void resetTotalAvailBirdsToday();
    uint evalBatchSizeFromMman( const BirdType& bt, const uint& amount );
    bool hasAvailableBirds( const BirdType& bt );
    //void removeBirdsFromCrop( Crop& crop, ChickenPool* birdMngr );
    

    std::function<bool (const Clock&, Random& randGen, ChickenPool* birdMngr)> addCrops;
    //std::function<void (const Clock& clock, Random& randGen)> sellBirds2Middlemen;
    std::function<void (const Clock& clock, Random& randGen, BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr)> scheduleTodayTrade;
    std::function<void (const Clock& clock, Random& randGen)> scheduleNextCycle;

    Farm* getContext()                                          { return farm;       }
    HoldsBirds* getUnderlyingSetting() override                 { return farm;       }
    //weighted_set<NodeFarm>& getNbrFarms()                       { return farmKernelDistanceWeightsVec; }
    uint getSizeBirds() override                                { return farm->getSizeBirds(); }
    uint getSizeInfectedBirds() override                        { return farm->getSizeBirdsInf(); }
    uint getDayRolloutBegin() const                             { return dayRolloutBegin; }
    uint getDayNextCycleBegin() const                           { return dayNextCycleBegin;   }
    NodeType getNodeType() override                             { return NodeType::farm; }
    uint getAreaId()                                            { return areaId; }
    uint getSizeAvailBirds( const BirdType& bt )                { return totalAvailBirdsToday[bt]; }
    uint getNumberBirdsDiscarded() const                        { return nBirdsDiscarded; }
    uint getNumberBirdsProduced()  const                        { return farm->getNBirdsCreated(); }
    double getAverageFarmSize() const                           { return averageFarmSize; }
    double getBaseWeight() override                             { return farm->getBaseWeight(); }

    bool hasInfectedBirds();

    void configCommercialFarm( const BirdType& bt, const uint& batchSize, const double& p_refill, const uint& ndays_rollout );
    void configCommercialFarm( const BirdType &bt, const uint &batchSize, const double& pNB, const double& nNB, const uint& ndays_rollout );

    
    //std::vector<weightedElem<NodeFarm>>::const_iterator beginFarmNbrs() { return farmKernelDistanceWeightsVec.cbegin(); }
    //std::vector<weightedElem<NodeFarm>>::const_iterator endFarmNbrs() { return farmKernelDistanceWeightsVec.cend(); }    
private:
    Farm* farm;
    StateFarm* state;
    
    // actual functions (usually bound to function pointers
    uint drawSingleCropFixedProb( const double& p_refill, Random& randGen );
    bool addSingleCrop( const BirdType& bt, const uint& batchSize, const Clock& clock, Random& randGen, ChickenPool* birdMngr );
    //bool addSingleCropFixedProb( const BirdType& bt, const uint& batchSize, const double& p_refill, const Clock& clock, Random& randGen );
    void scheduleNextCycleGeom1( const double& p_refill, const Clock& clock, Random& randGen );
    void scheduleNextCycleNegBinom1( const double& p, const double& n, const Clock& clock, Random& randGen );

    void scheduleSingleBatchRolloutDaily( const double& p_sold_daily, const uint& maxDaysSinceRollout, const Clock& clock, Random& randGen, BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr );
    //void sellSingleBatchRollout( const double& p_sold_daily, const Clock& clock, Random& randGen );

    // additional properties
    double averageFarmSize;
    uint areaId;
    uint dayRolloutBegin;
    uint nBirdsDiscarded;
    uint dayNextCycleBegin;
    bool performsRolloutTrade;
    std::unordered_map<BirdType, uint> totalAvailBirdsToday;
};

//====== Node Middleman ======//

struct CollectionTask {
    CollectionTask(NodeFarm* nf_, const uint& nmId_, const uint& nbirds_, const BirdType& bt_, const uint& t_ = 0): nf(nf_), nmId( nmId_ ), nbirds( nbirds_ ), bt( bt_ ), t( t_ ) {};
    NodeFarm* nf;
    uint nmId;
    BirdType bt;
    uint nbirds;
    uint t;
};

bool compareCollectionTaskTiming( const CollectionTask& left, const CollectionTask& right );
bool compareCollectionTaskTimingPtr( const CollectionTask* left, const CollectionTask* right );


class NodeMman: public BaseNode {
public:
    NodeMman(Middleman* mman_, StateMman* state_);
    
    void configSimpleMiddleman( const uint& nVisitedMarkets, const uint& nVisitedAreas, const double& probChangeArea );
    
    std::function<void ( Random&, AreaManager& )> contactTradingFarms;
    std::function<bool ( Random& )> decideToMoveToNewAreas;
    std::function<void ( Random&, AreaManager& )> moveToNewAreas;
    //std::function<uint (NodeFarm*, const BirdType&, const uint&)> acceptedBatchSize;
    std::function<void ( Random&, AreaManager& )> buyFromFarms;
    std::function<void (const Clock&, Random&)> scheduleTasks;
    std::function<void (Random&, BirdTransferManager&, std::vector<NodeMarket*>& )> sellToMarkets;
        
    //void addNbr(NodeMarket* nm, const BirdType& bt, const double& weight);
    void addCollectionTask(NodeFarm* nf, const uint& nmId, const BirdType& bt, const uint& nbirds);
    void processCollectionTask(CollectionTask* task, Random& randGen, BirdTransferManager& transferManager);
    void processCurrentCollectionTasks(const Clock& clock, Random& randGen, BirdTransferManager& transferManager);
    void clearCollectionTasks();
    bool hasCompletedCollectionTasks();

    //uint processBatchRequest(NodeFarm* nf, const BirdType& bt, const uint& nbirds);
    void moveToNewAreasRequest( Random& randGen, AreaManager& areaMngr );
    void forceMoveToNewAreasRequest( Random& randGen, AreaManager& areaMngr );
    void buyFromFarmsRequest( Random& randGen, AreaManager& areaMngr );
    void executeScheduleTasksRequest( const Clock& clock, Random& randGen );
    void executeCollectionTasksRequest( const Clock& clock, Random& randGen, BirdTransferManager& transferManager );
    void sellToMarketsRequest( Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList );
    void resetRequest( ChickenPool* birdMngr );
    
    void setState(StateMman* nextState);
    void setState(StateMmanAwaiting* stateAwait);
    void setBaseWeight( double weight ) override                { mman->setBaseWeight( weight ); }
    
    uint getNtasks();
    NodeType getNodeType()  override                            { return NodeType::middleman; }
    uint getSizeBirds()     override                            { return mman->getSizeBirds(); }
    uint getSizeInfectedBirds() override                        { return mman->getSizeBirdsInf(); }
    uint getExpectedNumberMarketsToVisit()                      { return nMarketsToVisitDaily; }
    double getBaseWeight() override                             { return mman->getBaseWeight(); }
    uint getNIncomingInfectedBirds()                            { return nIncomingInfectedBirds ; }



    void reset( ChickenPool* birdMngr ) override;

    Middleman* getContext();
    HoldsBirds* getUnderlyingSetting() override                 { return mman;       }

    const std::vector<BirdType>& getTradedBirdType() const { return mman->getTradedBirdTypes(); }
    
    void doRecordIncomingInfectedBirds( bool checkExposed = true ) ;
    void addFarmTradedBird( pBird& bird ) ;
    
    
private:
    
    Middleman* mman ;
    StateMman* state ;
    std::vector<uint> areasIds ;
    std::unordered_map<uint, std::vector<std::pair<uint, uint>>> orderTaskList ;
    std::vector<CollectionTask*> collectionTaskVec ;
    //uint provisionalCargo;
    uint currCollectionTask ;
    uint nMarketsToVisitDaily ;
    uint nIncomingInfectedBirds ;
    
    std::vector<std::function<void( pBird& bird )>> trackersFarmTradedBird ;
    void trackInfectedBird( pBird& bird ) ;
    void trackExposedAndInfectedBird( pBird& bird ) ;



    // functions to modify orderTaskList
    void addOrder( const uint& areaID, const uint& marketID, const uint& nbirds );
    void clearOrders();
    std::vector<std::pair<uint,uint>>& getOrdersSingleArea( const uint areaID );
    
    // actual functions
    void buyFromFarmsSingleSpecies( const uint& nMarketsToVisit, Random& randGen, AreaManager& areaMngr );
    bool decideToMoveFixedProb( const double& probMove, Random& randGen );
    void moveToNewAreasNeighboring( const uint& nAreas, Random& randGen, AreaManager& areaMngr );
    //uint evalBatchSizeCapacity(NodeFarm* nf, const BirdType&, const uint& nbirds);
    void scheduleCollectionTasksRandom( const Clock& clock, Random& randGen );
    //void sellFullCargoToMarkets(const uint& nVisitedMarkets, Random& randGen, BirdTransferManager& transferManager);
    void sellFullCargoToMarkets( Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList );

};

//====== Node Market ======//

class NodeMarket: public BaseNode {
public:
    NodeMarket(Market* market_, StateMarket* state_, std::vector<bool> openDays_ = std::vector<bool>(MARKET_NDAYS_CYCLE, true) );
    
    void configureLayersSingleSpecies( const BirdType& bt, const uint& nLayers, const double& propBirdsFromMiddlemenToWholesalers );
    
    void doTradingStepRequest( const Clock& clock, Random& randGen, BirdTransferManager& transferManager );
    void doTradingStep( const Clock& clock, Random& randGen, BirdTransferManager& transferManager );
    
    void doVendorDisplacementStep( const uint& layer, Random& randGen );
    void doWholesaleTradingStep( const uint& layer, const Clock& clock, Random& randGen, BirdTransferManager& transferManager );
    void resetRequest( ChickenPool* birdMngr );

    //void forceVendorSellingRequest( Random& randGen );
    //void forceVendorSelling( Random& randGen );
    
    void reset( ChickenPool* birdMngr ) override;
    
    void openMarket(const Clock &clock);
    void closeMarket();
    
    void addVendor(NodeVendor* vendor);
    void removeVendor(NodeVendor* vendor);
    
    void addBuyerWS(NodeVendor* vendor);
    void addBuyerR(NodeVendor* vendor);
    void addSellerWS(NodeVendor* vendor);
    void addSellerR(NodeVendor* vendor);

    void removeBuyerWS(NodeVendor* vendor);
    void removeBuyerR(NodeVendor* vendor);
    void removeSellerWS(NodeVendor* vendor);
    void removeSeller(NodeVendor* vendor);
    
    bool isWorkingDay(const Clock& clock);
    
    bool isOpen();
    inline void setState(StateMarket* nextState)                                { state = nextState; }
    void setBaseWeight( double weight ) override                                { market->setBaseWeight( weight ); }
    inline Market* getContext()                                                 { return market;     }
    HoldsBirds* getUnderlyingSetting() override                                 { return market;     }

    void printBuyersDemography();
    
    std::vector<NodeVendor*>& getSellersWholesalers(const BirdType& bt, const uint& layer);
    std::vector<NodeVendor*>& getBuyersWholesalers(const BirdType& bt, const uint& layer);
    std::vector<NodeVendor*>& getBuyersRetailers(const BirdType& bt, const uint& layer);

    inline std::vector<NodeVendor*>& getSellers(const BirdType& bt)             { return seller_list[bt]; }

    NodeType getNodeType() override                                 { return NodeType::market ; }
    uint getSizeBirds()    override                                 { return market->getSizeBirds() ; }
    uint getSizeInfectedBirds() override                            { return market->getSizeBirdsInf() ; }
    double getPropSoldMmenToWholesalers(const BirdType& bt)         { return propBirdsMmen2Wholesalers[bt] ; }
    double getBaseWeight() override                                 { return market->getBaseWeight() ; }
    std::map<uint,uint> getNMmanTradedBirdsByFarm()                 { return nMmanTradedBirdsByFarm ; }
    uint getNIncomingInfectedBirds()                                { return nIncomingInfectedBirds ; }
    void setPropSoldMmenToWholesalers( const BirdType& bt, const double& prop );
    
    void doRecordBirdSource() ;
    void doRecordIncomingInfectedBirds( bool checkExposed = true ) ;
    void addMmanTradedBird( pBird& bird ) ;
    //std::function<void( pBird& bird )> addMmanTradedBird ;


private:
    Market* market;
    StateMarket* state;
    std::vector<NodeVendor*> vendorPartners;
    
    std::map<BirdType, uint> maxLayers;
    std::map<BirdType, double> propBirdsMmen2Wholesalers;
    std::map<BirdType, std::vector<std::vector<NodeVendor*>>> buyer_wholesale_stack;
    std::map<BirdType, std::vector<std::vector<NodeVendor*>>> buyer_retail_stack;
    std::map<BirdType, std::vector<std::vector<NodeVendor*>>> seller_wholesale_stack;
    std::map<BirdType, std::vector<NodeVendor*>> seller_list;
    
    std::vector<bool> openDays;  // opening days are marked as true
    
    
    
    // track chicken flux
    std::vector<std::function<void( pBird& bird )>> trackersMmanTradedBird ;
    void doNothing( pBird& bird ) {};
    void trackFarmOrigin( pBird& bird ) ;
    void trackInfectedBird( pBird& bird ) ;
    void trackExposedAndInfectedBird( pBird& bird ) ;

    std::map<uint,uint> nMmanTradedBirdsByFarm ;
    
    // other
    uint nIncomingInfectedBirds;
    
};

class NodeVendor: public BaseNode {
public:
    NodeVendor(Vendor* vendor_, StateVendor* state_, uint dailyTarget, uint vendorLayer, NodeMarket* marketBuy_ = nullptr, NodeMarket* marketSell_ = nullptr);
    
    // configurers
    void configSimpleRetailer( const double& p_empty, const double& odUnsold, const double& propUnsold, const bool& prioritizesRepurposed );
    void configSimpleWholesaler( const double& p_empty, const double& odUnsold, const double& propUnsold, const double& p_wholesalers, const double& p_retailers, const bool& prioritizesRepurposed );

    // handle external requests
    void goHomeRequest();
    void goToMarketRequest( Random& randGen );
    void switchToSellingRequest( Random& randGen );
    void doWholesaleSellStepRequest( const Clock& clock, Random& randGen, BirdTransferManager& transferManager );
    //void evalSellingRoleRequest( Random& randGen );
    //void forceSellingRoleRequest( Random& randGen );
    uint processBatchFromMmanRequest(const uint& nbirdRequest);
    uint processBatchFromVendorRequest(const uint& nbirdRequest);
    void doSellStepRequest( const Clock& clock, Random& randGen, BirdTransferManager& transferManager );
    void resetRequest( ChickenPool* birdMngr );
    
    std::function<void ( const Clock&, Random&, BirdTransferManager& )> doWholesaleSellStep;
    void sellStepRetail( const Clock& clock, Random& randGen, BirdTransferManager& transferManager );
    void sellStepWholesale( const Clock& clock, Random& randGen, BirdTransferManager& transferManager );

    std::function<void ( Random& )> scheduleDailyTrade;
    std::function<std::pair<uint,uint>( const uint& )> getTransactionDetails;

    void withdrawFromMarket();
    uint evalBatchSizeTarget( const uint& nbirdsRequest)  ;
    uint evalRetailPurchaseRequest( const uint& nBirdsRequest ) ;
    
    
    std::function<void ()> goToMarketBuying;
    std::function<void ()> goToMarketSelling;
    std::function<void ()> goHomeBuyer;
    
    void goToMarketBuyingWS();
    void goToMarketBuyingR();
    void goToMarketSellingWS();
    void goToMarketSellingR();

    void goHomeBuyerWS();
    void goHomeBuyerR();
    void goHomeSeller();
    
    void goToRetail();
    
    void reset( ChickenPool* birdMngr ) override;
    
    void setState(StateVendor* nextState)                       { state = nextState;  }
    void setBaseWeight( double weight ) override                { vendor->setBaseWeight( weight ); }

    void increaseBirdBoughtCount( const uint& nbirds );
    void decreaseBirdRetailCount( const uint& nbirds );
    void resetBirdBoughtCount();
    
    Vendor* getContext()                                        { return vendor;      }
    HoldsBirds* getUnderlyingSetting() override                 { return vendor;       }
    NodeMarket* getMarketBuying()                               { return marketBuy;   }
    NodeMarket* getMarketSelling()                              { return marketSell;  }
    uint getDailyTarget()                                       { return dailyTarget; }
    uint getVendorLayer()                                       { return vendorLayer; }
    std::function<ActorType ()> getVendorType;
    
    NodeType getNodeType()  override                            { return NodeType::vendor; }
    uint getSizeBirds()     override                            { return vendor->getSizeBirds(); }
    uint getSizeInfectedBirds() override                        { return vendor->getSizeBirdsInf(); }
    double getBaseWeight() override                             { return vendor->getBaseWeight(); }

private:
    Vendor* vendor;
    StateVendor* state;
    NodeMarket* marketBuy;
    NodeMarket* marketSell;
    NodeMarket* marketCurrent;
    uint vendorLayer;
    uint dailyTarget;
    uint nBirdsWholesaleToWholesalers;      // # of birds to be sold to wholesalers (wholesalers only)
    uint nBirdsWholesaleToRetailers;        // # of birds to be sold to retailers   (wholesalers only)
    uint nBirdsRetail;                      // # of birds to be sold to end community
    
    uint nBirdsBoughtToday;
    
    void scheduleDailyTradeNegBinVendor(const double& p_empty, const double& odUnsold, const double& propUnsold, Random& randGen);
    void scheduleDailyTradeNegBinWS(const double& p_empty, const double& odUnsold, const double& propUnsold, const double& p_wholesalers, const double& p_retailers, Random& randGen);
    std::pair<uint,uint> getTransactionDetailsPriority( const uint& amount );
    std::pair<uint,uint> getTransactionDetailsNoPriority( const uint& amount );

    ActorType returnRetailerType();
    ActorType returnWholesalerType();

};

class NodeFieldExperimenter: public BaseNode {
public:
    NodeFieldExperimenter( StateExperimenter* state, BirdType birdType_, uint nRecruitsFarm_, uint nRecruitsMarket_, uint dtShelter_, uint dayExperiment_, uint dtExperiment_, NodeMarket* marketExperiment_, std::vector<NodeFarm*>* availableFarms_ ) ;
    
    //void updateRequest( const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) ;
    void goToNextStageExperimentRequest( Clock& clock, Random& randGen, BirdTransferManager& transferManager ) ;
    void resetRequest( ChickenPool* birdMngr );

    
    void goToMarket( Random& randGen, BirdTransferManager& transferManager ) ;
    void goHome() ;
    void recruitBirdsFarms( Clock& clock, Random& randGen, BirdTransferManager& transferManager ) ;
    void recruitBirdsMarket( Random& randGen, BirdTransferManager& transferManager ) ;
    void checkInfection( Clock& clock ) ;
    void reset( ChickenPool* birdMngr ) override;
    
    void setState( StateExperimenter* nextState )               { state = nextState ; }
    void setBaseWeight( double weight ) override                { fieldExperimenter->setBaseWeight( weight ) ; }

    void increaseBirdBoughtCount( const uint& nbirds );
    void resetBirdBoughtCount();
    
    FieldExperimenter* getContext()                             { return fieldExperimenter ; }
    HoldsBirds* getUnderlyingSetting() override                 { return fieldExperimenter ; }
    
    NodeType getNodeType()  override                            { return NodeType::fieldexperimenter ; }
    uint getSizeBirds()     override                            { return fieldExperimenter->getSizeBirds(); }
    uint getSizeInfectedBirds() override                        { return fieldExperimenter->getSizeBirdsInf(); }
    double getBaseWeight() override                             { return fieldExperimenter->getBaseWeight(); }
    
    uint getTimeFarmRecruit()       { return tRecruitFarm ; }
    uint getTimeStartExperiment()   { return tStartExperiment ; }
    uint getTimeStopExperiment()    { return tStopExperiment ; }


private:
    FieldExperimenter* fieldExperimenter ;
    StateExperimenter* state ;
    BirdType birdType ;
    uint nRecruitsFarm ;
    uint nRecruitsMarket ;
    uint dtShelter ;
    uint dayExperiment ;
    uint dtExperiment ;
    uint tRecruitFarm ;
    uint tStartExperiment ;
    uint tStopExperiment ;
    NodeMarket* marketExperiment ;
    std::vector<NodeFarm*>* availableFarms ;
    
    // output quantities
    std::unordered_map<BirdID,uint,hashBirdID> firstInvasionCheckPoint;

} ;


//====== States ======//
/*
 Each instance of NodeFarm, NodeMman, NodeMarket and NodeVendor holds
 one state object. Essentially, a node's state defines how the node reacts
 to internal and/or external events.
 */

//====== Farm states ======//

class StateFarm {
public:
    StateFarm() = default;
    virtual ~StateFarm() = default;
    virtual void onRefillRequest( NodeFarm* nf, const Clock& clock, Random& randGen, ChickenPool* birdMngr ) = 0;
    virtual void onStageCropRequest( NodeFarm* nf, const Clock& clock, Random& randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr ) = 0;
    //virtual void onSellRequest( NodeFarm* nf, const Clock& clock, Random& randGen ) = 0;
    virtual uint onEvalBatchSizeRequest( NodeFarm* nf, const BirdType& bt, const uint& amount ) = 0;
    virtual void onScheduleTodayTradeRequest( NodeFarm* nf, const Clock& clock, Random& randGen,  BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr ) = 0;
    virtual void onIsEmptyRequest( NodeFarm* nf, AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr ) = 0;
    virtual bool onHasEndedRolloutRequest( NodeFarm* nf ) = 0;
    virtual void onResetRequest( NodeFarm* nf, ChickenPool* birdMngr ) = 0;
};

// simple state sequence for commercial farms
// Empty --> Raising --> Trading --> Empty --> ...
class StateFarmEmpty;
class StateFarmRaising;
class StateFarmTrading;

class StateFarmEmpty: public StateFarm {
public:
    void onRefillRequest( NodeFarm* nf, const Clock& clock, Random& randGen, ChickenPool* birdMngr )                            override;
    void onStageCropRequest( NodeFarm* nf, const Clock& clock, Random& randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr ) override;
    //void onSellRequest( NodeFarm* nf, const Clock& clock, Random& randGen)      override;
    uint onEvalBatchSizeRequest( NodeFarm* nf, const BirdType& bt, const uint& amount ) override;
    void onScheduleTodayTradeRequest( NodeFarm* nf, const Clock& clock, Random& randGen,  BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr )               override;
    void onIsEmptyRequest( NodeFarm *nf, AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr ) override;
    bool onHasEndedRolloutRequest( NodeFarm* nf ) override;
    void onResetRequest( NodeFarm* nf, ChickenPool* birdMngr )                                                                 override;
    void setStateRaising(StateFarmRaising* stateRaising);
private:
    StateFarmRaising* stateRaising;
};

class StateFarmRaising: public StateFarm {
public:
    void onRefillRequest( NodeFarm* nf, const Clock& clock, Random& randGen, ChickenPool* birdMngr )                            override;
    void onStageCropRequest( NodeFarm* nf, const Clock& clock, Random& randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr ) override;
    //void onSellRequest( NodeFarm* nf, const Clock& clock, Random& randGen)      override;
    uint onEvalBatchSizeRequest( NodeFarm* nf, const BirdType& bt, const uint& amount )                 override;
    void onScheduleTodayTradeRequest( NodeFarm* nf, const Clock& clock, Random& randGen,  BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr )               override;
    void onIsEmptyRequest( NodeFarm *nf, AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr )                                        override;
    bool onHasEndedRolloutRequest( NodeFarm* nf )                                                       override;
    void onResetRequest( NodeFarm* nf, ChickenPool* birdMngr )                                                                 override;
    void setStateEmpty( StateFarmEmpty* stateEmpty );
    void setStateTrading( StateFarmTrading* stateTrading );
private:
    StateFarmEmpty* stateEmpty;
    StateFarmTrading* stateTrading;
};

class StateFarmTrading: public StateFarm {
public:
    void onRefillRequest( NodeFarm* nf, const Clock& clock, Random& randGen, ChickenPool* birdMngr )                            override;
    void onStageCropRequest( NodeFarm* nf, const Clock& clock, Random& randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr )  override;
    //void onSellRequest( NodeFarm* nf, const Clock& clock, Random& randGen)      override;
    uint onEvalBatchSizeRequest( NodeFarm* nf, const BirdType& bt, const uint& amount )                 override;
    void onScheduleTodayTradeRequest( NodeFarm* nf, const Clock& clock, Random& randGen,  BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr )               override;
    void onIsEmptyRequest( NodeFarm* nf, AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr )                                        override;
    bool onHasEndedRolloutRequest( NodeFarm* nf )                                                       override;
    void onResetRequest( NodeFarm* nf, ChickenPool* birdMngr )                                                                 override;
    void setStateEmpty(StateFarmEmpty* stateEmpty) ;
private:
    StateFarmEmpty* stateEmpty;
};

//====== Middleman states ======//

class StateMman {
public:
    StateMman() = default;
    virtual ~StateMman() = default;
    //virtual uint onProcessBatchRequest( NodeMman* nmm, NodeFarm* nf, const BirdType& bt, const uint& nbirds ) = 0;
    virtual void onMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) = 0;
    virtual void onForceMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) = 0;
    virtual void onBuyFromFarmsRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) = 0;
    virtual void onExecuteScheduleTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen ) = 0;
    virtual void onExecuteCollectionTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) = 0;
    virtual void onSellToMarketsRequest( NodeMman* nmm, Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) = 0;
    virtual void onResetRequest( NodeMman* nmm, ChickenPool* birdMngr ) = 0;
};

class StateMmanAwaiting;
class StateMmanCollecting;
class StateMmanSelling;

class StateMmanAwaiting: public StateMman {
public:
    //uint onProcessBatchRequest( NodeMman* nmm, NodeFarm* nf, const BirdType& bt, const uint& nbirds ) override;
    void onMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onForceMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onBuyFromFarmsRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onExecuteScheduleTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen ) override;
    void onExecuteCollectionTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    void onSellToMarketsRequest( NodeMman* nmm, Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) override;
    void onResetRequest( NodeMman* nmm, ChickenPool* birdMngr ) override;
    void setStateCollect( StateMmanCollecting* stateCollect );
    void setStateSell( StateMmanSelling* stateSell );
private:
    StateMmanCollecting* stateCollect;
    StateMmanSelling* stateSell;
};

class StateMmanCollecting: public StateMman {
public:
    //uint onProcessBatchRequest( NodeMman* nmm, NodeFarm* nf, const BirdType& bt, const uint& nbirds ) override;
    void onMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onForceMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onBuyFromFarmsRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onExecuteScheduleTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen ) override;
    void onExecuteCollectionTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    void onSellToMarketsRequest( NodeMman* nmm, Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) override;
    void onResetRequest( NodeMman* nmm, ChickenPool* birdMngr ) override;
    void setStateAwait( StateMmanAwaiting* stateAwait );
    void setStateSell( StateMmanSelling* stateSell );
private:
    StateMmanAwaiting* stateAwait;
    StateMmanSelling* stateSell;
};

class StateMmanSelling: public StateMman {
public:
    //uint onProcessBatchRequest( NodeMman* nmm, NodeFarm* nf, const BirdType& bt, const uint& nbirds ) override;
    void onMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onForceMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onBuyFromFarmsRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) override;
    void onExecuteScheduleTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen ) override;
    void onExecuteCollectionTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    void onSellToMarketsRequest( NodeMman* nmm, Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) override;
    void onResetRequest( NodeMman* nmm, ChickenPool* birdMngr ) override;
    void setStateAwait( StateMmanAwaiting* stateAwait );
private:
    StateMmanAwaiting* stateAwait;
};


//====== Market states ======//

class StateMarketOpen;
class StateMarketClose;

class StateMarket {
public:
    virtual ~StateMarket() = default;
    virtual void open(NodeMarket& nma, const Clock& clock) = 0;
    virtual void close(NodeMarket& nma, const Clock& clock) = 0;
    virtual void onDoTradingStepRequest(NodeMarket* nm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager) = 0;
    //virtual void onForceVendorSellingRequest( NodeMarket* nm, Random& randGen ) = 0;
    virtual bool isOpen() = 0;
    virtual void onResetRequest( NodeMarket* nm, ChickenPool* birdMngr ) = 0;
};

class StateMarketOpen: public StateMarket {
public:
    StateMarketOpen();
    void open(NodeMarket& nma, const Clock& clock) override;
    void close(NodeMarket& nma, const Clock& clock) override;
    void onDoTradingStepRequest(NodeMarket* nm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager) override;
    //void onForceVendorSellingRequest( NodeMarket* nm, Random& randGen ) override;
    bool isOpen() override                                      { return true; }
    void onResetRequest( NodeMarket* nm, ChickenPool* birdMngr ) override;
    void setStateClose(StateMarketClose* stateClose);
private:
    StateMarketClose* stateClose;
};

class StateMarketClose: public StateMarket {
public:
    StateMarketClose();
    void open(NodeMarket& nma, const Clock& clock) override;
    void close(NodeMarket& nma, const Clock& clock) override;
    void onDoTradingStepRequest(NodeMarket* nm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager) override;
    //void onForceVendorSellingRequest( NodeMarket* nm, Random& randGen ) override;
    bool isOpen() override                                      { return false; }
    void onResetRequest( NodeMarket* nm, ChickenPool* birdMngr ) override;
    void setStateOpen(StateMarketOpen* stateOpen);
private:
    StateMarketOpen* stateOpen;
};

//====== Vendor states ======//


// old states
class StateVendorAtHome;
class StateVendorBuy;
class StateVendorSell;

class StateVendor {
public:
    virtual ~StateVendor() = default;
    
    virtual void onGoHomeRequest( NodeVendor* nv ) = 0;
    virtual void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) = 0;
    virtual void onSwitchToSellingRequest( NodeVendor* nv, Random& randGen ) = 0;
    virtual void onDoWholesaleSellStepRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) = 0;
    //virtual void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) = 0;
    //virtual void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) = 0;
    virtual uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) = 0;
    virtual uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) = 0;
    virtual void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) = 0;
    virtual void onResetRequest( NodeVendor* nv, ChickenPool* birdMngr ) = 0;
    
    virtual bool isWorking() = 0;
};

//=== simple vendor states ===//

class StateVendorAtHome: public StateVendor {
public:
    
    void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) override;
    void onGoHomeRequest( NodeVendor* nv ) override;
    void onSwitchToSellingRequest( NodeVendor* nv, Random& randGen ) override;
    void onDoWholesaleSellStepRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    //void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    //void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) override;
    uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) override;
    void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    void onResetRequest( NodeVendor* nv, ChickenPool* birdMngr ) override;
    
    bool isWorking() override                                   { return false; }
    
    void setStateBuy(StateVendorBuy* stateBuy);
    void setStateSell(StateVendorSell* stateSell);
private:
    StateVendorBuy* stateBuy;
    StateVendorSell* stateSell;
};

class StateVendorBuy: public StateVendor {
public:
    void onGoHomeRequest( NodeVendor* nv ) override;
    void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) override;
    void onSwitchToSellingRequest( NodeVendor* nv, Random& randGen ) override;
    void onDoWholesaleSellStepRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    //void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    //void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) override;
    uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) override;
    void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    void onResetRequest( NodeVendor* nv, ChickenPool* birdMngr ) override;

    bool isWorking() override                                   { return true; }
   
    void setStateHome(StateVendorAtHome* stateHome);
    void setStateSell(StateVendorSell* stateSell);

private:
    StateVendorAtHome* stateHome;
    StateVendorSell* stateSell;
};

class StateVendorSell: public StateVendor {
public:
    void onGoHomeRequest( NodeVendor* nv ) override;
    void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) override;
    void onSwitchToSellingRequest( NodeVendor* nv, Random& randGen ) override;
    void onDoWholesaleSellStepRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    //void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    //void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) override;
    uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) override;
    void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    void onResetRequest( NodeVendor* nv, ChickenPool* birdMngr ) override;

    bool isWorking() override                                   { return true; }
    void setStateHome(StateVendorAtHome* stateHome);
private:
    StateVendorAtHome* stateHome;
};


//====== FieldExperimenter states ======//

class StateExperimenterRecruiting;
class StateExperimenterSheltering;
class StateExperimenterMarketStage;

class StateExperimenter {
public:
    virtual ~StateExperimenter() = default;
 
    virtual void onGoToNextStageRequest( NodeFieldExperimenter* nfe, Clock& clock, Random& randGen, BirdTransferManager& transferMngr ) = 0 ;
    virtual void onResetRequest( NodeFieldExperimenter* nfe, ChickenPool* birdMngr ) = 0 ;
} ;

class StateExperimenterRecruiting: public StateExperimenter {
public:
    
    void onGoToNextStageRequest( NodeFieldExperimenter* nfe, Clock& clock, Random& randGen, BirdTransferManager& transferMngr ) override ;
    void onResetRequest( NodeFieldExperimenter* nfe, ChickenPool* birdMngr ) override ;
    void setStateSheltering( StateExperimenterSheltering* stateSheltering ) ;

private:
    
    StateExperimenterSheltering* stateSheltering ;
    
} ;


class StateExperimenterSheltering: public StateExperimenter {
public:
    
    void onGoToNextStageRequest( NodeFieldExperimenter* nfe, Clock& clock, Random& randGen, BirdTransferManager& transferMngr ) override ;
    void onResetRequest( NodeFieldExperimenter* nfe, ChickenPool* birdMngr ) override ;
    void setStateMarket( StateExperimenterMarketStage* stateMarket ) ;
    void setStateRecruiting( StateExperimenterRecruiting* stateRecruiting ) ;


    
private:
    
    StateExperimenterMarketStage* stateMarket ;
    StateExperimenterRecruiting* stateRecruiting ;
    
} ;

class StateExperimenterMarketStage: public StateExperimenter {
public:
    
    void onGoToNextStageRequest( NodeFieldExperimenter* nfe, Clock& clock, Random& randGen, BirdTransferManager& transferMngr ) override ;
    void onResetRequest( NodeFieldExperimenter* nfe, ChickenPool* birdMngr ) override ;
    void setStateRecruiting( StateExperimenterRecruiting* stateRecruiting );

private:
    
    StateExperimenterRecruiting* stateRecruiting ;

    
} ;


/*
//=== wholesaler states ===//

class StateWSAtHome;
class StateWSBuy;
class StateWSWholesaleSell;
class StateWSRetailSell;

class StateWSAtHome: public StateVendor {
public:
    
    void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) override;
    void onGoHomeRequest( NodeVendor* nv ) override;
    void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) override;
    uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) override;
    void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    
    bool isWorking() override                                   { return false; }
    
    void setStateBuy(StateWSBuy* stateBuy);
    void setStateSell(StateWSWholesaleSell* stateSell);
private:
    StateWSBuy* stateBuy;
    StateWSWholesaleSell* stateSell;
};

class StateWSBuy: public StateVendor {
public:
    void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) override;
    void onGoHomeRequest( NodeVendor* nv ) override;
    void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) override;
    uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) override;
    void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    bool isWorking() override                                   { return true; }
   
    void setStateHome(StateWSAtHome* stateHome);
    void setStateSell(StateWSWholesaleSell* stateSell);

private:
    StateWSAtHome* stateHome;
    StateWSWholesaleSell* stateWholesale;
};

class StateWSWholesaleSell: public StateVendor {
public:
    void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) override;
    void onGoHomeRequest( NodeVendor* nv ) override;
    void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) override;
    uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) override;
    void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    bool isWorking() override                                   { return true; }
    void setStateHome(StateWSAtHome* stateHome);
    void setStateRetail(StateWSRetailSell* stateRetail);
private:
    StateWSAtHome* stateHome;
    StateWSRetailSell* stateRetail;

};

class StateWSRetailSell: public StateVendor {
public:
    void onGoToMarketRequest( NodeVendor* nv, Random& randGen ) override;
    void onGoHomeRequest( NodeVendor* nv ) override;
    void onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    void onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) override;
    uint onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) override;
    uint onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) override;
    void onSellRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) override;
    bool isWorking() override                                   { return true; }
    void setStateHome(StateWSAtHome* stateHome);
private:
    StateWSAtHome* stateHome;
};
 
 */

//====== State Managers ======//

class FarmStateManager {
public:
    FarmStateManager();
    ~FarmStateManager();
    StateFarmEmpty* getStateEmpty();
    //StateFarmRaising* getStateRaising();
    //StateFarmTrading* getStateTrading();
private:
    StateFarmEmpty* stateEmpty;
    StateFarmRaising* stateRaising;
    StateFarmTrading* stateTrading;
};

class MiddlemanStateManager {
public:
    MiddlemanStateManager();
    ~MiddlemanStateManager();
    StateMmanAwaiting* getStateAwaiting();
private:
    StateMmanAwaiting* stateAwaiting;
    StateMmanCollecting* stateCollecting;
    StateMmanSelling* stateSelling;
};

class MarketStateManager {
public:
    MarketStateManager();
    ~MarketStateManager();
    StateMarketOpen* getStateOpen();
    StateMarketClose* getStateClose();
private:
    StateMarketOpen* stateOpen;
    StateMarketClose* stateClose;

};

class VendorStateManager {
public:
    VendorStateManager();
    ~VendorStateManager();
    
    StateVendorAtHome* getStateHome();
    StateVendorBuy* getStateBuy();
    StateVendorSell* getStateSell();
    
    //StateWSAtHome* getStateWSHome();
    //StateWSBuy* getStateWSBuy();
    //StateWSWholesaleSell* getStateWSWholesaleSell();
    //StateWSRetailSell* getStateWSRetailSell();
    
private:
    StateVendorAtHome* stateHome;
    StateVendorBuy* stateBuy;
    StateVendorSell* stateSell;
    
    //StateWSAtHome* stateWSHome;
    //StateWSBuy* stateWSBuy;
    //StateWSWholesaleSell* stateWSWholesaleSell;
    //StateWSRetailSell* stateWSRetailSell;

};

class FieldExperimenterStateManager {
public:
    FieldExperimenterStateManager() ;
    ~FieldExperimenterStateManager() ;
    StateExperimenterRecruiting*    getStateRecruiting() ;
    StateExperimenterSheltering*    getStateSheltering() ;
    StateExperimenterMarketStage*   getStateMarket() ;
private:
    StateExperimenterRecruiting*    stateRecruiting ;
    StateExperimenterSheltering*    stateSheltering ;
    StateExperimenterMarketStage*   stateMarket ;
} ;


#endif /* node_hpp */
