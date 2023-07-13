//
//  nodeMarket.cpp
//  CBM
//
//

#include "node.hpp"

//====== NodeMarket ======//

// constructor

NodeMarket::NodeMarket(Market* market_, StateMarket* state_, std::vector<bool> openDays_): market(market_), state(state_), openDays(openDays_) {
    
    auto& bts = market->getTradedBirdTypes();
    
    for ( const BirdType& bt: bts ) {
        
        maxLayers[bt] = 0;
        seller_wholesale_stack[bt] = std::vector<std::vector<NodeVendor*>>();
        buyer_wholesale_stack[bt] = std::vector<std::vector<NodeVendor*>>();
        buyer_retail_stack[bt] = std::vector<std::vector<NodeVendor*>>();
        
        seller_list[bt] = std::vector<NodeVendor*>();
        seller_list[bt].reserve( 50 );
        
    }
    
    nIncomingInfectedBirds = 0 ;
    nMmanTradedBirdsByFarm.clear() ;
    trackersMmanTradedBird = {} ;
    //this->addMmanTradedBird = std::bind( &NodeMarket::doNothing, this, std::placeholders::_1 ) ;

}

void NodeMarket::reset( ChickenPool* birdMngr ) {
    
    nIncomingInfectedBirds = 0 ;
    nMmanTradedBirdsByFarm.clear() ;
    
    // reset market attributes
    closeMarket() ;

    // reset inner market
    market->reset( birdMngr ) ;
    
}

void NodeMarket::configureLayersSingleSpecies( const BirdType& bt, const uint& nLayers, const double& propBirdsFromMiddlemenToWholesalers ) {
    
    maxLayers[bt] = nLayers;
    propBirdsMmen2Wholesalers[bt] = propBirdsFromMiddlemenToWholesalers;

    for ( uint layer = 0; layer < maxLayers[bt]; ++layer ) {    // set up empty lists of buyers and sellers
        seller_wholesale_stack[bt].push_back( std::vector<NodeVendor*>() );
        buyer_wholesale_stack[bt].push_back( std::vector<NodeVendor*>() );
        buyer_retail_stack[bt].push_back( std::vector<NodeVendor*>() );
    }
}


void NodeMarket::doTradingStepRequest(const Clock &clock, Random &randGen, BirdTransferManager& transferManager) {
    state->onDoTradingStepRequest( this, clock, randGen, transferManager );
}

void NodeMarket::resetRequest( ChickenPool* birdMngr ) {
    state->onResetRequest( this, birdMngr );
}

void NodeMarket::openMarket( const Clock &clock ) {

    nIncomingInfectedBirds = 0 ;
    nMmanTradedBirdsByFarm.clear() ;
    state->open( *this, clock );

}

