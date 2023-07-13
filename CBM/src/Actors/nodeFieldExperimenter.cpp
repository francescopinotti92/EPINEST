//
//  nodeFieldExperimenter.cpp
//  CBM
//
//

#include "node.hpp"


NodeFieldExperimenter::NodeFieldExperimenter( StateExperimenter* state_, BirdType birdType_, uint nRecruitsFarm_, uint nRecruitsMarket_, uint dtShelter_, uint dayExperiment_, uint dtExperiment_, NodeMarket* marketExperiment_, std::vector<NodeFarm*>* availableFarms_ ): birdType( birdType_ ), nRecruitsFarm( nRecruitsFarm_ ), nRecruitsMarket( nRecruitsMarket_ ), dtShelter( dtShelter_ ), dayExperiment( dayExperiment_ ), dtExperiment( dtExperiment_ ), marketExperiment( marketExperiment_ ), availableFarms( availableFarms_ ) {
    
    fieldExperimenter = new FieldExperimenter( birdType, nRecruitsFarm + nRecruitsMarket ) ;
    setState( state_ ) ;
    
    firstInvasionCheckPoint = {} ;

    
    //=== Schedule experiment time points

    assert( dtShelter < dayExperiment * 24 + MARKET_OPENING_HOUR ) ;
    assert( marketExperiment != nullptr ) ;

    tStartExperiment    = dayExperiment * 24 + MARKET_OPENING_HOUR ;
    tRecruitFarm        = tStartExperiment - dtShelter ;
    tStopExperiment     = tStartExperiment + dtExperiment ;
    
    
}

void NodeFieldExperimenter::goToNextStageExperimentRequest( Clock& clock, Random& randGen, BirdTransferManager& transferManager ) {
    
    state->onGoToNextStageRequest( this, clock, randGen, transferManager ) ;
    
}

void NodeFieldExperimenter::resetRequest( ChickenPool* birdMngr ) {
    
    state->onResetRequest( this, birdMngr ) ;
    
}

void NodeFieldExperimenter::goToMarket( Random& randGen, BirdTransferManager& transferManager ) {
    
    // move birds to market
    
    Market* market = marketExperiment->getContext() ;
    auto& birds = fieldExperimenter->getBirds() ;
    std::unordered_set<uint>& birdIdxSet = fieldExperimenter->getBirdsMarketIndex() ;
    
    if ( market == nullptr )
        return ;
    
    if ( birds.empty() )
        return ;
    
    assert( birdIdxSet.empty() ) ;
    
    uint idx;
    while ( birds.size() > 0 ) { // push all birds to market, one by one
        
        pBird birdTmp = birds.removeLast() ;
        birdTmp->movements.increaseRepurposeCount() ;
        idx = ( market->insertBird( std::move( birdTmp ) ) ).index ; // get bird index back
        birdIdxSet.insert( idx ) ; // keep a list of birds at market
        
    }

}

void NodeFieldExperimenter::goHome() {
    
    // pull out birds from market using index
    
    assert( marketExperiment != nullptr ) ;
    
    Market* market = marketExperiment->getContext() ;
    
    {
        // return newly bought birds
        std::unordered_set<uint>& birdIdxSet = fieldExperimenter->getBirdsMarketIndex() ;
        //std::vector<uint>& birdIdxVec = vendor->getMarketedNewBirdsIndexes();
        for ( uint birdIdx: birdIdxSet ) {
            
            pBird bird = market->removeBird( birdIdx ) ;
            fieldExperimenter->insertBird( std::move( bird ) ) ;
            
        }
        birdIdxSet.clear() ; // clear index set
    }
    
}



void NodeFieldExperimenter::recruitBirdsFarms(  Clock& clock, Random& randGen, BirdTransferManager& transferManager) {
    
    //== select a farm that accepts to sell enough birds
    
    uint nAttempts = 0 ;
    uint nFarms = static_cast<uint>( availableFarms->size() ) ;
    
    while ( true ) {
        
        uint ixFarm = randGen.getUniInt( nFarms  - 1 ) ;
        NodeFarm* nf = availableFarms->at( ixFarm ) ;
        uint nAccept = nf->evalBatchSizeRequest( birdType, nRecruitsFarm ) ;
        
        if ( nAccept == nRecruitsFarm ) {
        
            transferManager.transferBirdsFarm2FieldExperimenter( nf, this, birdType, nRecruitsFarm, randGen ) ;
            break ; // done
            
        }
        else {
            
            nAttempts++ ;
            if ( nAttempts >= 100 * nFarms ) ;
                throw Warning( "Experimenter here: cannot find farms to purchase chickens from." ) ;
            
        }
        
    }
    
}


void NodeFieldExperimenter::recruitBirdsMarket( Random& randGen, BirdTransferManager& transferManager ) {

    // check market is open
    if ( !marketExperiment->isOpen() )
        return ;
    
    // pick a random vendor
    std::vector<NodeVendor*>& availableSellers = marketExperiment->getSellers( birdType ) ;
    
    while ( true ) {
        
        uint ixVendor = randGen.getUniInt( static_cast<uint>( availableSellers.size() )  - 1 ) ;
        NodeVendor* nv = availableSellers.at( ixVendor ) ;
        
        // ask nRecruitsMarket birds to purchase
        uint nBirdsAccept = nv->evalRetailPurchaseRequest( nRecruitsMarket ) ;
        if ( nBirdsAccept == nRecruitsMarket ) {
            
            // move ownership of these chickens
            transferManager.transferBirdsVendor2FieldExperimenter( nv, this, marketExperiment, birdType, nRecruitsMarket, randGen ) ;
            nv->decreaseBirdRetailCount( nRecruitsMarket ) ;
            break ; // done
            
        }

    }
    
    
}

void NodeFieldExperimenter::reset( ChickenPool* birdMngr ) {
    
    // reset properties
    firstInvasionCheckPoint.clear() ;
    
    // reset underlying fieldExperimenter
    fieldExperimenter->reset( birdMngr ) ;
    
}

void NodeFieldExperimenter::checkInfection( Clock& clock ) {
    
    uint t = clock.getCurrTime() ;
    auto& birdIdxs = fieldExperimenter->getBirdsMarketIndex() ;
    auto& birdsMarket = marketExperiment->getContext()->getBirds() ;
    
    for ( uint idx : birdIdxs ) {
        
        pBird& bird = birdsMarket.getBirdSparse( idx ) ; // ??? correct?
        
        if ( bird->isInfectious() ) { // bird is found to be infectious (hence tests positive)
            
            // !!! check if infection event is already registered
            BirdID birdID = bird->getID() ;
            if ( firstInvasionCheckPoint.count( birdID ) == 0 ) { // bird infection has not been recorded yet (hence first positive test)
                
                // !!! assign check point index
                //firstInvasionCheckPoint[birdID] = 0 ;
                
                // !!! sample viral lineage (if enabled)

            }
            
        }
        
        
    }
    
    
    
}

