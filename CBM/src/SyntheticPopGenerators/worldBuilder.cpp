//
//  worldBuilder.cpp
//  CBM
//
//

#include "worldBuilder.hpp"

//====== Data handlers ======//

//====== Farm data handlers ======//

FarmDataHandler::FarmDataHandler(uint id_, uint size_, uint catchmentArea_, Geom::Point pos_ ): id( id_ ), size( size_ ), catchmentArea( catchmentArea_ ), pos( pos_ ) {};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

CommercialFarmDataHandler::CommercialFarmDataHandler( uint id, uint size, uint catchmentArea, Geom::Point pos, BirdType bt_, double p_refill_, uint ndays_rollout_ ): FarmDataHandler( id, size, catchmentArea, pos ), bt( bt_ ), p_refill( p_refill_ ), ndays_rollout( ndays_rollout_ ) {
    
    assert( p_refill > 0. );
    assert( ndays_rollout > 0 );
    
};

NodeFarm* CommercialFarmDataHandler::createNodeFarm( WorldBuilder *wb ) {
    NodeFarm* nf = wb->createFarm( this );
    return nf;
}

double CommercialFarmDataHandler::getDailyOutput( const BirdType& bt ) {
    
    if (this->bt != bt)
        return 0.;
    
    if ( p_refill * ndays_rollout == 0. )
        return 0.;
    
    double mean_t_refill = 1. / p_refill;
    
    return size / ( mean_t_refill + constants::harvestTimes[bt] + ndays_rollout - 1 );
}

