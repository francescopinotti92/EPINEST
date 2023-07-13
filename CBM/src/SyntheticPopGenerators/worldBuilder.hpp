//
//  worldBuilder.hpp
//  CBM
//
//

#ifndef worldBuilder_hpp
#define worldBuilder_hpp

//#include "geometry_utils.hpp"
#include "point_manager.hpp"
#include "point.hpp"
#include "node.hpp"
#include "fstream"
#include <nlohmann/json.hpp>


class WorldBuilder;

//====== Data handlers ======//

// pop patch handler

struct PopPatch {
public:
    PopPatch( uint id_, uint population_, Geom::Point pos_ ): id( id_ ), population( population_ ), pos( pos_ ) {};
    uint id;
    uint population;
    Geom::Point pos;
    
};

// this classes hold necessary infos to create any actor

struct FarmDataHandler {
public:
    FarmDataHandler( uint id, uint size, uint catchmentArea, Geom::Point pos );
    virtual ~FarmDataHandler() = default;
    virtual NodeFarm* createNodeFarm(WorldBuilder* wb) = 0;
    virtual double getDailyOutput(const BirdType& bt) = 0;
    virtual std::vector<BirdType> getTradedBirdTypes() = 0;
    
    uint size;
    uint id;
    uint catchmentArea;
    Geom::Point pos;
    
};

struct CommercialFarmDataHandler: public FarmDataHandler {
    CommercialFarmDataHandler(uint id, uint size, uint catchmentArea, Geom::Point pos, BirdType bt, double p_refill, uint ndays_rollout);
    NodeFarm* createNodeFarm( WorldBuilder* wb ) override;
    double getDailyOutput( const BirdType& bt ) override;
    std::vector<BirdType> getTradedBirdTypes() override;
    
    BirdType bt;
    double p_refill;        // daily batch refill probability
    uint ndays_rollout;     // (minimum) number of days to sell a batch
};

// uses a negative binomial distribution for time to next cycle start
struct CommercialFarmDataHandlerNegBin: public FarmDataHandler {
    CommercialFarmDataHandlerNegBin( uint id, uint size, uint catchmentArea, Geom::Point pos, BirdType bt, double pNB, double nNB, uint ndays_rollout );
    NodeFarm* createNodeFarm( WorldBuilder* wb ) override;
    double getDailyOutput( const BirdType& bt ) override;
    std::vector<BirdType> getTradedBirdTypes() override;
    BirdType bt;
    double p_negbin;        // negative binomial's p argument
    double n_negbin;        // negative binomial's n argument
    uint ndays_rollout;     // (minimum) number of days to sell a batch
};

struct MarketDataHandler;

struct VendorDataHandler {
    VendorDataHandler( uint id, BirdType bt, double target, uint layer );
    virtual ~VendorDataHandler() = default;
    virtual NodeVendor* createNodeVendor(WorldBuilder* wb) = 0;
    virtual void updateSellingMarket( MarketDataHandler* market_handle ) = 0;
    
    //virtual double getExpectedDailyBirdsSold() = 0;
    
    void setMarketBuy(NodeMarket* mB_);
    void setMarketSell(NodeMarket* mS_);

    uint id;
    BirdType bt;
    double target;
    double capacity;
    uint layer;
    NodeMarket* mB;
    NodeMarket* mS;
};

struct VendorRetailerDataHandler: public VendorDataHandler {
    VendorRetailerDataHandler(uint id, BirdType bt, double target, uint layer, double p_empty, double odUnsold, double propUnsold, bool prioritizesRepurposed);
    NodeVendor* createNodeVendor( WorldBuilder* wb ) override;
    void updateSellingMarket( MarketDataHandler* market_handle ) override;
    //double getExpectedDailyBirdsSold() override;
    double p_empty;
    double odUnsold;
    double propUnsold;
    bool priotitizesRepurposed;
};

struct VendorWholesalerDataHandler: public VendorDataHandler {
    VendorWholesalerDataHandler(uint id, BirdType bt, double target, uint layer, double p_empty, double odUnsold, double propUnsold, double p_2wholesalers, double p_2retailers, bool prioritizesRepurposed);
    NodeVendor* createNodeVendor( WorldBuilder* wb ) override;
    void updateSellingMarket( MarketDataHandler* market_handle ) override;
    double p_empty;
    double odUnsold;
    double propUnsold;
    double p_2wholesalers;
    double p_2retailers;
    bool priotitizesRepurposed;
};

struct MarketDataHandler {
    MarketDataHandler( uint id_, Geom::Point pos );
    virtual ~MarketDataHandler() = default;
    
