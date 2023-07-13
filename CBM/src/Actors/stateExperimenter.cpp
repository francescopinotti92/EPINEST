//
//  stateExperimenter.cpp
//  CBM
//
//

#include "node.hpp"


//====== FieldExperimenter states ======//

void StateExperimenterRecruiting::onGoToNextStageRequest( NodeFieldExperimenter* nfe, Clock& clock, Random& randGen, BirdTransferManager &transferManager ) {
    
    // recruit birds from a random farm
    if ( clock.getCurrTime() == nfe->getTimeFarmRecruit() ) {
        
        nfe->recruitBirdsFarms( clock, randGen, transferManager ) ;
        nfe->setState( stateSheltering ) ;
        
    }
    
}

void StateExperimenterRecruiting::onResetRequest( NodeFieldExperimenter* nfe, ChickenPool* birdMngr ) {
    
    nfe->reset( birdMngr ) ;
    
}


void StateExperimenterRecruiting::setStateSheltering( StateExperimenterSheltering* stateSheltering ) {
    
    stateSheltering = stateSheltering ;
    
}


//~~~~~~~~~~~~~~~~//


void StateExperimenterSheltering::onGoToNextStageRequest( NodeFieldExperimenter* nfe, Clock& clock, Random& randGen, BirdTransferManager &transferManager ) {
    
    // go to market
    if ( clock.getCurrTime() == nfe->getTimeStartExperiment() ) {
        
        // Go to market (first time)
        nfe->goToMarket( randGen, transferManager ) ;
        nfe->recruitBirdsMarket( randGen, transferManager ) ;
        // Buy chickens from a random vendor
        nfe->setState( stateMarket ) ;
        
    }
    
}

void StateExperimenterSheltering::onResetRequest( NodeFieldExperimenter* nfe, ChickenPool* birdMngr ) {
    
    nfe->reset( birdMngr ) ;
    nfe->setState( stateRecruiting ) ;

    
}

void StateExperimenterSheltering::setStateMarket( StateExperimenterMarketStage* stateMarket ) {
    
    stateMarket = stateMarket ;
    
}

void StateExperimenterSheltering::setStateRecruiting( StateExperimenterRecruiting* stateRecruiting ) {
    
    stateRecruiting = stateRecruiting ;
    
}




//~~~~~~~~~~~~~~~~//


void StateExperimenterMarketStage::onGoToNextStageRequest( NodeFieldExperimenter* nfe, Clock& clock, Random& randGen, BirdTransferManager &transferManager ) {
        
    if ( clock.getCurrTime() == nfe->getTimeStopExperiment() ) {
        
        // !!! store experiment info
        // !!! get rid of chickens
        nfe->setState( stateRecruiting ) ; // !!! sure?
        
    }
    
}

void StateExperimenterMarketStage::onResetRequest( NodeFieldExperimenter* nfe, ChickenPool* birdMngr ) {
    
    nfe->reset( birdMngr ) ;
    
}



void StateExperimenterMarketStage::setStateRecruiting( StateExperimenterRecruiting* stateRecruiting ) {
    
    stateRecruiting = stateRecruiting ;
    
}


//====== FieldExperimenterStateManager ======//


FieldExperimenterStateManager::FieldExperimenterStateManager() {
    
    stateRecruiting = new StateExperimenterRecruiting() ;
    stateSheltering = new StateExperimenterSheltering() ;
    stateMarket     = new StateExperimenterMarketStage() ;
    
    stateRecruiting->setStateSheltering( stateSheltering ) ;
    stateSheltering->setStateMarket( stateMarket ) ;
    stateSheltering->setStateRecruiting( stateRecruiting ) ;
    stateMarket->setStateRecruiting( stateRecruiting ) ;
    
}

FieldExperimenterStateManager::~FieldExperimenterStateManager() {
    
    stateRecruiting->setStateSheltering( nullptr ) ;
    stateSheltering->setStateMarket( nullptr ) ;
    stateSheltering->setStateRecruiting( nullptr ) ;
    stateMarket->setStateRecruiting( nullptr ) ;
    
    delete stateRecruiting ;
    delete stateSheltering ;
    delete stateMarket ;
    
}


StateExperimenterRecruiting* FieldExperimenterStateManager::getStateRecruiting() {
    return stateRecruiting ;
}
StateExperimenterSheltering* FieldExperimenterStateManager::getStateSheltering() {
    return stateSheltering ;
}
StateExperimenterMarketStage* FieldExperimenterStateManager::getStateMarket() {
    return stateMarket ;
}