std::vector<BirdType> CommercialFarmDataHandler::getTradedBirdTypes() {
    return { bt };
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

CommercialFarmDataHandlerNegBin::CommercialFarmDataHandlerNegBin( uint id, uint size, uint catchmentArea, Geom::Point pos, BirdType bt_, double pNB, double nNB, uint ndays_rollout_ ): FarmDataHandler( id, size, catchmentArea, pos ), bt( bt_ ), p_negbin( pNB ), n_negbin( nNB ), ndays_rollout( ndays_rollout_ ) {
    
    assert( p_negbin > 0. );
    assert( n_negbin > 0. );
    assert( ndays_rollout > 0 );

};

NodeFarm* CommercialFarmDataHandlerNegBin::createNodeFarm( WorldBuilder *wb ) {
    NodeFarm* nf = wb->createFarm( this );
    return nf;
}

double CommercialFarmDataHandlerNegBin::getDailyOutput( const BirdType& bt ) {
    
    if ( this->bt != bt )
        return 0.;
    
    double mean_t_refill = 1 + n_negbin * ( 1 - p_negbin ) / p_negbin; 
    
    return size / ( mean_t_refill + constants::harvestTimes[bt] + ndays_rollout - 1. );


}

std::vector<BirdType> CommercialFarmDataHandlerNegBin::getTradedBirdTypes() {
    return { bt };
}

//====== Vendor data handlers ======//

VendorDataHandler::VendorDataHandler( uint id_, BirdType bt_, double target_, uint layer_ ): id( id_ ), bt( bt_ ), target( target_ ), layer( layer_ ) {
    mB = nullptr;
    mS = nullptr;
    capacity = 0;
}

void VendorDataHandler::setMarketBuy(NodeMarket* mB_) {
    mB = mB_;
}

void VendorDataHandler::setMarketSell(NodeMarket* mS_) {
    mS = mS_;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

VendorRetailerDataHandler::VendorRetailerDataHandler( uint id, BirdType bt, double target, uint layer, double p_empty_, double odUnsold_, double propUnsold_, bool prioritizesRepurposed_ ): VendorDataHandler( id, bt, target, layer ), p_empty( p_empty_ ), odUnsold( odUnsold_ ), propUnsold( propUnsold_ ), priotitizesRepurposed( prioritizesRepurposed_ ) {
    
    capacity = 1.5 * target / ( 1 - ( 1. - p_empty ) * propUnsold ); // set to 1.5 x (expected cargo)

    assert( capacity > 1. );
    
};

NodeVendor* VendorRetailerDataHandler::createNodeVendor( WorldBuilder *wb ) {
    NodeVendor* nv = wb->createVendor( this );
    return nv;
}

void VendorRetailerDataHandler::updateSellingMarket( MarketDataHandler* market_handle ) {
    market_handle->onAddingRetailerSeller( bt );
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//

VendorWholesalerDataHandler::VendorWholesalerDataHandler( uint id, BirdType bt, double target, uint layer, double p_empty_, double odUnsold_, double propUnsold_, double p_2wholesalers_, double p_2retailers_, bool prioritizesRepurposed_ ): VendorDataHandler( id, bt, target, layer ), p_empty( p_empty_ ), odUnsold( odUnsold_ ), propUnsold( propUnsold_ ), p_2wholesalers( p_2wholesalers_ ), p_2retailers( p_2retailers_ ), priotitizesRepurposed( prioritizesRepurposed_ ) {
        
    capacity = 1.5 * target / ( 1 - ( 1. - p_empty ) * propUnsold ); // set to 1.5 x (expected cargo)

    assert( capacity > 1. );
    
};

NodeVendor* VendorWholesalerDataHandler::createNodeVendor(WorldBuilder *wb) {
    NodeVendor* nv = wb->createVendor( this );
    return nv;
}

void VendorWholesalerDataHandler::updateSellingMarket( MarketDataHandler* market_handle ) {
    market_handle->onAddingWholesalerSeller( bt, layer, target );
}

//====== Market data handlers ======//

MarketDataHandler::MarketDataHandler( uint id_, Geom::Point pos_ ): id( id_ ), pos( pos_ ) {
    tradedBirds = {};
}

NodeMarket* MarketDataHandler::createNodeMarket( WorldBuilder* wb ) {
    NodeMarket* nm = wb->createMarket( this );
    return nm;
}


void MarketDataHandler::increaseDailyInputMmen( const BirdType& bt, const double& amount ) {
    // middlemen contribute to first layer (indexed by 0)
    expectedDailyInputStack[bt][0] += amount;
}

double MarketDataHandler::getDailyInputLayer(const BirdType &bt, const uint& layer) {
    return expectedDailyInputStack[bt][layer];
}

void MarketDataHandler::addBirdType(const BirdType &bt) {
    tradedBirds.push_back( bt );
    
    maxLayers.insert( { bt, 0 } );
    propSoldToWholesalerStack.insert( { bt, {} } );
    propSoldToRetailerStack.insert( { bt, {} } );
    expectedDailyInputStack.insert( { bt, {} } );
    
    Buyers.insert( std::make_pair(bt, std::vector<VendorDataHandler*>() ) );
    Sellers.insert( std::make_pair(bt, std::vector<VendorDataHandler*>() ) );
}

void MarketDataHandler::addVendorLayer(const BirdType &bt, const double &prop2WS, const double &prop2R) {
    maxLayers[bt]++;
    propSoldToWholesalerStack[bt].push_back( prop2WS );
    propSoldToRetailerStack[bt].push_back( prop2R );
    expectedDailyInputStack[bt].push_back( 0. );
        
}

void MarketDataHandler::addBuyer(VendorDataHandler* vdh) {
    Buyers[vdh->bt].push_back(vdh);
}

void MarketDataHandler::addSeller(VendorDataHandler* vdh) {
    Sellers[vdh->bt].push_back(vdh);
    vdh->updateSellingMarket( this );
}

void MarketDataHandler::onAddingRetailerSeller(const BirdType &bt) {};
void MarketDataHandler::onAddingWholesalerSeller(const BirdType &bt, const uint &layer, const double &nbirds) {
    // vendor in 'layer' contributes to ('layer' + 1)
    // if vendor expects 'target' birds daily, 'target' is also the expected output
    expectedDailyInputStack[ bt ][ layer + 1 ] += nbirds;

}

std::vector<bool> MarketDataHandler::getOpenDays() {
    return openDays;
}

//====== Middleman data handlers ======//

MiddleManDataHandler::MiddleManDataHandler(uint id_, uint outDegMarket_): id(id_), outDegMarket(outDegMarket_) {};

SimpleMiddleManDataHandler::SimpleMiddleManDataHandler( uint id_, uint outDegMarket_, double target, BirdType bt_, uint nVisitedAreas_, double pMovement_ ): MiddleManDataHandler(id_, outDegMarket_), dailyTarget(target), bt(bt_), nVisitedAreas( nVisitedAreas_ ), pMovement( pMovement_ ) {};

NodeMman* SimpleMiddleManDataHandler::createNodeMman( WorldBuilder* wb ) {
    NodeMman* nmm = wb->createMman( this );
    return nmm;
}

double SimpleMiddleManDataHandler::getDailyTargetFromFarms( const BirdType &bt ) {
    
    if ( bt == this->bt )
        return dailyTarget;
    else
        return 0.;
    
}

double SimpleMiddleManDataHandler::getDailyOutputToMarkets( const BirdType &bt ) {
    
    if (bt == this->bt)
        return dailyTarget;
    else
        return 0.;
    
}

//====== WorldBuilder ======//

//=== constructors ===//

WorldBuilder::WorldBuilder( std::string model_settings_path, Random& randGen ) {
    
    // set relevant values to false
    
    allNodeVec   = nullptr;
    farmVec      = nullptr;
    mmanVec      = nullptr;
    marketVec    = nullptr;
    vendorVec    = nullptr;
    areaMngr     = nullptr;
    farmStates   = nullptr;
    mmanStates   = nullptr;
    marketStates = nullptr;
    vendorStates = nullptr;
    reserveSpaceEnvironment = false;
    nFarms        = 0;
    nMmen         = 0;
    nMarkets      = 0;
    nVendors      = 0;
    nVendorLayers = 0;

    //====== Read model parameters ======//
    
    std::ifstream readFileModelParams( model_settings_path ); // contains all model parameters
    nlohmann::json paramsModel = nlohmann::json::parse( readFileModelParams );
    
    requireJsonKey( paramsModel, "bts" );
    requireJsonKey( paramsModel, "area_list_path" );
    requireJsonKey( paramsModel, "farm_list_path" );
    requireJsonKey( paramsModel, "market_list_path" );
    requireJsonKey( paramsModel, "area_market_fluxes_path" );
    requireJsonKey( paramsModel, "market_network_path" );
    requireJsonKey( paramsModel, "mmen_params_path" );
    requireJsonKey( paramsModel, "vendor_params_path" );
    requireJsonKey( paramsModel, "epi_model_path" );
    requireJsonKey( paramsModel, "vendorTotalSizeScaling" );
    requireJsonKey( paramsModel, "mmanTotalSizeScaling" );
    requireJsonKey( paramsModel, "mmanBatchSizeScaling" );
    requireJsonKey( paramsModel, "vendorMinSize" );
    requireJsonKey( paramsModel, "mmanMinSize" );

    //=== get some general parameters
    
    // read bird species properties
    std::vector<BirdType> bts = {};
    for ( std::string bt_str: paramsModel["bts"].get<std::vector<std::string>>() ) {
        BirdType bt = constants::getBirdTypeFromString( bt_str );
        bts.push_back( bt );
    }
    
    //=== get paths to more specific parameters
    std::string catchment_areas_list_path = paramsModel["area_list_path"].get<std::string>();
    std::string farm_list_path = paramsModel["farm_list_path"].get<std::string>();
    std::string market_list_path = paramsModel["market_list_path"].get<std::string>();
    std::string catchment_areas_network_path = paramsModel["area_market_fluxes_path"].get<std::string>();
    std::string market_network_path = paramsModel["market_network_path"].get<std::string>();
    std::string mmen_params_path = paramsModel["mmen_params_path"].get<std::string>();
    std::string vendor_params_path = paramsModel["vendor_params_path"].get<std::string>();
    std::string epi_model_path = paramsModel["epi_model_path"].get<std::string>();

    //=== additional parameters
    double increasedCapacityMarket = paramsModel["vendorTotalSizeScaling"].get<double>();
    double increasedCapacityMman = paramsModel["mmanTotalSizeScaling"].get<double>();
    double mmanSizeScalingFactor = paramsModel["mmanBatchSizeScaling"].get<double>();
    double minSizeVendor = paramsModel["vendorMinSize"].get<double>();    // minimum sizes vendors
    double minSizeMman = paramsModel["mmanMinSize"].get<double>();     // minimum sizes middlemen
        
    // vendor parameters/settings
    std::map<BirdType, uint> nMaxVendorLayersByBt;
    std::map<BirdType, std::function<uint ()>> sampleRetailerDailyTargetMmen; // retailer target distr
    std::map<BirdType, std::function<uint ()>> sampleWholesalerDailyTargetMmen; // wholesaler target distr
    std::map<BirdType, double> probEmptyBatchRetailByBt;
    std::map<BirdType, double> probEmptyBatchWholesaleByBt;
    std::map<BirdType, double> propSurplusRetailByBt;
    std::map<BirdType, double> propSurplusWholesaleByBt;
    std::map<BirdType, double> overDSurplusRetailByBt;
    std::map<BirdType, double> overDSurplusWholesaleByBt;
    std::map<BirdType, double> propPrioritizersRetailByBt;
    std::map<BirdType, double> propPrioritizersWholesaleByBt;

    std::ifstream readFileVendorParams( vendor_params_path ); // contains all model parameters
    nlohmann::json vendorParams = nlohmann::json::parse( readFileVendorParams );
    
    for (BirdType bt: bts) {
        
        std::string btStr = constants::getStringFromBirdType( bt );
        
        // for retailers
        {
            requireJsonKey( vendorParams[btStr], "dailyTargetMmenRetailers" );

            std::string distrType = vendorParams[btStr]["dailyTargetMmenRetailers"].get<std::string>();
            if ( distrType == "CustomDiscrete" ) {
                
                requireJsonKey( vendorParams[btStr], "dailyTargetMmenRetailersPMF" );

                DiscreteDistribution pmf(vendorParams[btStr]["dailyTargetMmenRetailersPMF"].get<std::vector<double>>());
              
                auto sampler = [&,pmf]() { return randGen.getCustomDiscrete(pmf); };
                sampleRetailerDailyTargetMmen.insert( { bt, sampler } );
                
            }
        }
        
        // for wholesalers
        {
            requireJsonKey( vendorParams[btStr], "dailyTargetMmenWholesalers" );

            std::string distrType = vendorParams[btStr]["dailyTargetMmenWholesalers"].get<std::string>();
            if ( distrType == "CustomDiscrete" ) {
                
                requireJsonKey( vendorParams[btStr], "dailyTargetMmenWholesalersPMF" );

                DiscreteDistribution pmf(vendorParams[btStr]["dailyTargetMmenWholesalersPMF"].get<std::vector<double>>());
              
                auto sampler = [&,pmf]() { return randGen.getCustomDiscrete(pmf); };
                sampleWholesalerDailyTargetMmen.insert( { bt, sampler } );
                
            }
            
        }
        
        // probability of emptying daily batch (Retailers) per bird type
        {
            requireJsonKey( vendorParams[btStr], "probEmptyBatchRetail" );
            for ( auto& elem: vendorParams[btStr]["probEmptyBatchRetail"].items() ) {
                double p = elem.value();
                probEmptyBatchRetailByBt.insert( { bt, p } );
            }
        }
        
        // probability of emptying daily batch (Wholesalers) per bird type
        {
            requireJsonKey( vendorParams[btStr], "probEmptyBatchWholesale" );
            for ( auto& elem: vendorParams[btStr]["probEmptyBatchWholesale"].items() ) {
                double p = elem.value();
                probEmptyBatchWholesaleByBt.insert( { bt, p } );
            }
        }
        
        // proportion of daily batch to surplus (Retailers) per bird type
        {
            requireJsonKey( vendorParams[btStr], "propSurplusRetail" );
            for ( auto& elem: vendorParams[btStr]["propSurplusRetail"].items() ) {
                double p = elem.value();
                propSurplusRetailByBt.insert( { bt, p } );
            }
        }
        
        // proportion of daily batch to surplus (Wholesalers) per bird type
        {
            requireJsonKey( vendorParams[btStr], "propSurplusWholesale" );
            for ( auto& elem: vendorParams[btStr]["propSurplusWholesale"].items() ) {
                double p = elem.value();
                propSurplusWholesaleByBt.insert( { bt, p } );
            }
        }

        // overdispersion of daily batch to surplus (Retailers) per bird type
        {
            requireJsonKey( vendorParams[btStr], "overDSurplusRetail" );
            for ( auto& elem: vendorParams[btStr]["overDSurplusRetail"].items() ) {
                double p = elem.value();
                overDSurplusRetailByBt.insert( { bt, p } );

            }
        }
        
        // overdispersion of daily batch to surplus (Wholesalers) per bird type
        {
            requireJsonKey( vendorParams[btStr], "overDSurplusWholesale" );
            for ( auto& elem: vendorParams[btStr]["overDSurplusWholesale"].items() ) {
                double p = elem.value();
                overDSurplusWholesaleByBt.insert( { bt, p } );
            }
        }
        
        // proportion of retailers prioritizing sale of repurposed birds
        {
            requireJsonKey( vendorParams[btStr], "propPriorityRetail" );
            for ( auto& elem: vendorParams[btStr]["propPriorityRetail"].items() ) {
                double p = elem.value();
                propPrioritizersRetailByBt.insert( { bt, p } );
            }
        }
        
        // proportion of wholesalers prioritizing sale of repurposed birds
        {
            requireJsonKey( vendorParams[btStr], "propPriorityWholesale" );
            for ( auto& elem: vendorParams[btStr]["propPriorityWholesale"].items() ) {
                double p = elem.value();
                propPrioritizersWholesaleByBt.insert( { bt, p } );
            }
        }
        
    }
    
    // middlemen parameters
    std::map<BirdType, double> maxRangeMmenKmByBt;
    std::map<BirdType, std::function<uint ()>> sampleMmenDailyTargetFarms;
    std::map<BirdType, std::function<uint ()>> sampleMmenNumberVisitedAreas;
    std::map<BirdType, std::function<double ()>> sampleMmenMovementProbability;
    std::function<uint ()> sampleMmenOutDeg;
    
    std::ifstream readFileMmenParams( mmen_params_path ); // contains all model parameters
    nlohmann::json middlemenParams = nlohmann::json::parse( readFileMmenParams );

    for (BirdType bt: bts) {

        std::string btStr = constants::getStringFromBirdType( bt );

        // max distance explored middlemen
        
        /*
        {
            double maxRangeMmenKm = paramsModel["middlemenParams"][btStr]["maxRangeMmenKm"].get<double>();
            maxRangeMmenKmByBt.insert( { bt, maxRangeMmenKm } );
        }
        */
        
        // instantiate functions to compute middlemen daily target
        {
            requireJsonKey( middlemenParams[btStr], "dailyTargetFarms" );

            std::string distrType = middlemenParams[btStr]["dailyTargetFarms"].get<std::string>();
            if ( distrType == "CustomDiscrete" ) {
                requireJsonKey( middlemenParams[btStr], "dailyTargetFarmsPMF" );
                DiscreteDistribution pmf(middlemenParams[btStr]["dailyTargetFarmsPMF"].get<std::vector<double>>());
                auto sampler = [&,pmf]() { return randGen.getCustomDiscrete(pmf); };
                sampleMmenDailyTargetFarms.insert( { bt, sampler } );
            }
            else {
                throw std::invalid_argument("distrType is invalid");
            }
        }
        
        // instantiate functions to compute aggregated middlemen out(market)-degree
        /*
        {
            std::string distrType = paramsModel["middlemenParams"][btStr]["aggOutDegreeDistr"].get<std::string>();
            if ( distrType == "Constant" ) {
                uint aggOutDeg = paramsModel["middlemenParams"][btStr]["aggOutDegree"].get<uint>();
                auto returnConstant = std::bind( []( const uint& val ) { return val; }, aggOutDeg );
                sampleMmenAggOutDeg.insert( { bt, returnConstant } );
            }
        }
         */
        
        // instantiate function to sample instantaneuous middlemen out(market)-degree
        {
            requireJsonKey( middlemenParams[btStr], "outDegreeDistr" ) ;
            std::string distrType = middlemenParams[btStr]["outDegreeDistr"].get<std::string>();
            if ( distrType == "Geometric" ) {
                requireJsonKey( middlemenParams[btStr], "p" ) ;
                double p = middlemenParams[btStr]["p"].get<double>();
                sampleMmenOutDeg = std::bind( &Random::getGeom1, &randGen, p );
            }
            else if ( distrType == "Poisson" ) {
                requireJsonKey( middlemenParams[btStr], "mu" );
                double mu = middlemenParams[btStr]["mu"].get<double>() ;
                sampleMmenOutDeg = std::bind( &Random::getZeroTruncPoisson, &randGen, mu );
            }
            else if ( distrType == "Constant") {
                requireJsonKey( middlemenParams[btStr], "outDegreeConst" ) ;
                int mu = middlemenParams[btStr]["outDegreeConst"].get<int>() ;
                sampleMmenOutDeg = std::bind( return_constant<int>, mu ) ;
            }
            else {
                throw std::invalid_argument("distrType is invalid");
            }
        }
        
        // instantiate function to sample number of areas visited daily
        {
            requireJsonKey( middlemenParams[btStr], "nVisitedAreasDistr" );
            std::string distrType = middlemenParams[btStr]["nVisitedAreasDistr"].get<std::string>();
            if ( distrType == "Constant" ) {
                requireJsonKey( middlemenParams[btStr], "nVisitedAreasConst" );
                uint aggOutDeg = middlemenParams[btStr]["nVisitedAreasConst"].get<uint>();
                auto returnConstant = std::bind( []( const uint& val ) { return val; }, aggOutDeg );
                sampleMmenNumberVisitedAreas.insert( { bt, returnConstant } );
            }
            else {
                throw std::invalid_argument("distrType is invalid");
            }
        }
        
        // instantiate function to sample daily movement probability
        {
            requireJsonKey( middlemenParams[btStr], "movementProbabilityDistr" );
            std::string distrType = middlemenParams[btStr]["movementProbabilityDistr"].get<std::string>();
            if ( distrType == "Constant" ) {
                requireJsonKey( middlemenParams[btStr], "movementProbabilityConst" );
                double p_move = middlemenParams[btStr]["movementProbabilityConst"].get<double>();
                auto returnConstant = std::bind( []( const double& val ) { return val; }, p_move );
                sampleMmenMovementProbability.insert( { bt, returnConstant } );
            }
            else {
                throw std::invalid_argument("distrType is invalid");
            }
        }
        
    }
    
    //====== Allocate quantities necessary to set up a synthetic population ======//
    
    nFarms      = 0;
    nMmen       = 0;
    nMarkets    = 0;
    nVendors    = 0;
    
    uint nCatchmentAreas = 0;
    
    std::vector<FarmDataHandler*> farm_handles;
    std::map<BirdType, double> totalDailyOutputFarmsByType;
    std::map<BirdType, std::vector<double> > totalDailyOutputFarmsByArea;
    std::map<BirdType, std::vector<std::vector<double> > > dailyOutputFarmsByArea;
    
    std::vector<MarketDataHandler*> market_handles = {};
    std::map<BirdType, net_utils::w_adjacency_list> adjListMarketNetwork;
    //std::unordered_map<BirdType, std::vector<std::pair<uint, uint>>> marketEdges;
    //std::unordered_map<BirdType, std::vector<double>> marketWeightVec;
    
    std::vector<VendorDataHandler*> vendor_handles;
    
    std::vector<MiddleManDataHandler*> mmen_handles;
    std::map<BirdType, std::vector<MiddleManDataHandler*> > mmen_handles_bybt;
    std::map<BirdType, std::vector<double> > dailyOutputMmen;
    
    std::map<BirdType, std::vector<double> > dailyOutputCatchAreaByBt;
    std::map<BirdType, net_utils::w_edge_list> relFlowsCatchAreaToMarketsByBt;
    std::map<BirdType, net_utils::w_edge_list> absFlowsCatchAreaToMarketsByBt;

    std::map<BirdType, net_utils::w_adjacency_list> adjListMmenMarketByBt;
    std::map<BirdType, net_utils::w_adjacency_list> adjListFarmMmenByBt;
    
    Geom::PointManager<uint> areasSPM;
    //SpatialPointManager farmSPM;

    
    //====== Read catchment areas details ======//
    
    std::ifstream readFile_catchment_areas( catchment_areas_list_path );
    
    if ( readFile_catchment_areas.good() ) {
        uint id;
        double xpos, ypos;
        
        while ( readFile_catchment_areas >> id >> xpos >> ypos ) {
            
            nCatchmentAreas++;
            areasSPM.addPoint( Geom::Point( xpos, ypos ), id );

        }
        readFile_catchment_areas.close();
    }
    
    //====== Read farm details ======//
    
    farm_handles.reserve(1000);

    std::ifstream readFile(farm_list_path);
    nlohmann::json file_data = nlohmann::json::parse(readFile);
    for ( auto& elem: file_data.items() ) {
        //std::cout << elem.key() << "\n";
        auto& data = elem.value();

        Geom::Point pos = Geom::Point( data["x"], data["y"] );
        ActorType farm_type = constants::getActorTypeFromString( data["Type"].get<std::string>() );

        switch ( farm_type ) {
            case ActorType::commercial_farm:
            {
                requireJsonKey( data, "id" );
                requireJsonKey( data, "Size" );
                requireJsonKey( data, "mode_refill" );
                requireJsonKey( data, "rollout_time" );
                requireJsonKey( data, "catchment_area" );
                requireJsonKey( data, "BirdType" );

                uint id = data["id"].get<uint>();
                uint size = data["Size"].get<uint>();
                std::string mode_refill = data["mode_refill"].get<std::string>();
                uint ndays_rollout = data["rollout_time"].get<uint>();
                uint catchment_area = data["catchment_area"].get<uint>();
                std::vector<std::string> bt_names = data["BirdType"].get<std::vector<std::string>>();
                
                assert( bt_names.size() == 1 ); // commercial farms deal with a single species
                
                // check different farm modes
                if ( mode_refill == "Geometric" ) {
                    
                    requireJsonKey( data, "p_refill" );
                    double p_refill = data["p_refill"].get<double>();

                    CommercialFarmDataHandler* farm_handle = new CommercialFarmDataHandler(id,
                                              size,
                                              catchment_area,
                                              pos,
                                              constants::getBirdTypeFromString( bt_names[0] ),
                                              p_refill,
                                              ndays_rollout);
                
                    farm_handles.push_back( farm_handle );
                }
                else if ( mode_refill == "NegativeBinomial" ) {
                    
                    requireJsonKey( data, "p_refill" );
                    requireJsonKey( data, "n_refill" );

                    double p_refill = data["p_refill"].get<double>();
                    double n_refill = data["n_refill"].get<double>();
                    
                    CommercialFarmDataHandlerNegBin* farm_handle = new CommercialFarmDataHandlerNegBin(id,
                                              size,
                                              catchment_area,
                                              pos,
                                              constants::getBirdTypeFromString( bt_names[0] ),
                                              p_refill,
                                              n_refill,
                                              ndays_rollout);
                
                    farm_handles.push_back( farm_handle );

                }
                else {
                    throw std::invalid_argument("mode_refill is invalid");
                }
                break;
            }
            default:
                break;
        }

        nFarms++;
    }
    
    // sort farms according to id
    // n.b. json reading might not yield farms in desired order
    std::sort( farm_handles.begin(), farm_handles.end(),
              []( auto& left, auto& right ) -> bool { return left->id < right->id; } );
    
    
    // compute bird daily output per farm per bird type (A_i)
   
    for ( BirdType& bt: bts ) {
        
        totalDailyOutputFarmsByType.emplace(bt, 0.);
        totalDailyOutputFarmsByArea.emplace( bt, std::vector<double>(nCatchmentAreas, 0.) );
        dailyOutputFarmsByArea.emplace( bt, std::vector<std::vector<double>>( nCatchmentAreas, std::vector<double>( nFarms, 0. ) ) );
        
        uint ihandle = 0;
        
        for ( auto& handle: farm_handles ) {
            double dailyOutput = handle->getDailyOutput(bt);
            totalDailyOutputFarmsByType[bt] += dailyOutput;
            totalDailyOutputFarmsByArea[bt][handle->catchmentArea] += dailyOutput;
            dailyOutputFarmsByArea[bt][handle->catchmentArea][ihandle] = dailyOutput;
            ihandle++;
        }
    }
                
    //====== Read market details ======//
    
    market_handles.reserve( 1000 );
    
    std::ifstream readFile_market( market_list_path );
    nlohmann::json file_data_market = nlohmann::json::parse( readFile_market );
        
    for ( auto& elem: file_data_market.items() ) {

        auto& data = elem.value();
        
        requireJsonKey( data, "x" );
        requireJsonKey( data, "y" );
        requireJsonKey( data, "Type" );
        requireJsonKey( data, "id" );
        requireJsonKey( data, "TradedBirds" );

        Geom::Point pos = Geom::Point( data["x"], data["y"] );
        ActorType market_type = constants::getActorTypeFromString( data["Type"].get<std::string>() );
        
        switch (market_type) {
            case ActorType::market:
            {
                uint id = data["id"].get<uint>();
                MarketDataHandler* market_handle = new MarketDataHandler( id, pos );
                
                // read bird species that are traded in market
                // including fraction of daily birds actually bought from middlemen.
                for ( auto& tradedBird: data["TradedBirds"].items() ) {
                    BirdType bt = constants::getBirdTypeFromString( std::string( tradedBird.key() ) );
                    market_handle->addBirdType(bt);
                }
                
                // read details about vendor layers
                for ( auto& tradedBird: data["TradedBirds"].items() ) {
                    
                    std::string btStr = std::string( tradedBird.key() );
                    BirdType bt = constants::getBirdTypeFromString( btStr );
                    
                    requireJsonKey( tradedBird.value(), "propBirdsSold2WS" );
                    requireJsonKey( tradedBird.value(), "propBirdsSold2R" );

                    std::vector<double> propBirds2WS = tradedBird.value()["propBirdsSold2WS"].get<std::vector<double>>();
                    std::vector<double> propBirds2R = tradedBird.value()["propBirdsSold2R"].get<std::vector<double>>();
                    
                    // some checks
                    assert( propBirds2WS.size() > 0 );
                    assert( propBirds2WS.back() == 0. );
                    assert( propBirds2R.size() > 0 );
                    assert( propBirds2R.back() == 0. );
                    assert( propBirds2R.size() == propBirds2WS.size() );
                    
                    uint nLayers = static_cast<uint>( propBirds2WS.size() );
                                        
                    for ( uint layer = 0; layer < nLayers; ++layer )
                        market_handle->addVendorLayer( bt, propBirds2WS[layer], propBirds2R[layer] );
                    
                }
                           
                market_handles.push_back( market_handle );
                break;
            }
            default:
                break;
        }
        nMarkets++;
    }
    
    // sort markets according to id
    // n.b. json reading might not yield farms in desired order
    std::sort( market_handles.begin(), market_handles.end(),
              []( auto& left, auto& right ) -> bool { return left->id < right->id; } );

    
    //====== read market network ======//
    
    std::ifstream readFile_marketNet( market_network_path );
    
    for (BirdType bt: bts) {
        adjListMarketNetwork.insert( { bt, net_utils::w_adjacency_list( nMarkets, nMarkets ) } );
    }
    
    if ( readFile_marketNet.good() ) {
        uint id1, id2;
        double weightM;
        std::string btS;
        
        while ( readFile_marketNet >> id1 >> id2 >> weightM >> btS) {
            //std::cout << id1 << " " << id2 << " " << btS << weightM << std::endl;
            BirdType bt = constants::getBirdTypeFromString(btS);
            adjListMarketNetwork[bt].addNbr( id1, id2, weightM );
        }
        readFile_marketNet.close();
    }
            
    //====== read catchment area network ======//
    // i.e. read chicken flows from each sub-area to each individual market

    for ( BirdType bt: bts ) {
        relFlowsCatchAreaToMarketsByBt.insert( { bt, net_utils::w_edge_list( nCatchmentAreas * nMarkets ) } );
        absFlowsCatchAreaToMarketsByBt.insert( { bt, net_utils::w_edge_list( nCatchmentAreas * nMarkets ) } );
    }
    
    std::ifstream readFile_catchment_areas_net(catchment_areas_network_path);
    if ( readFile_catchment_areas_net.good() ) {
        uint idArea, idMarket;
        double weight;
        std::string btStr;
        
        while ( readFile_catchment_areas_net >> idArea >> idMarket >> weight >> btStr ) {
            BirdType bt = constants::getBirdTypeFromString( btStr );
            relFlowsCatchAreaToMarketsByBt[bt].addEdge( idArea, idMarket, weight ); // relative flow (K_al / K_a)
            
            
            absFlowsCatchAreaToMarketsByBt[bt].addEdge( idArea, idMarket, weight * totalDailyOutputFarmsByArea[bt][idArea] ); // compute absolute flow (K_al)
        }
        readFile_catchment_areas_net.close();
    }
    
    //====== Compute expected daily output from each area ======//
    
    for ( BirdType bt: bts ) {
        
        auto& farmOutputByArea = dailyOutputFarmsByArea[bt];
        std::vector<double> totalOutputTmp( nCatchmentAreas, 0. );
        
        for ( int a = 0; a < nCatchmentAreas; ++a ) {
            
            totalOutputTmp[a] = std::accumulate( farmOutputByArea[a].cbegin(), farmOutputByArea[a].cend(), 0. );
            
        }
        
        dailyOutputCatchAreaByBt.insert( { bt, totalOutputTmp } );
        
    }
    
    
    //====== Compute expected daily input in markets given area outputs ======//
    
    // this requires that daily input matches daily farm (area) output
    std::map<BirdType, std::vector<double> > expectedDailyInputMarkets;  // C_l
    
    for ( BirdType bt: bts ) {
        
        expectedDailyInputMarkets.insert( { bt, std::vector<double>( nMarkets, 0. ) } );
        
        auto& w_edges = absFlowsCatchAreaToMarketsByBt[bt];
        
        for ( auto& edge: w_edges ) {
            
            expectedDailyInputMarkets[bt][edge.id2] += edge.weight;
            market_handles[edge.id2]->increaseDailyInputMmen( bt, edge.weight );
            
        }
        
    }
    
    
    //====== create AreaManager ======//
    
    areaMngr = new AreaManager( bts, nCatchmentAreas, nMarkets );
    
    //=== set area and area-market bird output/fluxes
    for ( BirdType bt: bts ) {
        // set probability to select a region proportionally to its bird output
        
        areaMngr->set_P_a( bt, dailyOutputCatchAreaByBt[bt] );
        
        // set relative flux matrix ( proportion of birds sold by areas to markets )
        std::vector<std::vector<double>> fluxTmp( nCatchmentAreas, std::vector<double>( nMarkets, 0. ) );
        auto& fluxEdgeListTmp = relFlowsCatchAreaToMarketsByBt[bt];
        
        for ( auto edge = fluxEdgeListTmp.begin(); edge != fluxEdgeListTmp.end(); ++edge )
            fluxTmp[ edge->id1 ][ edge->id2 ] = edge->weight;
        
        areaMngr->set_P_al( bt, fluxTmp );
    }
    
    //=== find closest regions (within a distance)
    
    {
        double maxDistanceNbrsKm = 80; // maximum distance between neighboring regions // ?? change later
        for ( auto const& pt: areasSPM.getPoints() ) {
            auto nearestAreas = areasSPM.getNodesWithinRadius( pt.first, maxDistanceNbrsKm );
            for ( auto& ptAreaNbr: nearestAreas ) {
                areaMngr->addAreaNbr( pt.second, ptAreaNbr.second );
            }
        }
    }
    
    //=== compute number of farms per area
    
    {
        
        for ( FarmDataHandler* handle : farm_handles ) {
            
            uint area = handle->catchmentArea ;
            areaMngr->increaseTotalFarmCount( area ) ;
            
        }
        
    }
    

    //====== Create vendor handles ======//
    
    vendor_handles.reserve( 10000 );
    
    // check that all markets have the same number of layers
    nVendorLayers = market_handles[0]->maxLayers[bts[0]];
    for ( BirdType bt: bts ) {
        for ( auto& market_handle: market_handles ) {
            uint nLayersTmp = market_handle->maxLayers[bt];
            if ( nLayersTmp != nVendorLayers ) {
                throw Warning("markets must have the same number of vendor layers");
            }
        }
    }
    
    for ( BirdType bt: bts ) {
        
        uint nMaxLayers = market_handles[0]->maxLayers[bt];
        
        for ( uint layer = 0; layer < nMaxLayers; layer++ ) {
        
            for ( auto& market_handle: market_handles ) {
                
                // get weights to allocate vendors to selling destinations
                std::vector<double> weightsTmp( nMarkets, 0. );
                for ( auto& nbr: adjListMarketNetwork[bt][market_handle->id] ) {
                    weightsTmp[nbr.id] = nbr.weight;
                }
                
                // check weights
                {
                    double weightSum = std::accumulate( std::begin( weightsTmp ), std::end( weightsTmp ), 0. );
                    assert( weightSum > 0. );
                    if ( weightSum != 1. ) {
                        for ( double& w: weightsTmp )
                            w /= weightSum; // normalize weights
                    }
                }
                AliasTable sampleSellerMarket( weightsTmp );
                
                // get expected bird flux impinging on this layer
                
                double propSoldToW = market_handle->propSoldToWholesalerStack[bt][layer];
                double propSoldToR = market_handle->propSoldToRetailerStack[bt][layer];
                double expectedInputThisLayer = market_handle->getDailyInputLayer( bt, layer );
                
                if ( layer == 0 ) // modify expected input to allocate even more vendors
                    expectedInputThisLayer *= increasedCapacityMarket * increasedCapacityMman;
                
                // these are specific to wholesalers in layer l
                double propSoldToWplus1 = 0.;
                double propSoldToRplus1 = 0.;
                if ( layer < nMaxLayers - 1 ) {
                    propSoldToWplus1 = market_handle->propSoldToWholesalerStack[bt][layer+1];
                    propSoldToRplus1 = market_handle->propSoldToRetailerStack[bt][layer+1];
                }
                
                // create wholesalers
                
                double expectedInputToWS = expectedInputThisLayer * propSoldToW;    // these birds go to Wholesalers
                double runningQuotaWS = 0.;
                
                while ( runningQuotaWS < expectedInputToWS ) {
                    
                    // draw daily amount of birds as input
                    double dailyTarget = (double)sampleWholesalerDailyTargetMmen[bt]();
                    dailyTarget = std::max( dailyTarget, minSizeVendor );

                    if ( runningQuotaWS + dailyTarget + minSizeVendor >= expectedInputToWS ) {
                        dailyTarget = std::max( expectedInputToWS - runningQuotaWS, minSizeVendor );
                    }

                    // check if vendor will prioritize repurposed birds
                    bool doesPriority = randGen.getBool( propPrioritizersWholesaleByBt[bt] );
                    
                    VendorWholesalerDataHandler* vendor_handle = new VendorWholesalerDataHandler( nVendors, bt, dailyTarget, layer, probEmptyBatchWholesaleByBt[bt], overDSurplusWholesaleByBt[bt], propSurplusWholesaleByBt[bt], propSoldToWplus1, propSoldToRplus1, doesPriority );
                    
                    // sample selling market
                    uint idxSellerMarket = randGen.getCustomDiscrete( sampleSellerMarket );

                    market_handle->addBuyer( vendor_handle );                       // register vendor to buying location
                    market_handles[ idxSellerMarket ]->addSeller(vendor_handle);    // register vendor to selling location
                    
                    vendor_handles.push_back( vendor_handle ); // this is necessary to clean up pointers later

                    runningQuotaWS += dailyTarget;

                    nVendors++;
                }
                
                double expectedInputToR = expectedInputThisLayer * propSoldToR;     // these go to Retailers
                double runningQuotaR = 0.;
                
                while ( runningQuotaR < expectedInputToR ) {
                    double dailyTarget = (double)sampleRetailerDailyTargetMmen[bt]();
                    dailyTarget = std::max( dailyTarget, minSizeVendor );
                    
                    if ( runningQuotaR + dailyTarget + minSizeVendor > expectedInputToR ) {
                        dailyTarget = std::max( expectedInputToR - runningQuotaR, minSizeVendor ) ; // adjust according to current quota
    
                    }
                    
                    bool doesPriority = randGen.getBool( propPrioritizersRetailByBt[bt] );
                    
                    VendorRetailerDataHandler* vendor_handle = new VendorRetailerDataHandler( nVendors, bt, dailyTarget, layer, probEmptyBatchRetailByBt[bt], overDSurplusRetailByBt[bt], propSurplusRetailByBt[bt], doesPriority );
                    
                    uint idxSellerMarket = randGen.getCustomDiscrete( sampleSellerMarket );

                    market_handle->addBuyer( vendor_handle );
                    market_handles[ idxSellerMarket ]->addSeller( vendor_handle );
                    
                    vendor_handles.push_back( vendor_handle ); // this is necessary to clean up pointers
                    
                    runningQuotaR += dailyTarget;
                    nVendors++;
                }
            }
        }
                
    }
    
    //====== Create middleman handles ======//
    
    mmen_handles.reserve( 10000 );
        
    for ( BirdType bt: bts ) {
        
        mmen_handles_bybt.insert( { bt, std::vector<MiddleManDataHandler*>() } );
        mmen_handles_bybt[bt].reserve( 10000 );

        double expectedTotalInputMarkets = std::accumulate( expectedDailyInputMarkets[bt].begin(), expectedDailyInputMarkets[bt].end(), 0. );  // This should coincide with output from middlemen
        double runningTotal = 0.;
        
        expectedTotalInputMarkets *= increasedCapacityMman; // increase total capacity (to create more middlemen)
        
        while ( runningTotal < expectedTotalInputMarkets ) {
            // create simple middlemen, each trading a single bird species
        
            double dailyTarget = (double)sampleMmenDailyTargetFarms[bt]() / mmanSizeScalingFactor; // expected daily target from farms
            dailyTarget = std::max( dailyTarget, minSizeMman );
            
            uint outDegMarket = sampleMmenOutDeg();  // instantaneous degree
            
            while ( outDegMarket > nMarkets ) // degree has to be less than number of markets
                outDegMarket = sampleMmenOutDeg() ;
            
            uint nVisitedAreas = sampleMmenNumberVisitedAreas[bt]();
            double pMovement = sampleMmenMovementProbability[bt]();
                        
            // update runningTotal and eventually correct dailyTarget
            if ( runningTotal + dailyTarget + minSizeMman >= expectedTotalInputMarkets  ) {
                dailyTarget = expectedTotalInputMarkets - runningTotal;
                runningTotal = expectedTotalInputMarkets;
            }
            else {
                runningTotal += dailyTarget;
            }
            
            // create handle
            SimpleMiddleManDataHandler* handle = new SimpleMiddleManDataHandler( nMmen, outDegMarket, dailyTarget, bt, nVisitedAreas, pMovement );

            mmen_handles.push_back( handle );
            mmen_handles_bybt[bt].push_back( handle );
            
            nMmen++;
        }
        
    }
    
    //====== Instantiate containers that will contain the actors ======//
    
    allNodeVec = new std::vector<BaseNode*>();
    farmVec    = new std::vector<NodeFarm*>();
    mmanVec    = new std::vector<NodeMman*>();
    marketVec  = new std::vector<NodeMarket*>();
    vendorVec  = new std::vector<NodeVendor*>();
    
    //====== Instantiate state managers ======//

    farmStates =    new FarmStateManager();
    mmanStates =    new MiddlemanStateManager();
    marketStates =  new MarketStateManager();
    vendorStates =  new VendorStateManager();
    
    //====== Instantiate actors ======//
        
    // create farms
    for ( auto& farm_handle: farm_handles ) {
        NodeFarm* nf = farm_handle->createNodeFarm( this );
        farmVec->push_back( nf );
        allNodeVec->push_back( nf );
    }
    
    // create middlemen
    for ( auto& mman_handle: mmen_handles ) {
        NodeMman* nmm = mman_handle->createNodeMman( this );
        mmanVec->push_back( nmm );
        allNodeVec->push_back( nmm );
    }
    
    // create markets
    for ( auto& market_handle: market_handles ) {
        NodeMarket* nm = market_handle->createNodeMarket( this );
        marketVec->push_back( nm );
        allNodeVec->push_back( nm );
    }
    
    // create vendors (must be done after market instantiation)
    for ( auto& vendor_handle: vendor_handles ) {
        NodeVendor* nv = vendor_handle->createNodeVendor( this );
        vendorVec->push_back( nv );
        allNodeVec->push_back( nv );
    }
    
    
    //====== clean unused memory ======//
    
    for ( auto& farm_handle: farm_handles)
        delete farm_handle;
    
    for ( auto& mman_handle: mmen_handles )
        delete mman_handle;
    
    for ( auto& vendor_handle: vendor_handles )
        delete vendor_handle;
    
    for ( auto& market_handle: market_handles )
        delete market_handle;
        
}


// this function creates a commercial farm (all-in-all-out system, single species)
// with geometric time to next cycle
NodeFarm* WorldBuilder::createFarm( CommercialFarmDataHandler* farmHandle ) {
    
    uint id = farmHandle->id;
    BirdType bt = farmHandle->bt;
    uint batchSize = farmHandle->size;
    Geom::Point pos = farmHandle->pos;
    double p_refill = farmHandle->p_refill;
    uint ndays_rollout = farmHandle->ndays_rollout;
    uint area = farmHandle->catchmentArea;
    
    assert( p_refill > 0. );
    assert( batchSize > 0 );
    
    Farm* farm = new Farm( id, batchSize, {bt} );

    NodeFarm* nf = new NodeFarm( farm, farmStates->getStateEmpty(), area, pos, Clock() );
    nf->configCommercialFarm( bt, batchSize, p_refill, ndays_rollout );
        
    return nf;
}

NodeFarm* WorldBuilder::createFarm( CommercialFarmDataHandlerNegBin* farmHandle ) {
    
    uint id = farmHandle->id;
    BirdType bt = farmHandle->bt;
    uint batchSize = farmHandle->size;
    Geom::Point pos = farmHandle->pos;
    double p_negbin = farmHandle->p_negbin;
    double n_negbin = farmHandle->n_negbin;
    uint ndays_rollout = farmHandle->ndays_rollout;
    uint area = farmHandle->catchmentArea;
    
    assert( p_negbin > 0. );
    assert( n_negbin > 0. );
    assert( batchSize > 0 );
    
    Farm* farm = new Farm( id, batchSize, {bt} );

    NodeFarm* nf = new NodeFarm( farm, farmStates->getStateEmpty(), area, pos, Clock() );
    nf->configCommercialFarm( bt, batchSize, p_negbin, n_negbin, ndays_rollout );
        
    return nf;
}


// this function create a "simple" middleman
// trades a single bird species
// sells its stock as early as possible
NodeMman* WorldBuilder::createMman( SimpleMiddleManDataHandler* mmanHandle ) {
    BirdType bt = mmanHandle->bt;
    double target = mmanHandle->getDailyTargetFromFarms( bt );
    double pMovement = mmanHandle->pMovement;
    uint id = mmanHandle->id;
    uint outDegMarket = mmanHandle->outDegMarket;
    uint nVisitedAreas = mmanHandle->nVisitedAreas;
    uint capacityBirds = (uint)( target );
        
    Middleman* mman = new Middleman( id, {bt}, capacityBirds );
    NodeMman* nmm = new NodeMman( mman, mmanStates->getStateAwaiting() );
    
    nmm->configSimpleMiddleman( outDegMarket, nVisitedAreas, pMovement );
    //std::cout << outDegMarket << ",";
    
    return nmm;
    
}

NodeMarket* WorldBuilder::createMarket( MarketDataHandler* marketHandle ) {
    
    uint id = marketHandle->id;
    std::vector<BirdType> bts = marketHandle->tradedBirds;
    
    // compute market's bird capacity
    double capacity_tmp = 0.;
    for ( BirdType bt: bts ) {
        for ( VendorDataHandler* vdh: marketHandle->Buyers[bt] ) {
            capacity_tmp += vdh->target;
        }
        for ( VendorDataHandler* vdh: marketHandle->Sellers[bt] ) {
            capacity_tmp += vdh->target;
        }
    }
    
    uint capacity = static_cast<uint>( capacity_tmp * 2 + 100 );
    
    Market* market = new Market( id, bts, capacity );
    NodeMarket* nm = new NodeMarket( market, marketStates->getStateClose() ) ;
    
    // add layers and set proportion of birds sold from mmen to wholesalers in first layer
    for ( BirdType bt: bts )
        nm->configureLayersSingleSpecies( bt, marketHandle->maxLayers[bt], marketHandle->propSoldToWholesalerStack[bt][0] );
    
    // update vendorDataHandlers with buying and selling locations
    for ( BirdType bt: bts ) {
        for ( VendorDataHandler* vdh: marketHandle->Buyers[bt] ) {
            vdh->mB = nm;
        }
        for ( VendorDataHandler* vdh: marketHandle->Sellers[bt] ) {
            vdh->mS = nm;
        }
    }
    
    return nm;
}

NodeVendor* WorldBuilder::createVendor( VendorRetailerDataHandler* vendorHandle ) {
    uint id = vendorHandle->id;
    BirdType bt = vendorHandle->bt;
    NodeMarket* mB = vendorHandle->mB;
    NodeMarket* mS = vendorHandle->mS;
    uint dailyTarget = static_cast<uint>( vendorHandle->target );
    uint layer = vendorHandle->layer;
    double p_empty = vendorHandle->p_empty;
    double odUnsold = vendorHandle->odUnsold;
    double propUnsold = vendorHandle->propUnsold;
    bool prioritizesRepurposedBirds = vendorHandle->priotitizesRepurposed;

    uint capacity = static_cast<uint>( vendorHandle->capacity );
    
    Vendor* vendor = new Vendor( id, capacity, {bt} );
    NodeVendor* nv = new NodeVendor( vendor, vendorStates->getStateHome(), dailyTarget, layer, mB, mS );
    nv->configSimpleRetailer( p_empty, odUnsold, propUnsold, prioritizesRepurposedBirds );
    
    return nv;
}

NodeVendor* WorldBuilder::createVendor( VendorWholesalerDataHandler* vendorHandle ) {
    uint id = vendorHandle->id;
    BirdType bt = vendorHandle->bt;
    NodeMarket* mB = vendorHandle->mB;
    NodeMarket* mS = vendorHandle->mS;
    uint dailyTarget = static_cast<uint>( vendorHandle->target );
    uint layer = vendorHandle->layer;
    double p_empty = vendorHandle->p_empty;
    double odUnsold = vendorHandle->odUnsold;
    double propUnsold = vendorHandle->propUnsold;
    double p_2wholesalers = vendorHandle->p_2wholesalers;
    double p_2retailers = vendorHandle->p_2retailers;
    bool prioritizesRepurposedBirds = vendorHandle->priotitizesRepurposed;

    uint capacity = static_cast<uint>( vendorHandle->capacity );
    
    Vendor* vendor = new Vendor( id, capacity, { bt } );
    NodeVendor* nv = new NodeVendor( vendor, vendorStates->getStateHome(), dailyTarget, layer, mB, mS );
    nv->configSimpleWholesaler( p_empty, odUnsold, propUnsold, p_2wholesalers, p_2retailers, prioritizesRepurposedBirds );
    
    return nv;
}


