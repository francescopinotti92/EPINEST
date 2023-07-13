//
//  nodeVendor.cpp
//  CBM
//
//

#include "node.hpp"

//====== Node ======//

/*
// constructor
template<typename T, typename U, typename V>
Node<T,U,V>::Node(T* context_, U* state_, V* nbrs_): context(context_), state(state_), nbrs(nbrs_) {};

template<typename T, typename U, typename V>
void Node<T,U,V>::update() {
    // call update on state
    //U* newState = state->update(this);
    
    // update state if necessary
    //
};


// setters

template<typename T, typename U, typename V>
void Node<T,U,V>::setState(U* newState) {
    state = newState;
}

template<typename T, typename U, typename V>
void Node<T,U,V>::setNbrs(V* nbrs_) {
    nbrs = nbrs_;
}
*/


//====== NodeVendor ======//

// constructor
NodeVendor::NodeVendor(Vendor* vendor_, StateVendor* state_, uint dailyTarget_, uint vendorLayer_, NodeMarket* marketBuy_, NodeMarket* marketSell_): vendor( vendor_ ), state( state_ ), dailyTarget(dailyTarget_), vendorLayer( vendorLayer_ ), marketBuy( marketBuy_ ), marketSell( marketSell_ ) {
    
    marketCurrent = nullptr;

    nBirdsWholesaleToWholesalers = 0;
    nBirdsWholesaleToRetailers = 0;
    nBirdsRetail = 0;
    
    nBirdsBoughtToday = 0;
};

//=== Configurers ===//

// configures a simple vendor:
// sells through retail only.
void NodeVendor::configSimpleRetailer( const double& p_empty, const double& odUnsold, const double& propUnsold, const bool& prioritizesRepurposed ) {
   
    this->goToMarketBuying = std::bind( &NodeVendor::goToMarketBuyingR, this );
    this->goToMarketSelling = std::bind( &NodeVendor::goToMarketSellingR, this );
    this->goHomeBuyer = std::bind( &NodeVendor::goToMarketBuyingR, this );

    this->scheduleDailyTrade = std::bind( &NodeVendor::scheduleDailyTradeNegBinVendor, this, p_empty, odUnsold, propUnsold, std::placeholders::_1 );
    
    if ( prioritizesRepurposed )
        this->getTransactionDetails = std::bind( &NodeVendor::getTransactionDetailsPriority, this, std::placeholders::_1 );
    else
        this->getTransactionDetails = std::bind( &NodeVendor::getTransactionDetailsNoPriority, this, std::placeholders::_1 );

    
    // set empty function that does nothing
    this->doWholesaleSellStep = []( const Clock& clock, Random& randGen, BirdTransferManager& transferManager) {};
    
    this->getVendorType = std::bind( &NodeVendor::returnRetailerType, this );
    
    
}

