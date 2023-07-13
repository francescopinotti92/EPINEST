//
//  nodeFarm.cpp
//  CBM
//
//

#include "node.hpp"

template class weighted_set<NodeFarm>;

// constructor
NodeFarm::NodeFarm( Farm* farm_, StateFarm* state_, const Clock& clock ): farm( farm_ ), state( state_ ) {
    
    auto& bts = farm->getTradedBirdTypes();
    for (auto& bt: bts) {
        totalAvailBirdsToday[bt] = 0;
    }
    averageFarmSize = 0.;
    
    areaId = 0;
    dayRolloutBegin   = 0;
    nBirdsDiscarded   = 0;
    dayNextCycleBegin = clock.getCurrDay();
}

// constructor (with position)
NodeFarm::NodeFarm( Farm* farm_, StateFarm* state_, uint area, Geom::Point pos, const Clock& clock ): farm( farm_ ), state( state_ ) {
    
    setPos( pos );
    
    auto& bts = farm->getTradedBirdTypes();
    for ( auto& bt: bts ) {
        //mmanPartners.insert( std::make_pair( bt, edge_manager<NodeMman>() ) );
        totalAvailBirdsToday[bt] = 0;
    }
    averageFarmSize = 0.;

    areaId = area;
    dayRolloutBegin   = 0;
    nBirdsDiscarded   = 0;
    dayNextCycleBegin = clock.getCurrDay();

}

void NodeFarm::reset( ChickenPool* birdMngr ) {
    // reset NodeFarm attributes
    dayRolloutBegin   = 0;
    nBirdsDiscarded   = 0;
    dayNextCycleBegin = 0;
    auto& bts = farm->getTradedBirdTypes();
    for ( auto& bt: bts ) {
        totalAvailBirdsToday[bt] = 0;
    }
    
    // reset inner farm attributes
    farm->reset( birdMngr );
    
}

// handle external crop refill requests
void NodeFarm::refillRequest( const Clock& clock, Random& randGen, ChickenPool* birdMngr  ) {
    state->onRefillRequest( this, clock, randGen, birdMngr );
}

// handle external stage crop requests
void NodeFarm::stageCropRequest( const Clock& clock, Random& randGen, AreaManager& areaMngr, CompletedCropTrackerManager* cropMngr ) {
    state->onStageCropRequest( this, clock, randGen, areaMngr, cropMngr );
}

uint NodeFarm::evalBatchSizeRequest( const BirdType& bt, const uint &amount ) {
    return state->onEvalBatchSizeRequest( this, bt, amount );
}

void NodeFarm::scheduleTodayTradeRequest( const Clock& clock, Random& randGen, BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr ) {
    
    state->onScheduleTodayTradeRequest( this, clock, randGen, transferMngr, cropMngr );
    
}

void NodeFarm::isEmptyRequest( AreaManager& areaMngr, const Clock& clock, Random& randGen, CompletedCropTrackerManager* cropMngr ) {
    state->onIsEmptyRequest( this, areaMngr, clock, randGen, cropMngr );
}

// returns 'true' if farm was selling birds but has ended rollout
// similar as method above, but does not update a farm's state
bool NodeFarm::hasEndedRolloutRequest() {
    return state->onHasEndedRolloutRequest( this );
}

// handle external reset request
void NodeFarm::resetRequest( ChickenPool* birdMngr ) {
    
    state->onResetRequest( this, birdMngr );
    
}

bool NodeFarm::isEmpty() {
    return ( ( farm->getSizeBirds() == 0 ) ? true : false );
}


/*
void NodeFarm::addNbr( NodeMman *nmm, const BirdType& bt, const double& weight ) {
    mmanPartners[bt].addNbr( nmm, weight );
}*/

// adds a farm to list of neighboring farms, together with its
// distance-dependent spatial transmission kernel weight K(d_ij)
/*
void NodeFarm::addNbrKernel(NodeFarm* nbrFarm, const double& weight) {
    farmKernelDistanceWeightsVec.addNbr( nbrFarm, weight );
}
 */

void NodeFarm::increaseTotalAvailBirdsToday( const BirdType& bt, const uint& amount ) {
    totalAvailBirdsToday[bt] += amount;
}

void NodeFarm::reduceTotalAvailBirdsToday( const BirdType& bt, const uint& amount ) {
    totalAvailBirdsToday[bt] -= amount;
}

void NodeFarm::resetTotalAvailBirdsToday() {
    for ( auto& el: totalAvailBirdsToday ) {
        el.second = 0;
    }
}

// returns 'true' if there is at least a single infected bird
// and false otherwise
bool NodeFarm::hasInfectedBirds() {
    return ( getSizeInfectedBirds() > 0 ) ? true : false;
}

// configures farm as a commercial farm, with geometric time to next cycle


