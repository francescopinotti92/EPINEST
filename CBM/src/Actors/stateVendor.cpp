//
//  stateVendor.cpp
//  CBM
//
//

#include "node.hpp"

//====== Vendor states ======//

//=== simple vendor (retailer) states ===//

void StateVendorAtHome::onGoToMarketRequest(NodeVendor *nv, Random& randGen ) {
    // check if both buying and selling markets are open
    if ( nv->getMarketBuying()->isOpen() and nv->getMarketSelling()->isOpen() ) {
        if ( nv->getSizeBirds() < nv->getContext()->getCapacity() ) {
            nv->goToMarketBuying();
            nv->setState( stateBuy );
        }
        else {
            /*
            if ( nv->getContext()->getId() == 328 ) {
                std::cout << "hi from onGoToMarketRequest " << std::endl;
            }*/
            nv->goToMarketSelling();
            nv->scheduleDailyTrade( randGen );
            nv->setState( stateSell );
        }
    }
}
void StateVendorAtHome::onGoHomeRequest(NodeVendor *nv) {};

void StateVendorAtHome::onSwitchToSellingRequest( NodeVendor* nv, Random& randGen ) {};

void StateVendorAtHome::onDoWholesaleSellStepRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {};

//void StateVendorAtHome::onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};
//void StateVendorAtHome::onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};