// configures a simple wholesaler
// sells through wholesale and retail
void NodeVendor::configSimpleWholesaler(const double& p_empty, const double& odUnsold, const double& propUnsold, const double& p_wholesalers, const double& p_retailers, const bool& prioritizesRepurposed ) {
    
    this->goToMarketBuying = std::bind( &NodeVendor::goToMarketBuyingWS, this );
    this->goToMarketSelling = std::bind( &NodeVendor::goToMarketSellingWS, this );
    this->goHomeBuyer = std::bind( &NodeVendor::goToMarketBuyingWS, this );

    this->scheduleDailyTrade = std::bind( &NodeVendor::scheduleDailyTradeNegBinWS, this, p_empty, odUnsold, propUnsold, p_wholesalers, p_retailers, std::placeholders::_1 );
    
    if ( prioritizesRepurposed )
        this->getTransactionDetails = std::bind( &NodeVendor::getTransactionDetailsPriority, this, std::placeholders::_1 );
    else
        this->getTransactionDetails = std::bind( &NodeVendor::getTransactionDetailsNoPriority, this, std::placeholders::_1 );
    
    this->doWholesaleSellStep = std::bind( &NodeVendor::sellStepWholesale, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
    
    this->getVendorType = std::bind( &NodeVendor::returnWholesalerType, this );
}


void NodeVendor::goToMarketRequest( Random& randGen ) {
    state->onGoToMarketRequest( this, randGen );
}

void NodeVendor::goHomeRequest() {
    state->onGoHomeRequest( this );
}

void NodeVendor::switchToSellingRequest( Random &randGen ) {
    state->onSwitchToSellingRequest( this, randGen );
}

void NodeVendor::doWholesaleSellStepRequest( const Clock &clock, Random &randGen, BirdTransferManager &transferManager ) {
    state->onDoWholesaleSellStepRequest( this, clock, randGen, transferManager );
}

/*
void NodeVendor::evalSellingRoleRequest( Random& randGen ) {
    state->onEvalSellingRoleRequest( this, randGen );
}

void NodeVendor::forceSellingRoleRequest( Random &randGen ) {
    state->onForceSellingRoleRequest( this, randGen );
}
 */

uint NodeVendor::processBatchFromMmanRequest( const uint& nbirdRequest ) {
    return state->onProcessBatchFromMmanRequest( this, nbirdRequest );
}

uint NodeVendor::processBatchFromVendorRequest( const uint& nbirdRequest ) {
    return state->onProcessBatchFromVendorRequest( this, nbirdRequest );
}


void NodeVendor::doSellStepRequest( const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {
    state->onSellRequest( this, clock, randGen, transferManager );
}

void NodeVendor::resetRequest( ChickenPool* birdMngr ) {
    state->onResetRequest( this, birdMngr );
}

// transfers unsold birds from current market to own home stock
void NodeVendor::withdrawFromMarket() {
    Market* market = marketCurrent->getContext();
    
    {   // return newly bought birds
        std::vector<uint>& birdIdxVec = vendor->getMarketedNewBirdsIndexes();
        for ( uint& birdIdx: birdIdxVec ) {
            pBird bird = market->removeBird( birdIdx );
            vendor->insertBird( std::move( bird ) );
        }
        birdIdxVec.clear();
    }
    
    {   // return repurposed birds
        std::vector<uint>& birdIdxVec = vendor->getMarketedRepurposedBirdsIndexes();
        for ( uint& birdIdx: birdIdxVec ) {
            pBird bird = market->removeBird( birdIdx );
            vendor->insertBird( std::move( bird ) );
        }
        birdIdxVec.clear();
    }
    
    marketCurrent = nullptr;
}

// evaluate how many birds can be accepted at most in a single transaction
uint NodeVendor::evalBatchSizeTarget( const uint& nbirdsRequest ) {
    
    // check:
    // 1) Did not meet own daily quota already
    // 2) Does not exceed own homespace
    // 3) Does not exceed own shared market space
    
    if ( nBirdsBoughtToday >= dailyTarget ) // 1)
        return 0;

    uint availableSpaceVendor = 0;
    if ( vendor->getCapacity() < vendor->getSizeBirdsMarket() ) { // 2
        return 0;
    }
    else
        availableSpaceVendor = vendor->getCapacity() - vendor->getSizeBirdsMarket();

    uint availableSpaceMarket = (uint)( marketCurrent->getContext()->getAvailableSpace() );
    uint availableSpace = std::min( availableSpaceMarket, availableSpaceVendor );
    
    if ( availableSpace == 0 ) // 3)
        return 0;
    
    uint missingQuotaAmount = ( uint )( dailyTarget - nBirdsBoughtToday );
    
    return std::min( std::min( missingQuotaAmount, availableSpace ) , nbirdsRequest );
}

// Returns how many birds could be sold from this vendor as part of retail
uint NodeVendor::evalRetailPurchaseRequest( const uint& nBirdsRequest ) {
    
    // the only constrain is the amount of birds available for retail
    return std::min( nBirdsRetail, nBirdsRequest ) ;
    
}


// make wholesaler go to buying
void NodeVendor::goToMarketBuyingWS() {
    if ( marketCurrent == nullptr ) {
        vendor->transferBirds( marketBuy->getContext() );
        marketCurrent = marketBuy;
        marketCurrent->addBuyerWS( this );
    }
}

void NodeVendor::goToMarketBuyingR() {
    if (marketCurrent == nullptr) {
        vendor->transferBirds( marketBuy->getContext() );
        marketCurrent = marketBuy;
        marketCurrent->addBuyerR( this );
    }
}

void NodeVendor::goToMarketSellingWS() {
    
    if (marketCurrent == nullptr) { // go directly to sell birds
        vendor->transferBirds( marketSell->getContext() );
        marketCurrent = marketSell;
        marketCurrent->addSellerWS(this);
    }
    else if (marketCurrent == marketBuy) { // go from marketBuy to marketSell
        
        // move birds if markets differ
        if (marketBuy != marketSell) {
            Market* marketSrc = marketCurrent->getContext();
            Market* marketDst = marketSell->getContext();
            
            // move newly bought birds
            {
                std::vector<uint>& birdIdxVec = vendor->getMarketedNewBirdsIndexes();
                for ( uint& birdIdx: birdIdxVec ) {
                    pBird bird = marketSrc->removeBird( birdIdx );
                    birdIdx = ( marketDst->insertBird( std::move( bird ) ) ).index; // update index
                }
            }
            
            // move repurposed birds
            {
                std::vector<uint>& birdIdxVec = vendor->getMarketedRepurposedBirdsIndexes();
                for ( uint& birdIdx: birdIdxVec ) {
                    pBird bird = marketSrc->removeBird( birdIdx );
                    birdIdx = ( marketDst->insertBird( std::move( bird ) ) ).index; // update index
                }
            }
        }
        
        // update markets' buyer and seller lists
        marketCurrent->removeBuyerWS(this);
        marketCurrent = marketSell;
        marketCurrent->addSellerWS(this);
    }
    else
        return;
}

void NodeVendor::goToMarketSellingR() {
        
    if (marketCurrent == nullptr) { // go directly to sell birds
        vendor->transferBirds( marketSell->getContext() );
        marketCurrent = marketSell;
        marketCurrent->addSellerR(this);
    }
    else if (marketCurrent == marketBuy) { // go from marketBuy to marketSell
        
        // move birds if markets differ
        if (marketBuy != marketSell) {
            Market* marketSrc = marketCurrent->getContext();
            Market* marketDst = marketSell->getContext();

            
            // move newly bought birds
            {
                std::vector<uint>& birdIdxVec = vendor->getMarketedNewBirdsIndexes();
                for ( uint& birdIdx: birdIdxVec ) {
                    pBird bird = marketSrc->removeBird( birdIdx );
                    birdIdx = ( marketDst->insertBird( std::move( bird ) ) ).index; // update index
                }
            }
            
            // move repurposed birds
            {
                std::vector<uint>& birdIdxVec = vendor->getMarketedRepurposedBirdsIndexes();
                for ( uint& birdIdx: birdIdxVec ) {
                    pBird bird = marketSrc->removeBird( birdIdx );
                    birdIdx = ( marketDst->insertBird( std::move( bird ) ) ).index; // update index
                }
            }
        }
        
        // update markets' buyer and seller lists
        marketCurrent->removeBuyerR(this);
        marketCurrent = marketSell;
        marketCurrent->addSellerR(this);
    }
    else
        return;
}

// make a wholesaler register to wholesaler list
// should be called only after terminating wholesale activity
// ??? perhaps implement additional cases (e.g. when vendor not in stack)
void NodeVendor::goToRetail() {
    marketCurrent->removeSellerWS( this );
    marketCurrent->addSellerR( this );
}

void NodeVendor::goHomeBuyerWS() {
    withdrawFromMarket();
    getMarketBuying()->removeBuyerWS( this );
    resetBirdBoughtCount();
}

void NodeVendor::goHomeBuyerR() {
    withdrawFromMarket();
    getMarketBuying()->removeBuyerR( this );
    resetBirdBoughtCount();

}

void NodeVendor::goHomeSeller() {
    withdrawFromMarket();
    getMarketSelling()->removeSeller( this );
    resetBirdBoughtCount();
}

// make vendor sell to end-consumers
// assumes that all markets operate over the same time window
void NodeVendor::sellStepRetail( const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {
    
    if ( nBirdsRetail == 0 )
        return; // nothing to sell
    
    uint nbirdsStep = 0; // birds to sell during this timestep

    uint nStepsAvail = MARKET_CLOSING_HOUR - clock.getCurrHour();  // compute number of steps that remain before market closure
    
    if ( nStepsAvail < 0 ) // handle case where current time and closing time occur on consecutive days
        nStepsAvail += HOURS_DAY;
    
    if ( nStepsAvail > 1 ) {
        
        nbirdsStep = std::min( nBirdsRetail, uint( round( randGen.getUni() + randGen.getPoisson( nBirdsRetail / ( nStepsAvail ) ) ) ) ) ; // draw number of birds to sell during this timestep
    
    }
    else
        nbirdsStep = nBirdsRetail;
    
    // transfer birds to community
    if ( nbirdsStep > 0 ) {
        transferManager.transferBirdsVendor2Community( this, marketCurrent, nbirdsStep, randGen );
        nBirdsRetail -= nbirdsStep; // update #birds available for retail
    }
    
}

// WARNING: this function assumes a single species traded
void NodeVendor::sellStepWholesale( const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {
    
    const BirdType& bt = vendor->getTradedBirdTypes()[0];
    
    // sell to other wholesalers
    if ( nBirdsWholesaleToWholesalers > 0 ) {
        std::vector<NodeVendor*>& buyers = marketCurrent->getBuyersWholesalers( bt, vendorLayer + 1 ); // get potential buyers
        
        uint nbirdsSellBatch = 0;
        
        for ( auto& buyer: buyers ) {
            
            nbirdsSellBatch = buyer->processBatchFromVendorRequest( nBirdsWholesaleToWholesalers ); // check how many birds can be traded in this transaction
            if ( nbirdsSellBatch > 0 ) {
                
                transferManager.transferBirdsVendor2Vendor( this, buyer, marketCurrent, nbirdsSellBatch, randGen );
                nBirdsWholesaleToWholesalers -= nbirdsSellBatch;
            }
            
            if ( nBirdsWholesaleToWholesalers == 0 )
                break;
            
            if ( buyers.size() == 0 ) {
                break;
            }
        }
    }
    
    // repurpose birds to be sold to retailers (or end-consumers)
    nBirdsWholesaleToRetailers += nBirdsWholesaleToWholesalers;
    nBirdsWholesaleToWholesalers = 0;
    
    // sell to retailers
    if ( nBirdsWholesaleToRetailers > 0 ) {
        std::vector<NodeVendor*>& buyers = marketCurrent->getBuyersRetailers( bt, vendorLayer + 1 ); // get potential buyers
    
        uint nbirdsSellBatch = 0;
        
        for ( auto& buyer: buyers ) {
            
            nbirdsSellBatch = buyer->processBatchFromVendorRequest( nBirdsWholesaleToRetailers ); // check how many birds can be traded for this transaction
            if ( nbirdsSellBatch > 0 ) {
                transferManager.transferBirdsVendor2Vendor( this, buyer, marketCurrent, nbirdsSellBatch, randGen );
                nBirdsWholesaleToRetailers -= nbirdsSellBatch;
            }
            
            if ( nBirdsWholesaleToRetailers == 0 )
                break;
        }
    }
    
    // repurpose birds to be sold to end-consumers
    nBirdsRetail += nBirdsWholesaleToRetailers;
    nBirdsWholesaleToRetailers = 0;
    
    // vendor goes to retail
    this->goToRetail();
    
}


// computes birds to be sold as retail practice
// unsold birds are drawn from a negative binomial with given proportion and overdispersion
// no birds are scheduled to wholesale practice
void NodeVendor::scheduleDailyTradeNegBinVendor(const double& p_empty, const double& odUnsold, const double& propUnsold, Random& randGen) {
    
    uint nBirdsAvail = vendor->getSizeBirdsMarket();
    
    if ( randGen.getBool( p_empty ) ) { // no birds left unsold at the end of the day
        nBirdsRetail = nBirdsAvail;
    }
    else { // draw number of birds left unsold from negative binomial
        
        double mu = propUnsold * nBirdsAvail; // expected number of unsold chickens
        double n = 1. / odUnsold; // variance is mu * ( 1 + mu * odUnsold )
        double p  = n / ( mu + n );
        
        uint nBirdsUnsold = std::min( randGen.getNegBinom( p, n ), nBirdsAvail );
        nBirdsRetail = nBirdsAvail - nBirdsUnsold;
    }
}

// computes birds to be sold as wholesale and retail practice
// unsold birds are drawn from a negative binomial with given proportion and overdispersion
// birds to be sold in the context of wholesale practice are drawn from a binomial distribution
void NodeVendor::scheduleDailyTradeNegBinWS( const double& p_empty, const double& odUnsold, const double& propUnsold, const double& p_wholesalers, const double& p_retailers, Random& randGen ) {
    
    uint nBirdsAvail = vendor->getSizeBirdsMarket();
    uint nBirdsSold = 0;
    
    // compute number of birds sold in this day (and those that remain unsold)
    if ( randGen.getBool( p_empty ) ) { // no birds left unsold at the end of the day
        nBirdsSold = nBirdsAvail;
    }
    else { // draw number of birds left unsold from negative binomial
        
        double mu = propUnsold * nBirdsAvail; // expected number of unsold chickens
        double n = 1. / odUnsold; // variance is mu * ( 1 + mu * odUnsold )
        double p  = n / ( mu + n );
        
        nBirdsSold = nBirdsAvail - std::min( randGen.getNegBinom( p, n ), nBirdsAvail );
    }
    
    // compute number of birds that are sold to other wholesalers
    nBirdsWholesaleToWholesalers = randGen.getBinom( p_wholesalers, nBirdsSold );
    nBirdsSold -= nBirdsWholesaleToWholesalers;
    
    // compute number of birds that are sold to other retailers
    nBirdsWholesaleToRetailers = randGen.getBinom( p_retailers / ( 1. - p_wholesalers ), nBirdsSold );
    nBirdsSold -= nBirdsWholesaleToRetailers;

    // compute number of birds sold as part of retail activity
    nBirdsRetail = nBirdsSold;
    
}


// computes counts of repurposed and new birds to be sold in new transaction
// prioritizies sale of repurposed birds
std::pair<uint,uint> NodeVendor::getTransactionDetailsPriority( const uint& amount ) {
    std::pair<uint, uint> res = std::make_pair( 0, 0 );
    res.first = std::min( vendor->getSizeRepurposedBirdsMarket(), amount );
    if ( res.first < amount ) {
        res.second = std::min( vendor->getSizeNewBirdsMarket(), amount - res.first );
    }
    return res;
}


// computes counts of repurposed and new birds to be sold in new transaction
// does not prioritize sale of repurposed birds
std::pair<uint,uint> NodeVendor::getTransactionDetailsNoPriority( const uint& amount ) {
    std::pair<uint, uint> res = std::make_pair( 0, 0 );
    double f = vendor->getSizeRepurposedBirdsMarket() * 1. / vendor->getSizeBirdsMarket();
    uint nrep = std::min( (uint)round( 0.5 + f * amount ), amount );
    res.first = std::min( nrep, vendor->getSizeRepurposedBirdsMarket() );
    if ( res.first < amount ) {
        res.second = std::min( vendor->getSizeNewBirdsMarket(), amount - res.first );
    }
    return res;
}



ActorType NodeVendor::returnRetailerType() {
    return ActorType::retailer;
}

ActorType NodeVendor::returnWholesalerType() {
    return ActorType::wholesaler;
}

void NodeVendor::increaseBirdBoughtCount( const uint& nbirds ) {
    nBirdsBoughtToday += nbirds;
}

void NodeVendor::resetBirdBoughtCount() {
    nBirdsBoughtToday = 0;
}

void NodeVendor::decreaseBirdRetailCount( const uint& nbirds ) {
    
    assert( nBirdsRetail > nbirds ) ; // check there are enough birds
    nBirdsRetail -= nbirds ;
}


void NodeVendor::reset( ChickenPool* birdMngr ) {
    // reset NodeVendor attributes
    marketCurrent = nullptr;
    nBirdsWholesaleToWholesalers = 0;
    nBirdsWholesaleToRetailers = 0;
    nBirdsRetail = 0;
    nBirdsBoughtToday = 0;
    
    // reset inner vendor attributes
    vendor->reset( birdMngr );
}
