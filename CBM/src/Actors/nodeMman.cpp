//
//  nodeMman.cpp
//  CBM
//
//

#include "node.hpp"

bool compareCollectionTaskTiming(const CollectionTask &left, const CollectionTask &right) {
    return left.t < right.t;
}
bool compareCollectionTaskTimingPtr( const CollectionTask* left, const CollectionTask* right ) {
    return left->t < right->t;
}

//=== constructor ===//
NodeMman::NodeMman( Middleman* mman_, StateMman* state_ ): mman( mman_ ), state( state_ ) {

    orderTaskList.clear();
    collectionTaskVec.reserve( 5 );
    currCollectionTask = 0 ;
    nMarketsToVisitDaily = 0 ;
    
    nIncomingInfectedBirds = 0 ;
    trackersFarmTradedBird = {} ;

    /*
    auto& bts = mman->getTradedBirdTypes();
    for (auto& bt: bts) {
        marketNbrs.insert( { bt, edge_manager<NodeMarket>() } );
    }
     */
    
}

// configures a "simple" middleman
// illustrates how to compose a NodeMman
void NodeMman::configSimpleMiddleman( const uint& nVisitedMarkets, const uint& nVisitedAreas, const double& probChangeArea ) {
    // configure purchase evaluation routine
    // "simple" middleman is concerned with capacity only
    
    //this->acceptedBatchSize = std::bind( &NodeMman::evalBatchSizeCapacity, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );
    
    nMarketsToVisitDaily = nVisitedMarkets ; // number of markets to visit daily
    
    // configure movement routines
    this->decideToMoveToNewAreas = std::bind( &NodeMman::decideToMoveFixedProb, this, probChangeArea, std::placeholders::_1 );
    
    this->moveToNewAreas = std::bind( &NodeMman::moveToNewAreasNeighboring, this, nVisitedAreas, std::placeholders::_1, std::placeholders::_2 );
    
    // configure purchasing routine
    // "simple" middleman contacts farms according to volume in AreaManager
    this->buyFromFarms = std::bind( &NodeMman::buyFromFarmsSingleSpecies, this, nVisitedMarkets, std::placeholders::_1, std::placeholders::_2 );
    
    // configure task scheduling routine
    // "simple" middleman uses full time range to distribute task execution times
    this->scheduleTasks = std::bind( &NodeMman::scheduleCollectionTasksRandom, this, std::placeholders::_1, std::placeholders::_2 );
    
    // configure selling routine
    // "simple" middleman sells the entire cargo to a fixed number (nVisitedMarkets) of markets
    //this->sellToMarkets = std::bind( &NodeMman::sellFullCargoToMarkets, this, nVisitedMarkets, std::placeholders::_1, std::placeholders::_2 );
    this->sellToMarkets = std::bind( &NodeMman::sellFullCargoToMarkets, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 );

}

void NodeMman::reset( ChickenPool* birdMngr ) {
    
    // reset NodeMman attributes
    currCollectionTask = 0;
    nIncomingInfectedBirds = 0 ;
    
    clearOrders();
    
    for ( auto& task: collectionTaskVec )
        delete task;
    collectionTaskVec.clear();
    
    // reset inner mman
    mman->reset( birdMngr );
}

//=== other functions ===//

/*
void NodeMman::addNbr(NodeMarket* nm, const BirdType& bt, const double& weight) {
    marketNbrs[bt].addNbr( nm, weight );
}*/

// creates and enqueues a new collection task
void NodeMman::addCollectionTask( NodeFarm* nf, const uint& nmId, const BirdType& bt, const uint& nbirds ) {
    collectionTaskVec.emplace_back( new CollectionTask( nf, nmId, nbirds, bt ) );
}

