//
//  stateMarket.cpp
//  CBM
//
//

#include "node.hpp"


//====== Market states ======//

//====== Open state ======//

// constructor
StateMarketOpen::StateMarketOpen(): stateClose(nullptr) {};

// does nothing: market is already open
void StateMarketOpen::open(NodeMarket& nma, const Clock& clock) {};

// close the market
void StateMarketOpen::close(NodeMarket& nma, const Clock& clock) {
    // make vendors go home
    nma.closeMarket();
    nma.setState( stateClose );
};

void StateMarketOpen::onDoTradingStepRequest( NodeMarket* nm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {
    nm->doTradingStep( clock, randGen, transferManager );
}

void StateMarketOpen::onResetRequest( NodeMarket* nm, ChickenPool* birdMngr ) {
    nm->reset( birdMngr );
    nm->setState( stateClose );
}

void StateMarketOpen::setStateClose(StateMarketClose* stateClose_) {
    stateClose = stateClose_;
}


//====== Close state ======//

StateMarketClose::StateMarketClose(): stateOpen(nullptr) {};
void StateMarketClose::open(NodeMarket& nma, const Clock& clock) {
    if ( !nma.isWorkingDay(clock) )
        return;
    nma.setState(stateOpen);
};

void StateMarketClose::close( NodeMarket& nma, const Clock& clock ) {};

void StateMarketClose::onDoTradingStepRequest( NodeMarket* nm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {};

void StateMarketClose::onResetRequest( NodeMarket* nm, ChickenPool* birdMngr ) {
    nm->reset( birdMngr );
}

void StateMarketClose::setStateOpen( StateMarketOpen* stateOpen_ ) {
    stateOpen = stateOpen_;
}


//====== Market State Manager ======//

// constructor
MarketStateManager::MarketStateManager() {
    stateOpen = new StateMarketOpen();
    stateClose = new StateMarketClose();
    
    stateOpen->setStateClose(stateClose);
    stateClose->setStateOpen(stateOpen);
}

// destructor
MarketStateManager::~MarketStateManager() {
    stateOpen->setStateClose(nullptr);
    stateClose->setStateOpen(nullptr);
    
    delete stateOpen;
    delete stateClose;
}

// returns open state
StateMarketOpen* MarketStateManager::getStateOpen() {
    return stateOpen;
}

// returns close state
StateMarketClose* MarketStateManager::getStateClose() {
    return stateClose;
}