uint StateVendorAtHome::onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
uint StateVendorAtHome::onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
void StateVendorAtHome::onSellRequest( NodeVendor *nv, const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {};

void StateVendorAtHome::onResetRequest( NodeVendor* nv, ChickenPool* birdMngr ) {
    nv->reset( birdMngr );
}

void StateVendorAtHome::setStateBuy(StateVendorBuy* stateBuy) {
    this->stateBuy = stateBuy;
}

void StateVendorAtHome::setStateSell(StateVendorSell *stateSell) {
    this->stateSell = stateSell;
}

//~~~~~~~~~~~~~~~~//

void StateVendorBuy::onGoToMarketRequest(NodeVendor *nv, Random& randGen) {};
void StateVendorBuy::onGoHomeRequest(NodeVendor *nv) {
    nv->goHomeBuyer();
    nv->setState( stateHome );
    nv->resetBirdBoughtCount();
}

void StateVendorBuy::onSwitchToSellingRequest( NodeVendor* nv, Random& randGen ) {
    nv->goToMarketSelling();
    nv->scheduleDailyTrade( randGen );
    nv->setState( stateSell );
}

void StateVendorBuy::onDoWholesaleSellStepRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {};

uint StateVendorBuy::onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) {
    return nv->evalBatchSizeTarget( nbirds );
}
uint StateVendorBuy::onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) {
    return nv->evalBatchSizeTarget( nbirds );
}
void StateVendorBuy::onSellRequest(NodeVendor *nv, const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {};
void StateVendorBuy::onResetRequest( NodeVendor* nv, ChickenPool* birdMngr ) {
    nv->reset( birdMngr );
    nv->setState( stateHome );
}

void StateVendorBuy::setStateHome(StateVendorAtHome* stateHome) {
    this->stateHome = stateHome;
}
void StateVendorBuy::setStateSell(StateVendorSell* stateSell) {
    this->stateSell = stateSell;
}

//~~~~~~~~~~~~~~~~//

void StateVendorSell::onGoToMarketRequest(NodeVendor *nv, Random& randGen) {};
void StateVendorSell::onGoHomeRequest(NodeVendor *nv) {
    nv->goHomeSeller();
    nv->setState(stateHome);
    nv->resetBirdBoughtCount();
}
void StateVendorSell::onSwitchToSellingRequest( NodeVendor* nv, Random& randGen ) {};

void StateVendorSell::onDoWholesaleSellStepRequest( NodeVendor* nv, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {
    
    nv->doWholesaleSellStep( clock, randGen, transferManager );
}

//void StateVendorSell::onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};
//void StateVendorSell::onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};

uint StateVendorSell::onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
uint StateVendorSell::onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
void StateVendorSell::onSellRequest(NodeVendor *nv, const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {
    nv->sellStepRetail( clock, randGen, transferManager );
}
void StateVendorSell::onResetRequest( NodeVendor* nv, ChickenPool* birdMngr ) {
    nv->reset( birdMngr );
    nv->setState( stateHome );
}

void StateVendorSell::setStateHome(StateVendorAtHome* stateHome) {
    this->stateHome = stateHome;
}

//=== wholesaler states ===//
/*
void StateWSAtHome::onGoToMarketRequest(NodeVendor *nv, Random& randGen) {
    // check if both a vendor's markets are open
    if ( nv->getMarketBuying()->isOpen() and nv->getMarketSelling()->isOpen() ) {
        
        if ( nv->getSizeBirds() < nv->getDailyTarget() ) {
            nv->goToMarketBuying();
            nv->setState( stateBuy );
        }
        else {
            nv->goToMarketSell();
            nv->scheduleDailyTrade( randGen );
            nv->setState( stateSell );
        }
        
    }
}
void StateWSAtHome::onGoHomeRequest(NodeVendor *nv) {};
void StateWSAtHome::onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};
void StateWSAtHome::onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};
uint StateWSAtHome::onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
uint StateWSAtHome::onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
void StateWSAtHome::onSellRequest(NodeVendor *nv, const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {};

void StateWSAtHome::setStateBuy(StateWSBuy* stateBuy) {
    this->stateBuy = stateBuy;
}

void StateWSAtHome::setStateSell( StateWSWholesaleSell* stateSell ) {
    this->stateSell = stateSell;
}

//~~~~~~~~~~~~~~~~//

void StateWSBuy::onGoToMarketRequest(NodeVendor *nv, Random& randGen) {};
void StateWSBuy::onGoHomeRequest(NodeVendor *nv) {
    nv->withdrawFromMarket();
    nv->getMarketBuying()->removeBuyer(nv);
    nv->setState(stateHome);
}
void StateWSBuy::onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) {
    if ( nv->getContext()->getSizeBirdsMarket() >= nv->getDailyTarget() ) {
        nv->goToMarketSell();
        nv->scheduleDailyTrade( randGen );
        nv->setState( stateWholesale );
    }
}
void StateWSBuy::onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) {
    nv->goToMarketSell();
    nv->scheduleDailyTrade( randGen );
    nv->setState( stateWholesale );
}

uint StateWSBuy::onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) {
    return nv->evalBatchSizeTarget( nbirds );
}
uint StateWSBuy::onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) {
    return nv->evalBatchSizeTarget( nbirds );
}

void StateWSBuy::onSellRequest(NodeVendor *nv, const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {};


void StateWSBuy::setStateHome(StateWSAtHome* stateHome) {
    this->stateHome = stateHome;
}
void StateWSBuy::setStateSell(StateWSWholesaleSell* stateWholesale) {
    this->stateWholesale = stateWholesale;
}

//~~~~~~~~~~~~~~~~//

void StateWSWholesaleSell::onGoToMarketRequest(NodeVendor *nv, Random& randGen) {};
void StateWSWholesaleSell::onGoHomeRequest(NodeVendor *nv) {
    nv->withdrawFromMarket();
    nv->getMarketSelling()->removeSeller( nv );
    nv->setState( stateHome );
}
void StateWSWholesaleSell::onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};
void StateWSWholesaleSell::onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};
uint StateWSWholesaleSell::onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
uint StateWSWholesaleSell::onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
void StateWSWholesaleSell::onSellRequest(NodeVendor *nv, const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {
    nv->sellStepWholesale( clock, randGen, transferManager );
    nv->setState( stateRetail );
}

void StateWSWholesaleSell::setStateHome(StateWSAtHome* stateHome) {
    this->stateHome = stateHome;
}
void StateWSWholesaleSell::setStateRetail(StateWSRetailSell* stateRetail) {
    this->stateRetail = stateRetail;
}

//~~~~~~~~~~~~~~~~//

void StateWSRetailSell::onGoToMarketRequest(NodeVendor *nv, Random& randGen) {};
void StateWSRetailSell::onGoHomeRequest(NodeVendor *nv) {
    nv->withdrawFromMarket();
    nv->getMarketSelling()->removeSeller( nv );
    nv->setState( stateHome );
}

void StateWSRetailSell::onEvalSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};
void StateWSRetailSell::onForceSellingRoleRequest( NodeVendor* nv, Random& randGen ) {};

uint StateWSRetailSell::onProcessBatchFromMmanRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
uint StateWSRetailSell::onProcessBatchFromVendorRequest( NodeVendor* nv, const uint& nbirds ) {
    return 0;
}
void StateWSRetailSell::onSellRequest(NodeVendor *nv, const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {
    nv->sellStepRetail( clock, randGen, transferManager );
}

void StateWSRetailSell::setStateHome(StateWSAtHome* stateHome) {
    this->stateHome = stateHome;
}

 */

//====== Vendor State Manager ======//

// constructor
VendorStateManager::VendorStateManager() {
   
    stateHome = new StateVendorAtHome();
    stateBuy = new StateVendorBuy();
    stateSell = new StateVendorSell();
    
    stateHome->setStateBuy( stateBuy );
    stateHome->setStateSell( stateSell );
    stateBuy->setStateHome( stateHome );
    stateBuy->setStateSell( stateSell );
    stateSell->setStateHome( stateHome );
    
    /*
    stateWSHome = new StateWSAtHome();
    stateWSBuy = new StateWSBuy();
    stateWSWholesaleSell = new StateWSWholesaleSell();
    stateWSRetailSell = new StateWSRetailSell();
    
    stateWSHome->setStateBuy( stateWSBuy );
    stateWSHome->setStateSell( stateWSWholesaleSell );
    stateWSBuy->setStateHome( stateWSHome );
    stateWSBuy->setStateSell( stateWSWholesaleSell );
    stateWSWholesaleSell->setStateHome( stateWSHome );
    stateWSWholesaleSell->setStateRetail( stateWSRetailSell );
    stateWSRetailSell->setStateHome( stateWSHome );
    */
}

// destructor
VendorStateManager::~VendorStateManager() {
    stateHome->setStateBuy( nullptr );
    stateHome->setStateSell( nullptr );
    stateBuy->setStateHome( nullptr );
    stateBuy->setStateSell( nullptr );
    stateSell->setStateHome( nullptr );

    delete stateHome;
    delete stateBuy;
    delete stateSell;
    
    /*
    stateWSHome->setStateBuy( nullptr );
    stateWSHome->setStateSell( nullptr );
    stateWSBuy->setStateHome( nullptr );
    stateWSBuy->setStateSell( nullptr );
    stateWSWholesaleSell->setStateHome( nullptr );
    stateWSWholesaleSell->setStateRetail( nullptr );
    stateWSRetailSell->setStateHome( nullptr );
    
    delete stateWSHome;
    delete stateWSBuy;
    delete stateWSWholesaleSell;
    delete stateWSRetailSell;
     */
    
}

// returns at home state
StateVendorAtHome* VendorStateManager::getStateHome() {
    return stateHome;
}

// returns buying (at market) state
StateVendorBuy* VendorStateManager::getStateBuy() {
    return stateBuy;
}

// returns selling (at market) state
StateVendorSell* VendorStateManager::getStateSell() {
    return stateSell;
}

// return at home state (WS)
/*
StateWSAtHome* VendorStateManager::getStateWSHome() {
    return stateWSHome;
}
 

// return buying state (WS)
StateWSBuy* VendorStateManager::getStateWSBuy() {
    return stateWSBuy;
}

// return wholesale selling state (WS)
StateWSWholesaleSell* VendorStateManager::getStateWSWholesaleSell() {
    return stateWSWholesaleSell;
}

// return retail selling state (WS)
StateWSRetailSell* VendorStateManager::getStateWSRetailSell() {
    return stateWSRetailSell;
}
 */