void NodeMman::scheduleCollectionTasksRandom( const Clock &clock, Random &randGen ) {

    nIncomingInfectedBirds = 0 ; // is 0 since new buying cycle starts
    
    // check how much time left until next market opening
    uint tCurr = clock.getCurrTime();
    uint tSale = clock.computeTimingHourDayAfter( MARKET_OPENING_HOUR ); // collect until market opening

    if ( ( tCurr == tSale ) or ( collectionTaskVec.size() == 0 ) )
        return; // nothing to do
    
    // assign random execution times to tasks
    // lump repeated visits to the same farm together
    std::unordered_map<NodeFarm*,uint> taskTimes;
    for (auto& task: collectionTaskVec) {
        if ( taskTimes.find( task->nf ) == taskTimes.end() ) {  // farm not yet assigned: sample new visit time
            task->t = randGen.getUniInt( tCurr, tSale - 1 );
            taskTimes[ task->nf ] = task->t;
        }
        else {                                                  // farm already assigned: use same visit time
            task->t = taskTimes[ task->nf ];
        }
    }
    
    // sort tasks in ascending order according to their timing
    std::sort( collectionTaskVec.begin(), collectionTaskVec.end(), compareCollectionTaskTimingPtr );
    

    /*
    if ( mman->getId() == 0 ) {
        for ( auto& task: collectionTaskVec ) {
            std::cout << task->t << " " << task->nf->getContext()->getId() << " " << task->nbirds << " " << task->nmId << std::endl;
        }
    }
     */
    
}

// process a single collection task
void NodeMman::processCollectionTask( CollectionTask* task, Random& randGen, BirdTransferManager& transferManager ) {
    transferManager.transferBirdsFarm2Mman( task->nf, this, task->bt, task->nmId, task->nbirds, randGen );
}

// process current collection tasks
void NodeMman::processCurrentCollectionTasks( const Clock &clock, Random& randGen, BirdTransferManager& transferManager ) {
    
    uint tCurr = clock.getCurrTime();
    
    while( true ) {

        if ( currCollectionTask == collectionTaskVec.size() )
            break; // no more tasks
        
        CollectionTask* task = collectionTaskVec[currCollectionTask];
                
        if ( task->t == tCurr ) {
            processCollectionTask( task, randGen, transferManager );
            currCollectionTask++;
            
            //if ( mman->getId() == 0 )
            //    std::cout << "t: " << clock.getCurrHour() << " " << currCollectionTask << std::endl;
        }
        else
            break; // no more tasks (for now)
    }
    
}

void NodeMman::clearCollectionTasks() {
    for ( CollectionTask*& task: collectionTaskVec )
        delete task;
    collectionTaskVec.resize(0);
    currCollectionTask = 0;
    //provisionalCargo = 0;
    
}

bool NodeMman::hasCompletedCollectionTasks() {
    if ( currCollectionTask == 0 )
        return false;
    if ( currCollectionTask == collectionTaskVec.size() )
        return true;
    else
        return false;
}

//=== selling routines ===//

// makes middleman sell its batch to markets
void NodeMman::sellFullCargoToMarkets( Random& randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) {
    
    BirdType bt = getTradedBirdType()[0]; // assumes a single species is traded by this middleman
    std::vector<uint>* marketDestinations = mman->getNonEmptyMarketIndexes( bt );
        
    for ( uint& marketDst: *marketDestinations ) {
        NodeMarket* nm = marketList[ marketDst ];
        if ( nm->isOpen() ) {
            uint nBirds = mman->getBirdSizeType( bt, marketDst );
            //std::cout << this->getContext()->getId() << " " << marketDst << " " << nBirds << " " <<  mman->getBirdSizeType( bt, marketDst ) << std::endl ;
            transferManager.transferBirdsMman2Market( this, nm, bt, nBirds, randGen );
        }
    }
    
    delete marketDestinations; // delete array with market destinations
    
    
}