void NodeFarm::configCommercialFarm(const BirdType &bt, const uint &batchSize, const double& p_refill, const uint& ndays_rollout) {
    
    this->setAverageFarmSize( batchSize );
    
    this->scheduleNextCycle = std::bind( &NodeFarm::scheduleNextCycleGeom1, this, p_refill, std::placeholders::_1, std::placeholders::_2 );
    
    this->addCrops = std::bind( &NodeFarm::addSingleCrop, this, bt, batchSize, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
    
    this->scheduleTodayTrade = std::bind( &NodeFarm::scheduleSingleBatchRolloutDaily, this, 1./ndays_rollout, ndays_rollout + 8, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4 ); // rollout ends automatically after 8 days post 'ndays_rollout'
    
    this->performsRolloutTrade = true;

}
 
// configures farm as a commercial farm, with negative binomial time to next cycle


void NodeFarm::configCommercialFarm( const BirdType &bt, const uint &batchSize, const double& pNB, const double& nNB, const uint& ndays_rollout ) {
        
    this->setAverageFarmSize( batchSize );

    this->scheduleNextCycle = std::bind( &NodeFarm::scheduleNextCycleNegBinom1, this, pNB, nNB, std::placeholders::_1, std::placeholders::_2 );
    
    this->addCrops = std::bind( &NodeFarm::addSingleCrop, this, bt, batchSize, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
    
    this->scheduleTodayTrade = std::bind( &NodeFarm::scheduleSingleBatchRolloutDaily, this, 1./ndays_rollout, ndays_rollout + 8, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4 ); // rollout ends automatically after 8 days post 'ndays_rollout'

    
    this->performsRolloutTrade = true;

}


// gets 0 or 1 crops according to fixed refill probability
uint NodeFarm::drawSingleCropFixedProb(const double &p_refill, Random &randGen) {
    return ( randGen.getBool( p_refill ) ) ? 1 : 0;
}

// adds a single crop of birds to farm
bool NodeFarm::addSingleCrop( const BirdType& bt, const uint& batchSize, const Clock& clock, Random& randGen, ChickenPool* birdMngr ) {

    if ( batchSize > farm->getBirds().available_space() )
        return false; // not enough space to add crop
    else {
        farm->addCrop( bt, batchSize, clock, randGen, birdMngr );
        return true;
    }
    return false;
}

// this function is meant for commercial farms only
uint NodeFarm::evalBatchSizeFromMman( const BirdType& bt, const uint& amount ) {
    
    uint nBirdsSold = std::min( amount, this->totalAvailBirdsToday[bt] );
    
    this->totalAvailBirdsToday[bt] -= nBirdsSold;
    
    return nBirdsSold;
}

// checks if there are birds made available to vendors
bool NodeFarm::hasAvailableBirds( const BirdType &bt ) {
    return ( this->totalAvailBirdsToday[bt] > 0 ) ? true : false;
}

// removes all birds from single crop forcefully
/*
void NodeFarm::removeBirdsFromCrop( Crop &crop, ChickenPool* birdMngr ) {
    while( !crop.empty() ) {
        uint idx = crop.removeLast( false, true );
        pBird bird = farm->removeBird( idx );
        birdMngr->return_bird( std::move( bird ) );
    }
}*/

// This function is meant for commercial farms only
void NodeFarm::scheduleSingleBatchRolloutDaily( const double& p_sold_daily, const uint& maxDaysSinceRollout, const Clock& clock, Random& randGen, BirdTransferManager* transferMngr, CompletedCropTrackerManager* cropMngr ) {
    
    const BirdType& bt = getContext()->getTradedBirdTypes()[0];
    
    if ( farm->hasReadyCrops() ) {
        
        Crop* crop = getContext()->getReadyCrops( bt )[0]; // get the single crop
        
        // compute (corrected) trade probability
        double daysSinceRollout = clock.getCurrDay() - dayRolloutBegin;
        
        // remove birds if rollout takes too long
        if ( daysSinceRollout > maxDaysSinceRollout ) {
            nBirdsDiscarded += crop->size();    // increase amount of birds discarded
            //removeBirdsFromCrop( *crop, birdMngr );       // discard birds
            transferMngr->transferBirdsFarm2Nothing( this, *crop ) ;
            cropMngr->processCompletedCrop( *this, crop, clock ); // call tracker
            delete crop;
            getContext()->getReadyCrops( bt ).clear();
            return;
        }

        if ( daysSinceRollout * p_sold_daily < 1. ) {
            // place additional birds on sale
            // accounts for any unsold birds from previous day
            double prop_sale_t = p_sold_daily / ( 1. -  p_sold_daily * daysSinceRollout );
            uint unsoldBirdsSize = totalAvailBirdsToday[bt];            // birds unsold in previous days
            uint cropSize = crop->size();                               // birds in the current batch
            
            uint birdsYetToStage = cropSize - unsoldBirdsSize; // birds yet to be checked for trade
            totalAvailBirdsToday[bt] += randGen.getBinom( prop_sale_t, birdsYetToStage );
            
        }
        else { // after too many days, try to clear entire batch
            // sell all birds
            totalAvailBirdsToday[bt] = crop->size();
        }
    }

}


void NodeFarm::scheduleNextCycleGeom1( const double& p_refill, const Clock& clock, Random& randGen ) {
    
    uint daysToNextCycle = randGen.getGeom1( p_refill );
    this->setNextCycleBeginTime( clock.getCurrDay() + daysToNextCycle );
    
}
void NodeFarm::scheduleNextCycleNegBinom1( const double& p, const double& n, const Clock& clock, Random& randGen ) {
    
    uint daysToNextCycle = 1 + randGen.getNegBinom( p, n ); // just shift by 1
    this->setNextCycleBeginTime( clock.getCurrDay() + daysToNextCycle );
    
    
}