// make sellers sell birds to buyers
void NodeMarket::doTradingStep( const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {
    for (auto& itemm: seller_list) {
        //const BirdType& bt = itemm.first;
        std::vector<NodeVendor*>& sellers = itemm.second;
        for (NodeVendor* nv: sellers) {
            nv->doSellStepRequest( clock, randGen, transferManager );
        }
    }
}


// make vendors in 'layer' decide whether to move to another market
void NodeMarket::doVendorDisplacementStep( const uint &layer, Random& randGen ) {
    auto& bts = market->getTradedBirdTypes();
    for ( const BirdType& bt: bts ) {
        
        // move wholesalers
        if ( layer < maxLayers[bt] - 1 ) {
            std::vector<NodeVendor*>& nvs = buyer_wholesale_stack[bt][layer];
            while( !nvs.empty() ) {
                auto& nv = nvs[0];
                nv->switchToSellingRequest( randGen );
            }
        }
        
        // move retailers
        if ( layer < maxLayers[bt] ) {
            std::vector<NodeVendor*>& nvs = buyer_retail_stack[bt][layer];
            while( !nvs.empty() ) {
                auto& nv = nvs[0];
                nv->switchToSellingRequest( randGen );
            }
        }
    }
}

void NodeMarket::doWholesaleTradingStep( const uint& layer, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {
    
    auto& bts = market->getTradedBirdTypes();
    for ( const BirdType& bt: bts ) {
        
        // check layer
        if ( layer >= maxLayers[bt] - 1 )
            continue;
        
        // make wholesalers sell to other vendors in same market
        // at the end, vendor switches to retail activity
        std::vector<NodeVendor*>& nvs_src = seller_wholesale_stack[bt][layer];
        while ( !nvs_src.empty() ) {
            auto& nv = nvs_src[0];
            nv->doWholesaleSellStepRequest( clock, randGen, transferManager );
        }
        
    }
}

// send vendors home
void NodeMarket::closeMarket() {
    auto& bts = market->getTradedBirdTypes();
    
    for ( const BirdType& bt: bts ) {
        // should not be needed might not be needed
        /*
        for ( uint layer = 0; layer < maxLayers[bt] - 1; ++layer ) {
            std::vector<NodeVendor*>& nvsb = buyer_wholesale_stack[bt][layer];
            while( !nvsb.empty() ) {
                nvsb[0]->goHomeRequest();
            }
        }
        
        for ( uint layer = 0; layer < maxLayers[bt]; ++layer ) {
            std::vector<NodeVendor*>& nvsb = buyer_retail_stack[bt][layer];
            while( !nvsb.empty() ) {
                nvsb[0]->goHomeRequest();
            }
        }
         */
    
        // return sellers
        std::vector<NodeVendor*>& nvss = seller_list[bt];
        while( !nvss.empty() ) {
            nvss[0]->goHomeRequest();
        }
    }
}

// add a wholesaler to stack of buyers
void NodeMarket::addBuyerWS(NodeVendor *vendor) {
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    uint layer = vendor->getVendorLayer();

    for ( const BirdType& bt: bts )
        buyer_wholesale_stack[bt][layer].push_back( vendor );
}

// add a retailer to stack of buyers
void NodeMarket::addBuyerR(NodeVendor *vendor) {
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    uint layer = vendor->getVendorLayer();

    for ( const BirdType& bt: bts )
        buyer_retail_stack[bt][layer].push_back( vendor );
}

// add a wholesaler to stack of wholesaler sellers
void NodeMarket::addSellerWS(NodeVendor *vendor) {
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    uint layer = vendor->getVendorLayer();

    for ( const BirdType& bt: bts )
        seller_wholesale_stack[bt][layer].push_back( vendor );
}

// add a retailer to list of retailing actors
void NodeMarket::addSellerR(NodeVendor *vendor) {
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    for ( const BirdType& bt: bts )
        seller_list[bt].push_back( vendor );
}

// remove retailer from stack of buyers
void NodeMarket::removeBuyerR(NodeVendor *vendor) {
    
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    uint layer = vendor->getVendorLayer();
   
    for (const BirdType& bt: bts) {
        auto it = buyer_retail_stack[bt][layer].begin();
        auto end = buyer_retail_stack[bt][layer].end();
        while( it < end ) {
            if ( *it == vendor ) {
                std::iter_swap( it, end - 1 );
                buyer_retail_stack[bt][layer].pop_back();
                break;
            }
            ++it;
        }
    }
}

// remove wholesaler from stack of buyers
void NodeMarket::removeBuyerWS(NodeVendor *vendor) {
    
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    uint layer = vendor->getVendorLayer();
    
    for (const BirdType& bt: bts) {
        auto it = buyer_wholesale_stack[bt][layer].begin();
        auto end = buyer_wholesale_stack[bt][layer].end();
        while( it < end ) {
            if ( *it == vendor ) {
                std::iter_swap( it, end - 1 );
                buyer_wholesale_stack[bt][layer].pop_back();
                break;
            }
            ++it;
        }
    }
}

// remove wholesaler from stack of wholesaler sellers
void NodeMarket::removeSellerWS(NodeVendor *vendor) {
    
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    uint layer = vendor->getVendorLayer();
    
    for (const BirdType& bt: bts) {
        auto it = seller_wholesale_stack[bt][layer].begin();
        auto end = seller_wholesale_stack[bt][layer].end();
        while( it < end ) {
            if ( *it == vendor ) {
                std::iter_swap( it, end - 1 );
                seller_wholesale_stack[bt][layer].pop_back();
                break;
            }
            ++it;
        }
    }
}

// remove vendor from list of retailing agents
void NodeMarket::removeSeller(NodeVendor *vendor) {
    auto& bts = vendor->getContext()->getTradedBirdTypes();
    for (const BirdType& bt: bts) {
        auto it = seller_list[bt].begin();
        auto end = seller_list[bt].end();
        while( it < end ) {
            if ( *it == vendor ) {
                std::iter_swap( it, end - 1 );
                seller_list[bt].pop_back();
                break;
            }
            ++it;
        }
    }
}

std::vector<NodeVendor*>& NodeMarket::getSellersWholesalers(const BirdType& bt, const uint& layer) {
    return seller_wholesale_stack[bt][layer];
}

std::vector<NodeVendor*>& NodeMarket::getBuyersWholesalers(const BirdType& bt, const uint& layer) {
    return buyer_wholesale_stack[bt][layer];
}

std::vector<NodeVendor*>& NodeMarket::getBuyersRetailers(const BirdType& bt, const uint& layer) {
    return buyer_retail_stack[bt][layer];
}


// returns true if market is open
bool NodeMarket::isOpen() {
    return state->isOpen();
}

// register vendor to market
void NodeMarket::addVendor(NodeVendor* vendor) {
    vendorPartners.push_back(vendor);
}

// remove vendor from list of vendors in this market
void NodeMarket::removeVendor(NodeVendor* vendor) {
    auto it = vendorPartners.begin();
    auto end = vendorPartners.end();
    
    while( it < end ) {
        if ( *it == vendor ) {
            std::iter_swap( it, vendorPartners.end() - 1 );
            vendorPartners.pop_back();
        }
        ++it;
    }
}

void NodeMarket::setPropSoldMmenToWholesalers( const BirdType &bt, const double &prop ) {
    checkUnitInterval( prop );
    propBirdsMmen2Wholesalers[bt] = prop;
}

bool NodeMarket::isWorkingDay(const Clock& clock) {
    return openDays[clock.getCurrDay() % MARKET_NDAYS_CYCLE];
}

void NodeMarket::printBuyersDemography() {
    
    auto& bts = market->getTradedBirdTypes();
    
    for ( auto bt: bts ) {
        
        uint nRetailers = 0;
        uint nWholesalers = 0;
        
        std::cout << "Market: " << market->getId() << ", bt: " << constants::getStringFromBirdType( bt ) << ", ";
        std::cout << "W: " << nWholesalers << ", R: " << nRetailers << std::endl;
    }
}


// store properties of incoming birds bought from middlemen
void NodeMarket::addMmanTradedBird( pBird& bird ) {
    
    for ( auto& tracker : trackersMmanTradedBird )
        tracker( bird ) ;
    
}

// counts number of chickens visited
void NodeMarket::doRecordBirdSource() {
    
    nMmanTradedBirdsByFarm.clear() ;
    
    trackersMmanTradedBird.push_back( std::bind( &NodeMarket::trackFarmOrigin, this, std::placeholders::_1 ) ) ;
    
    
    //this->addMmanTradedBird = std::bind( &NodeMarket::addTradedBird, this, std::placeholders::_1 ) ;
    
}

void NodeMarket::doRecordIncomingInfectedBirds( bool checkExposed ) {
    
    nIncomingInfectedBirds = 0 ;
    
    if ( checkExposed ) { // track both E and I birds
        
        trackersMmanTradedBird.push_back( std::bind( &NodeMarket::trackExposedAndInfectedBird, this, std::placeholders::_1 ) ) ;
        
    }
    else { // track I birds only
        
        trackersMmanTradedBird.push_back( std::bind( &NodeMarket::trackInfectedBird, this, std::placeholders::_1 ) ) ;
        
    }
        
}


// updates count of birds traded from a given farm during this cycle
void NodeMarket::trackFarmOrigin( pBird &bird ) {
    
    ++nMmanTradedBirdsByFarm[bird->id.idFarm] ;
    
}

// updates count of infectious incoming birds
void NodeMarket::trackInfectedBird( pBird& bird ) {
    
    if ( bird->infState != 0 )
        ++nIncomingInfectedBirds ;
    
}

// updates count of exposed and infectious incoming birds
void NodeMarket::trackExposedAndInfectedBird( pBird& bird ) {
    
    if ( ( bird->infState ) != 0 or ( bird->pendingInfections > 0 ) )
        ++nIncomingInfectedBirds ;
    
}