// contacts farms in currently scouted areas according to probabilities held in areaMngr
// it assumes a single species is traded
void NodeMman::buyFromFarmsSingleSpecies( const uint& nMarketsToVisit, Random& randGen, AreaManager& areaMngr ) {
    
    BirdType bt = mman->getTradedBirdTypes()[0];
    uint nBirdsToBuyMax = mman->getAvailableSpace(); // maximum number of birds that can be bought
    uint nBirdsToBuyMaxRunning = nBirdsToBuyMax;
    
    DiscreteDistribution P_jl = areaMngr.synthesizeMarketChoiceDistribution( bt, areasIds );
    
    if ( !P_jl.isValid() ) // stop: no farms trading
        return ;
    
    uint nMarketsToVisitSafe = P_jl.getNumberPosWeights();
    nMarketsToVisitSafe = std::min( nMarketsToVisitSafe, nMarketsToVisit );
    
    // i) draw market visit tokens from P_jl until enough distinct markets are generated
    // tokens define the relative number of visits to each market
    
    std::unordered_map<uint, uint> tokens = {};
    uint nTokensTotal = 0;
    
    std::vector<uint> marketsToVisit = {};
    marketsToVisit.reserve( nMarketsToVisitSafe );
    
    while ( tokens.size() < nMarketsToVisitSafe ) { // number of distinct token types -> number of distinct markets to visit
        uint marketId = randGen.getCustomDiscrete( P_jl );
        tokens[marketId]++;
        nTokensTotal++;
        
        if ( tokens[marketId] == 1 )
            marketsToVisit.push_back( marketId ) ;
        
        if ( nTokensTotal > 1000 )
            break ; // stop after too many trials

    }
        
    // ii) For each token type, get token count and redistribute across regions
    // weighting factor is given by Q_al, normalized by sum of Q_al over visited regions
    
    for ( auto& elem: tokens ) {
        
        double cumP = 0.;
        
        uint marketId = elem.first;
        uint nTokens = elem.second;
        
        // compute P_aj(l) and redistribute tokens
        // create list of tokens
        std::vector<double> p_ajl = areaMngr.synthesize_P_ajl( bt, marketId, areasIds );
        for ( uint ia = 0; ia < areasIds.size(); ++ia ) {
            double p_tmp = p_ajl[ia];
            if ( p_tmp > 0. ) {
                uint nTokensTmp = randGen.getBinom( p_tmp / ( 1 - cumP ), nTokens );
                if ( nTokensTmp > 0 ) {
                    
                    uint areaId = areasIds[ia];
                    
                    uint nBirdsToBuyAreaMarket = (uint)( 0.5 + nBirdsToBuyMax * nTokensTmp * 1. / nTokensTotal );
                    nBirdsToBuyAreaMarket = std::min( nBirdsToBuyAreaMarket, nBirdsToBuyMaxRunning );
                    if ( nBirdsToBuyAreaMarket > 0 ) {
                        addOrder( areaId, marketId, nBirdsToBuyAreaMarket );
                        nBirdsToBuyMaxRunning -= nBirdsToBuyAreaMarket;
                    }
                    
                    nTokens -= nTokensTmp;
                }
            }
            cumP += p_tmp;
        }
    }
    
    // now use tokens to contact farms
    uint nBirdsSecured = 0;
    std::vector<NodeFarm*> backupFarms = {};
    backupFarms.reserve(15);
    for ( auto& elem: orderTaskList ) {
        uint areaId = elem.first;
        auto& orders = elem.second;
        auto& farmList = areaMngr.getTradingFarmsArea( bt, areaId );

        if ( farmList.empty() ) // no farms trading
            continue;
        
        auto itnf = farmList.begin();
        auto itnfend = farmList.end();

        while( !orders.empty() ) { // no more orders
            auto& order = orders.back();
            uint& marketId = order.first;
            uint& nbirds = order.second;
            auto nf = *itnf;
            
            // contact farm, ask for birds
            uint nBirdsAccept = nf->evalBatchSizeRequest( bt, nbirds );
            
            if ( nBirdsAccept > 0 ) {
                addCollectionTask( nf, marketId, bt, nBirdsAccept );
                nbirds -= nBirdsAccept;
                nBirdsSecured += nBirdsAccept;
                
                // check if farm still has available birds (might be contacted later
                // and add it to list
                if ( nf->hasAvailableBirds( bt ) ) {
                    if ( backupFarms.empty() )
                        backupFarms.push_back( nf );
                    else {
                        if ( backupFarms.back() != nf )
                            backupFarms.push_back( nf );
                    }
                }
                
            }
            else {
                // move to next farm in the same area
                ++itnf;
                if ( itnf >= itnfend )
                    break; // no more farms
            }
            
            if ( nbirds == 0 )
                orders.pop_back(); // remove this particular order
        
        }
        
    }
    
    // ask again to previously visited farms
    if ( nBirdsSecured < mman->getAvailableSpace() ) {
        uint nbirds = mman->getAvailableSpace() - nBirdsSecured;

        for ( auto& nf: backupFarms ) {
            uint nBirdsAccept = nf->evalBatchSizeRequest( bt, nbirds );
            if ( nBirdsAccept > 0 ) {
                DiscreteDistribution p_l = areaMngr.synthesize_P_al_Subset( bt, nf->getAreaId(), marketsToVisit );
                uint marketId = marketsToVisit[ randGen.getCustomDiscrete( p_l ) ];
                addCollectionTask( nf, marketId, bt, nBirdsAccept );
                nbirds -= nBirdsAccept;
                nBirdsSecured += nBirdsAccept;
                //std::cout << mman->getId() << "," << nBirdsAccept << "\n";
            }
            
            if ( nbirds == 0 )  // stop if birds satisfied
                break;
        }
        
    }
    clearOrders();
    
        
}

