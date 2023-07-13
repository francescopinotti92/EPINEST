//
//  stateMiddleman.cpp
//  CBM
//
//

#include <stdio.h>

#include "node.hpp"

//=== StateMmanAwaiting ===//

/*
uint StateMmanAwaiting::onProcessBatchRequest( NodeMman* nmm, NodeFarm* nf, const BirdType& bt, const uint& nbirds ) {
    return nmm->acceptedBatchSize( nf, bt, nbirds );
}*/

void StateMmanAwaiting::onMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {
    
    if ( nmm->decideToMoveToNewAreas( randGen ) )
        nmm->moveToNewAreas( randGen, areaMngr );
}

void StateMmanAwaiting::onForceMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {
    nmm->moveToNewAreas( randGen, areaMngr );
}

void StateMmanAwaiting::onBuyFromFarmsRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {
    nmm->buyFromFarms( randGen, areaMngr );
}

void StateMmanAwaiting::onExecuteScheduleTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen ) {

    if ( nmm->getNtasks() > 0 ) {
        nmm->scheduleTasks( clock, randGen );     // schedule task timing
        nmm->setState( stateCollect );            // nmm becomes a collector
    }
    else {
        // go selling if birds allowed to be sold
        if ( nmm->getSizeBirds() > 0 )
            nmm->setState( stateSell );
    }
    
};

void StateMmanAwaiting::onExecuteCollectionTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {};

void StateMmanAwaiting::onSellToMarketsRequest( NodeMman* nmm, Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) {};

void StateMmanAwaiting::onResetRequest( NodeMman *nmm, ChickenPool* birdMngr ) {
    nmm->reset( birdMngr );
}

void StateMmanAwaiting::setStateCollect( StateMmanCollecting* stateCollect_ ) {
    stateCollect = stateCollect_;
}

void StateMmanAwaiting::setStateSell( StateMmanSelling* stateSell_ ) {
    stateSell = stateSell_;
}

//=== StateMmanCollecting ===//

/*
uint StateMmanCollecting::onProcessBatchRequest( NodeMman* nmm, NodeFarm* nf, const BirdType& bt, const uint& nbirds ) {
    return 0;
}*/

void StateMmanCollecting::onMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {
    if ( nmm->decideToMoveToNewAreas( randGen ) )
        nmm->moveToNewAreas( randGen, areaMngr );
}
void StateMmanCollecting::onForceMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {
    nmm->moveToNewAreas( randGen, areaMngr );
}
void StateMmanCollecting::onBuyFromFarmsRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {};
void StateMmanCollecting::onExecuteScheduleTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen ) {};

void StateMmanCollecting::onExecuteCollectionTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {
        
    nmm->processCurrentCollectionTasks( clock, randGen, transferManager );
    if ( nmm->hasCompletedCollectionTasks() ) {
        nmm->clearCollectionTasks();
        nmm->setState( stateSell );
    }
}

void StateMmanCollecting::onSellToMarketsRequest( NodeMman* nmm, Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) {};

void StateMmanCollecting::onResetRequest( NodeMman *nmm, ChickenPool* birdMngr ) {
    nmm->reset( birdMngr );
    nmm->setState( stateAwait );
}

void StateMmanCollecting::setStateAwait( StateMmanAwaiting* stateAwait ) {
    this->stateAwait = stateAwait;
}

void StateMmanCollecting::setStateSell( StateMmanSelling* stateSell ) {
    this->stateSell = stateSell;
}

//=== StateMmanSelling ===//

/*
uint StateMmanSelling::onProcessBatchRequest( NodeMman* nmm, NodeFarm* nf, const BirdType& bt, const uint& nbirds ) {
    return 0;
}*/

void StateMmanSelling::onMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {
    if ( nmm->decideToMoveToNewAreas( randGen ) )
        nmm->moveToNewAreas( randGen, areaMngr );
}
void StateMmanSelling::onForceMoveToNewAreasRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {
    nmm->moveToNewAreas( randGen, areaMngr );
}
void StateMmanSelling::onBuyFromFarmsRequest( NodeMman* nmm, Random& randGen, AreaManager& areaMngr ) {};
void StateMmanSelling::onExecuteScheduleTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen ) {};
void StateMmanSelling::onExecuteCollectionTasksRequest( NodeMman* nmm, const Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {};
void StateMmanSelling::onSellToMarketsRequest( NodeMman* nmm, Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) {
    
    nmm->sellToMarkets( randGen, transferManager, marketList );
    
    nmm->setState( stateAwait );
};

void StateMmanSelling::onResetRequest( NodeMman *nmm, ChickenPool* birdMngr ) {
    nmm->reset( birdMngr );
    nmm->setState( stateAwait );
}

void StateMmanSelling::setStateAwait( StateMmanAwaiting* stateAwait_ ) {
    stateAwait = stateAwait_;
}

//=== MiddlemanStateManager ===//

MiddlemanStateManager::MiddlemanStateManager() {
    stateAwaiting = new StateMmanAwaiting();
    stateCollecting = new StateMmanCollecting();
    stateSelling = new StateMmanSelling();
    
    stateAwaiting->setStateCollect( stateCollecting );
    stateAwaiting->setStateSell( stateSelling );
    stateCollecting->setStateAwait( stateAwaiting );
    stateCollecting->setStateSell( stateSelling );
    stateSelling->setStateAwait( stateAwaiting );
}

MiddlemanStateManager::~MiddlemanStateManager() {
    stateAwaiting->setStateCollect( nullptr );
    stateAwaiting->setStateSell( nullptr );
    stateCollecting->setStateAwait( nullptr );
    stateCollecting->setStateSell( nullptr );
    stateSelling->setStateAwait( nullptr );
    
    delete stateAwaiting;
    delete stateCollecting;
    delete stateSelling;
}

StateMmanAwaiting* MiddlemanStateManager::getStateAwaiting() {
    return stateAwaiting;
}
