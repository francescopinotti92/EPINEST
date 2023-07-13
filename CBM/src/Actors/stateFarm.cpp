//
//  stateFarm.cpp
//  CBM
//
//

#include "node.hpp"


// handle requests
void StateFarmEmpty::onRefillRequest( NodeFarm* nf, const Clock& clock, Random& randGen, ChickenPool* birdMngr ) {
        
    if ( nf->getDayNextCycleBegin() == clock.getCurrDay() ) {
        nf->addCrops( clock, randGen, birdMngr );
        nf->setState( stateRaising );
    }
    
}
void StateFarmEmpty::onStageCropRequest( NodeFarm *nf, const Clock &clock, Random &randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr )  {};
//void StateFarmEmpty::onSellRequest( NodeFarm *nf, const Clock &clock, Random &randGen )       {};
uint StateFarmEmpty::onEvalBatchSizeRequest( NodeFarm* nf, const BirdType& bt, const uint& amount ) {
    return 0;
}

void StateFarmEmpty::onScheduleTodayTradeRequest( NodeFarm* nf, const Clock& clock, Random& randGen,  BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr ) {};
void StateFarmEmpty::onIsEmptyRequest( NodeFarm* nf, AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr )                  {};
bool StateFarmEmpty::onHasEndedRolloutRequest( NodeFarm* nf ) { return false; }
void StateFarmEmpty::onResetRequest( NodeFarm* nf, ChickenPool* birdMngr ) {
    nf->reset( birdMngr );
}
void StateFarmRaising::onRefillRequest( NodeFarm* nf, const Clock& clock, Random& randGen, ChickenPool* birdMngr ) {};
void StateFarmRaising::onStageCropRequest( NodeFarm *nf, const Clock &clock, Random &randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr ) {
    
    // check if farm is empty for any reason (e.g. birds dying of disease)
    if ( nf->isEmpty() ) {
        nf->getContext()->clearCropList( clock, cropMngr ); // clear empty crop
        nf->scheduleNextCycle( clock, randGen );
        nf->setState( stateEmpty );
        return;
    }
    
    // check if crop is ready for sale
    nf->getContext()->moveReadyCrops( clock, cropMngr );
    bool hasReadyCrops = nf->getContext()->hasReadyCrops();
    if ( hasReadyCrops ) {
        areaMngr.registerTradingFarm( nf );
        nf->setState( stateTrading );
        nf->setRolloutBeginTime( clock.getCurrDay() );
    }
}
//void StateFarmRaising::onSellRequest( NodeFarm *nf, const Clock &clock, Random &randGen )         {};
uint StateFarmRaising::onEvalBatchSizeRequest( NodeFarm* nf, const BirdType& bt, const uint& amount ) {
    return 0;
}
void StateFarmRaising::onScheduleTodayTradeRequest( NodeFarm* nf, const Clock& clock, Random& randGen, BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr ) {};
void StateFarmRaising::onIsEmptyRequest( NodeFarm *nf, AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr ) {};
bool StateFarmRaising::onHasEndedRolloutRequest( NodeFarm* nf ) { return false; }
void StateFarmRaising::onResetRequest( NodeFarm* nf, ChickenPool* birdMngr ) {
    nf->reset( birdMngr );
    nf->setState( stateEmpty );
}

void StateFarmTrading::onRefillRequest( NodeFarm* nf, const Clock& clock, Random& randGen, ChickenPool* birdMngr ) {};
void StateFarmTrading::onStageCropRequest( NodeFarm *nf, const Clock &clock, Random &randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr ) {};
/*
void StateFarmTrading::onSellRequest( NodeFarm *nf, const Clock &clock, Random &randGen ) {
    // sell birds
    nf->sellBirds2Middlemen( clock, randGen );
}*/
uint StateFarmTrading::onEvalBatchSizeRequest( NodeFarm* nf, const BirdType& bt, const uint& amount ) {
    return nf->evalBatchSizeFromMman( bt, amount );
}

void StateFarmTrading::onScheduleTodayTradeRequest( NodeFarm* nf, const Clock& clock, Random& randGen, BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr ) {
    nf->scheduleTodayTrade( clock, randGen, transferMngr, cropMngr );
};


void StateFarmTrading::onIsEmptyRequest( NodeFarm *nf, AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr ) {
    // check if farm is empty
    if ( nf->isEmpty() ) {
        areaMngr.unregisterTradingFarm( nf );
        nf->getContext()->clearCropList( clock, cropMngr ); // clear empty crop
        nf->resetTotalAvailBirdsToday();
        nf->setState( stateEmpty );
        nf->scheduleNextCycle( clock, randGen );
    }
}

bool StateFarmTrading::onHasEndedRolloutRequest( NodeFarm* nf ) {
    return ( nf->isEmpty() ) ? true : false;
}

void StateFarmTrading::onResetRequest( NodeFarm* nf, ChickenPool* birdMngr ) {
    nf->reset( birdMngr );
    nf->setState( stateEmpty );
}

void StateFarmEmpty::setStateRaising(StateFarmRaising* stateRaising_) {
    stateRaising = stateRaising_;
}

void StateFarmRaising::setStateEmpty(StateFarmEmpty* stateEmpty_) {
    stateEmpty = stateEmpty_;
}

void StateFarmRaising::setStateTrading(StateFarmTrading* stateTrading_) {
    
    stateTrading = stateTrading_;
}

void StateFarmTrading::setStateEmpty(StateFarmEmpty* stateEmpty_) {
    stateEmpty = stateEmpty_;
}

// constructor
FarmStateManager::FarmStateManager() {
    stateEmpty = new StateFarmEmpty();
    stateRaising = new StateFarmRaising();
    stateTrading = new StateFarmTrading();
    
    stateEmpty->setStateRaising( stateRaising );
    stateRaising->setStateEmpty( stateEmpty );
    stateRaising->setStateTrading( stateTrading );
    stateTrading->setStateEmpty( stateEmpty );
}

// destructor
FarmStateManager::~FarmStateManager() {
    stateEmpty->setStateRaising(nullptr);
    stateRaising->setStateEmpty(nullptr);
    stateRaising->setStateTrading(nullptr);
    stateTrading->setStateEmpty(nullptr);
    
    delete stateEmpty;
    delete stateRaising;
    delete stateTrading;
}

// returns open state
StateFarmEmpty* FarmStateManager::getStateEmpty() {
    return stateEmpty;
}