/* // old code
for ( auto& elem: tokens ) {
    uint marketId = elem.first;
    uint nTokens = elem.second;
    
    // compute P_aj(l) and redistribute tokens
    // create list of tokens
    std::vector<double> p_ajl = areaMngr.synthesize_P_ajl( bt, marketId, areasIds );
            
    for ( uint ia = 0; ia < areasIds.size(); ++ia ) {
        uint areaId = areasIds[ia];
        
        // iii) sample number of chickens to source from area a and direct them to market l
        
        double weight = p_ajl[ia] * nTokens / nTokensTotal;
        uint nBirdsToBuyAreaMarket = 0;
        if ( weight > 0. ) {
            //std::cout << weight << "," << (uint)(nBirdsToBuyMax) << "," << (uint)(weight * nBirdsToBuyMax) << "\n";
            nBirdsToBuyAreaMarket = randGen.getPoisson( weight * nBirdsToBuyMax );
            nBirdsToBuyAreaMarket = std::min( nBirdsToBuyAreaMarket, nBirdsToBuyMaxRunning );
        }
        
        if ( nBirdsToBuyAreaMarket > 0 ) {
            addOrder( areaId, marketId, nBirdsToBuyAreaMarket );
            nBirdsToBuyMaxRunning -= nBirdsToBuyAreaMarket;
        }
    }
}
 */

    
// decide to move according to fixed probability
bool NodeMman::decideToMoveFixedProb( const double &probMove, Random &randGen ) {
    return randGen.getBool( probMove );
}

// move to new areas. A focal area is first chosen according to outgoing bird flux volume
// Additional areas are then chosen according to distance.
void NodeMman::moveToNewAreasNeighboring( const uint& nAreas, Random& randGen, AreaManager& areaMngr ) {
    
    // assumes a single species is traded
    
    // clear currently visited areas
    areasIds.clear();
    
    // select a single area proportionally to Pa
    //BirdType bt = mman->getTradedBirdTypes()[0];
    //DiscreteDistribution Pa( areaMngr.get_P_a(bt) ); // choose proportionally to bird output
    //uint ia = randGen.getCustomDiscrete( Pa );
    
    //uint Na = areaMngr.getNumberOfAreas(); // choose a random area
    //uint ia = randGen.getUniInt( Na - 1 );
    //areasIds.push_back( ia );
    
    // choose proportionally to bird output
    uint ia = areaMngr.getRandomAreaUsingFarmProduction( mman->getTradedBirdTypes()[0], randGen );
    areasIds.push_back( ia );
    
    // select (nAreas - 1) additional areas that are near to ia
    if ( nAreas > 1 ) {
        std::vector<uint> ia_nbrs = areaMngr.getNeighboringAreas( ia, nAreas - 1, randGen );
        for ( uint ia_nbr: ia_nbrs )
            areasIds.push_back( ia_nbr );
    }
    
}