    NodeMarket* createNodeMarket(WorldBuilder* wb);
    
    
    void addBirdType(const BirdType& bt);
    void addVendorLayer(const BirdType& bt, const double& prop2WS, const double& prop2R);
    
    void addBuyer(VendorDataHandler* vdh);
    void addSeller(VendorDataHandler* vdh);
    
    void onAddingWholesalerSeller( const BirdType& bt, const uint& layer, const double& nbirds );
    void onAddingRetailerSeller( const BirdType& bt );

    void increaseDailyInputMmen(const BirdType& bt, const double& amount);

    double getDailyInputLayer(const BirdType& bt, const uint& layer);
    
    std::vector<bool> getOpenDays();
    
    uint id;
    Geom::Point pos;
    
    std::map<BirdType, uint> maxLayers;
    std::map<BirdType, std::vector<double>> propSoldToWholesalerStack;
    std::map<BirdType, std::vector<double>> propSoldToRetailerStack;
    std::map<BirdType, std::vector<double>> expectedDailyInputStack;
    
    std::map<BirdType, std::vector<VendorDataHandler*> > Buyers;
    std::map<BirdType, std::vector<VendorDataHandler*> > Sellers;
    std::vector<BirdType> tradedBirds;
    std::vector<bool> openDays;

};

struct MiddleManDataHandler {
public:
    MiddleManDataHandler(uint id_, uint outDegMarket_);
    virtual ~MiddleManDataHandler() = default;
    virtual NodeMman* createNodeMman( WorldBuilder* wb ) = 0;
    virtual double getDailyTargetFromFarms(const BirdType& bt) = 0;
    virtual double getDailyOutputToMarkets(const BirdType& bt) = 0;
    uint id;
    uint outDegMarket;
};

struct SimpleMiddleManDataHandler: public MiddleManDataHandler {
    SimpleMiddleManDataHandler(uint id_, uint outDegMarket_, double target, BirdType bt_, uint nVisitedAreas_, double pMovement_);
    NodeMman* createNodeMman(WorldBuilder* wb) override;
    double getDailyTargetFromFarms( const BirdType& bt ) override;
    double getDailyOutputToMarkets( const BirdType& bt ) override;
    BirdType bt;
    double dailyTarget;
    uint nVisitedAreas;
    double pMovement;
};

template <typename T>
T return_constant( const T& val ) {
    return val ;
}

//====== WorldBuilder ======//

// this class contains functions to create different types of actors
//
class WorldBuilder {
public:
    
    WorldBuilder( std::string model_settings_path, Random& randGen );
        
    NodeFarm* createFarm(CommercialFarmDataHandler* farmHandle);
    NodeFarm* createFarm(CommercialFarmDataHandlerNegBin * farmHandle);

    NodeMman* createMman(SimpleMiddleManDataHandler* mmanHandle);
    NodeMarket* createMarket(MarketDataHandler* marketHandle);
    NodeVendor* createVendor(VendorRetailerDataHandler* vendorHandle);
    NodeVendor* createVendor(VendorWholesalerDataHandler* vendorHandle);
    
    // getters
    std::vector<BaseNode*>* getNodeVec()        { return allNodeVec; }
    std::vector<NodeFarm*>* getFarmVec()        { return farmVec;    }
    std::vector<NodeMman*>* getMmanVec()        { return mmanVec;    }
    std::vector<NodeMarket*>* getMarketVec()    { return marketVec;  }
    std::vector<NodeVendor*>* getVendorVec()    { return vendorVec;  }
    
    FarmStateManager* getFarmStates()          { return farmStates;   }
    MiddlemanStateManager* getMmanStates()     { return mmanStates;   }
    MarketStateManager* getMarketStates()      { return marketStates; }
    VendorStateManager* getVendorStates()      { return vendorStates; }
    
    AreaManager* getAreaManager()               { return areaMngr; }
    
    uint getNoVendorLayers()                    { return nVendorLayers; }
    
private:
    
    
    
    std::vector<BaseNode*>* allNodeVec;
    std::vector<NodeFarm*>* farmVec;
    std::vector<NodeMman*>* mmanVec;
    std::vector<NodeMarket*>* marketVec;
    std::vector<NodeVendor*>* vendorVec;
    
    AreaManager* areaMngr;
    
    FarmStateManager* farmStates;
    MiddlemanStateManager* mmanStates;
    MarketStateManager* marketStates;
    VendorStateManager* vendorStates;
    
    uint nFarms;
    uint nMmen;
    uint nMarkets;
    uint nVendors;
    
    uint nVendorLayers;
    
    bool reserveSpaceEnvironment;
};

#endif /* worldBuilder_hpp */