void NodeMman::addOrder( const uint& areaID, const uint& marketID, const uint& nbirds ) {
    orderTaskList[areaID].push_back( { marketID, nbirds } );
}
void NodeMman::clearOrders() {
    orderTaskList.clear();
}

std::vector<std::pair<uint,uint>>& NodeMman::getOrdersSingleArea( const uint areaID ) {
    return orderTaskList[areaID];
}


// store properties of incoming birds bought from farms
void NodeMman::addFarmTradedBird( pBird& bird ) {
    
    for ( auto& tracker : trackersFarmTradedBird )
        tracker( bird ) ;
    
}


void NodeMman::doRecordIncomingInfectedBirds( bool checkExposed ) {
    
    nIncomingInfectedBirds = 0 ;
    
    if ( checkExposed ) { // track both E and I birds
        
        trackersFarmTradedBird.push_back( std::bind( &NodeMman::trackExposedAndInfectedBird, this, std::placeholders::_1 ) ) ;
        
    }
    else { // track I birds only
        
        trackersFarmTradedBird.push_back( std::bind( &NodeMman::trackInfectedBird, this, std::placeholders::_1 ) ) ;
        
    }
        
}


// updates count of infectious incoming birds
void NodeMman::trackInfectedBird( pBird& bird ) {
    
    if ( bird->infState != 0 )
        ++nIncomingInfectedBirds ;
    
}

// updates count of exposed and infectious incoming birds
void NodeMman::trackExposedAndInfectedBird( pBird& bird ) {
    
    if ( ( bird->infState ) != 0 or ( bird->pendingInfections > 0 ) )
        ++nIncomingInfectedBirds ;
    
}

//=== external requests handlers ===//

/*
uint NodeMman::processBatchRequest( NodeFarm *nf, const BirdType& bt, const uint &nbirds ) {
    return state->onProcessBatchRequest( this, nf, bt, nbirds );
}
 */

void NodeMman::moveToNewAreasRequest( Random& randGen, AreaManager& areaMngr ) {
    state->onMoveToNewAreasRequest( this, randGen, areaMngr );
}

void NodeMman::forceMoveToNewAreasRequest( Random& randGen, AreaManager& areaMngr ) {
    state->onForceMoveToNewAreasRequest( this, randGen, areaMngr );
}


void NodeMman::buyFromFarmsRequest( Random &randGen, AreaManager &areaMngr ) {
    state->onBuyFromFarmsRequest( this, randGen, areaMngr );
}

void NodeMman::executeScheduleTasksRequest( const Clock &clock, Random &randGen ) {
    state->onExecuteScheduleTasksRequest( this, clock, randGen );
}

void NodeMman::executeCollectionTasksRequest( const Clock &clock, Random &randGen, BirdTransferManager& transferManager ) {
    state->onExecuteCollectionTasksRequest( this, clock, randGen, transferManager );
}

void NodeMman::sellToMarketsRequest(Random &randGen, BirdTransferManager& transferManager, std::vector<NodeMarket*>& marketList ) {
    state->onSellToMarketsRequest( this, randGen, transferManager, marketList );
}

void NodeMman::resetRequest( ChickenPool* birdMngr ) {
    state->onResetRequest( this, birdMngr );
}


//=== getters and setters ===//

void NodeMman::setState(StateMman* nextState) {
    state = nextState;
}

void NodeMman::setState( StateMmanAwaiting* nextState ) {
    state = nextState;
}


uint NodeMman::getNtasks() {
    return static_cast<uint>( collectionTaskVec.size() );
}

Middleman* NodeMman::getContext() {
    return mman;
}
