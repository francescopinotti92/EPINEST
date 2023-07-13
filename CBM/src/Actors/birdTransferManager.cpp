//
//  birdTransferManager.cpp
//  CBM
//
//

#include "birdTransferManager.hpp"

TransactionFarm2Mman::TransactionFarm2Mman() {};
TransactionFarm2Mman::TransactionFarm2Mman( const uint& farmID_, const uint& mmanID_, const uint& marketID_, const BirdType& bt_, const uint& nbirds_, const uint& t_): farmID( farmID_ ), mmanID( mmanID_ ), marketID( marketID_ ), bt( bt_ ), nbirds( nbirds_ ), t( t_ ) {};

std::string TransactionFarm2Mman::stringify() {
    std::string res = "PF-";
    res += std::to_string( farmID ) + "-";
    res += "MM-";
    res += std::to_string( mmanID ) + "-";
    res += "M-";
    res += std::to_string( marketID ) + "-";
    res += constants::getStringFromBirdType( bt ) + "-";
    res += std::to_string( t ) + "-";
    res += std::to_string( nbirds );
    return res;
}

TransactionFarm2Vendor::TransactionFarm2Vendor() {};
TransactionFarm2Vendor::TransactionFarm2Vendor(const uint& farmID_, const uint& vendorID_, const uint& marketID_, const BirdType& bt_, const uint& nbirds_, const uint& t_): farmID( farmID_ ), vendorID( vendorID_ ), marketID( marketID_ ), bt( bt_ ), nbirds( nbirds_ ), t( t_ ) {};
std::string TransactionFarm2Vendor::stringify() {
    return fmt::format("PF-{}-V-{}-M-{}-{}-{}-{}", farmID, vendorID, marketID, constants::getStringFromBirdType( bt ), t, nbirds );
}


TransactionMman2Vendor::TransactionMman2Vendor() {};
TransactionMman2Vendor::TransactionMman2Vendor( const uint& mmanID_, const uint& vendorID_, const uint& marketID_, const BirdType& bt_, const uint& nbirds_, const uint& t_): mmanID( mmanID_), vendorID(vendorID_), marketID( marketID_ ), bt( bt_ ), nbirds( nbirds_ ), t( t_ ) {};

std::string TransactionMman2Vendor::stringify() {
    std::string res = "MM-";
    res += std::to_string( mmanID ) + "-";
    res += "V-";
    res += std::to_string( vendorID ) + "-";
    res += "M-";
    res += std::to_string( marketID ) + "-";
    res += constants::getStringFromBirdType( bt ) + "-";
    res += std::to_string( t ) + "-";
    res += std::to_string( nbirds );
    return res;
}

TransactionVendor2Vendor::TransactionVendor2Vendor() {};
TransactionVendor2Vendor::TransactionVendor2Vendor( const uint& vendorID1_, const uint& vendorID2_, const uint& marketID_, const BirdType& bt_, const uint& nbirds_, const uint& t_): vendorID1( vendorID1_ ), vendorID2( vendorID2_ ), marketID( marketID_ ), bt( bt_ ), nbirds( nbirds_ ), t( t_ ) {};

std::string TransactionVendor2Vendor::stringify() {
    std::string res = "V-";
    res += std::to_string( vendorID1 ) + "-";
    res += "V-";
    res += std::to_string( vendorID2 ) + "-";
    res += "M-";
    res += std::to_string( marketID ) + "-";
    res += constants::getStringFromBirdType( bt ) + "-";
    res += std::to_string( t ) + "-";
    res += std::to_string( nbirds );
    return res;
}

TransactionVendor2Community::TransactionVendor2Community() {};
TransactionVendor2Community::TransactionVendor2Community( const uint& vendorID_, const uint& marketID_, const BirdType& bt_, const uint& nbirds_, const uint& t_): vendorID( vendorID_ ), marketID( marketID_ ), bt( bt_ ), nbirds( nbirds_ ), t( t_ ) {};

std::string TransactionVendor2Community::stringify() {
    std::string res = "V-";
    res += std::to_string( vendorID ) + "-";
    res += "M-";
    res += std::to_string( marketID ) + "-";
    res += constants::getStringFromBirdType( bt ) + "-";
    res += std::to_string( t ) + "-";
    res += std::to_string( nbirds );
    return res;
}

//========================= BirdTransferManager =============================//

//====== constructors =======//

BirdTransferManager::BirdTransferManager( NamedOfstream outFile_, Clock* clock_, ChickenPool* pool, uint dayObsStart_, uint dayObsStop_, bool logFarm2Mman, bool logMman2Vendor, bool logVendor2Vendor, bool logVendor2Community, uint maxTransactionsBuffer_ ): clock( clock_ ), birdMngr( pool ), dayObsStart( dayObsStart_ ), dayObsStop( dayObsStop_ ), maximumTransactions( maxTransactionsBuffer_ ), outFile( outFile_ ) {
        
    birdExitTracker =       nullptr;
    completedCropTracker =  nullptr;
    
    if ( logFarm2Mman or logMman2Vendor or logVendor2Vendor or logVendor2Community ) {
        if ( outFile.isClosed() ) {
            throw Warning( "stream to write transactions not valid" );
        }
        if ( maximumTransactions == 0 )
            maximumTransactions = 1;
        if ( maximumTransactions > 1000000 )
            maximumTransactions = 100000;
    }
    
    if ( logFarm2Mman ) {
        this->transferBirdsFarm2Mman = std::bind( &BirdTransferManager::_transferBirdsFarm2MmanLog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6 );
        
        farm2MmanBuffer.reserve( maximumTransactions );
    }
    else {
        this->transferBirdsFarm2Mman = std::bind( &BirdTransferManager::_transferBirdsFarm2Mman, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6 );
    }
    
    
    if ( logMman2Vendor ) {
        this->transferBirdsMman2Market = std::bind( &BirdTransferManager::_transferBirdsMman2MarketLog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5 );
        
        mman2VendorBuffer.reserve( maximumTransactions );
    }
    else {
        this->transferBirdsMman2Market = std::bind( &BirdTransferManager::_transferBirdsMman2Market, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5 );
    }

    if ( logVendor2Vendor ) {
        this->transferBirdsVendor2Vendor = std::bind( &BirdTransferManager::_transferBirdsVendor2VendorLog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5) ;
        
        vendor2VendorBuffer.reserve( maximumTransactions );
    }
    else {
        this->transferBirdsVendor2Vendor = std::bind( &BirdTransferManager::_transferBirdsVendor2Vendor, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5 );
    }
    
    if ( logVendor2Community ) {
        this->transferBirdsVendor2Community = std::bind( &BirdTransferManager::_transferBirdsVendor2CommunityLog, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4 );
        
        vendor2CommunityBuffer.reserve( maximumTransactions );
    }
    else {
        this->transferBirdsVendor2Community = std::bind( &BirdTransferManager::_transferBirdsVendor2Community, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4 );
    }
    
}

//====== resetter ======//

void BirdTransferManager::reset( bool updateStream ) {
    
    farm2MmanBuffer.clear();
    mman2VendorBuffer.clear();
    vendor2VendorBuffer.clear();
    vendor2CommunityBuffer.clear();
    
    birdExitTracker->reset( updateStream );
    completedCropTracker->reset( updateStream );
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
}


// set tracker manager
void BirdTransferManager::setBirdExitTrackerManager( BirdExitTrackerManager* trackerManager ) {
   
    // set tracker only if there is no tracker set yet
    if ( birdExitTracker == nullptr )
        birdExitTracker = trackerManager ;

}

void BirdTransferManager::setCompletedCropTrackerManager( CompletedCropTrackerManager* trackerManager ) {
    
    // set tracker only if there is no tracker set yet
    if ( completedCropTracker == nullptr )
        completedCropTracker = trackerManager ;
    
}



//====== single transaction-adding functions ======//

void BirdTransferManager::addFarm2MmanTransaction( NodeFarm* nf, NodeMman* nmm, const uint& marketID, const BirdType& bt, const uint& nbirds ) {
    
    if ( ( clock->getCurrDay() < dayObsStart ) or ( clock->getCurrDay() > dayObsStop ) )
        return; // do not record outside of logging window
    
    if ( farm2MmanBuffer.size() >= maximumTransactions ) {
        flushFarm2MmanTransactions();
    }
    
    farm2MmanBuffer.push_back( TransactionFarm2Mman( nf->getContext()->getId(), nmm->getContext()->getId(), marketID, bt, nbirds, clock->getCurrTime() ) );
    
}

void BirdTransferManager::addMman2VendorTransaction( NodeMman *nmm, NodeVendor *nv, NodeMarket *nm, const BirdType &bt, const uint &nbirds ) {
    
    if ( ( clock->getCurrDay() < dayObsStart ) or ( clock->getCurrDay() > dayObsStop ) )
        return; // do not record outside of logging window
    
    if ( mman2VendorBuffer.size() >= maximumTransactions ) {
        flushMman2VendorTransactions();
    }
    
    mman2VendorBuffer.push_back( TransactionMman2Vendor( nmm->getContext()->getId(), nv->getContext()->getId(), nm->getContext()->getId(), bt, nbirds, clock->getCurrTime() ) );
    
}

void BirdTransferManager::addVendor2VendorTransaction( NodeVendor* nv1, NodeVendor* nv2, NodeMarket* nm, const BirdType& bt, const uint& nbirds ) {
    
    if ( ( clock->getCurrDay() < dayObsStart ) or ( clock->getCurrDay() > dayObsStop ) )
        return; // do not record outside of logging window
    
    if ( vendor2VendorBuffer.size() >= maximumTransactions ) {
        flushVendor2VendorTransactions();
    }
    
    vendor2VendorBuffer.push_back( TransactionVendor2Vendor( nv1->getContext()->getId(), nv2->getContext()->getId(), nm->getContext()->getId(), bt, nbirds, clock->getCurrTime() ) );
    
}
void BirdTransferManager::addVendor2CommunityTransaction( NodeVendor* nv, NodeMarket* nm, const BirdType& bt, const uint& nbirds ) {
    
    if ( ( clock->getCurrDay() < dayObsStart ) or ( clock->getCurrDay() > dayObsStop ) )
        return; // do not record outside of logging window
    
    if ( vendor2CommunityBuffer.size() >= maximumTransactions ) {
        flushVendor2CommunityTransactions();
    }
    
    vendor2CommunityBuffer.push_back( TransactionVendor2Community( nv->getContext()->getId(), nm->getContext()->getId(), bt, nbirds, clock->getCurrTime() ) );
    
}

//====== output functions ======//

void BirdTransferManager::flushFarm2MmanTransactions() {
    for ( auto& transaction: farm2MmanBuffer )
        outFile.getStream() << transaction.stringify() << std::endl;
    farm2MmanBuffer.resize(0);
}

void BirdTransferManager::flushMman2VendorTransactions() {
    for ( auto& transaction: mman2VendorBuffer )
        outFile.getStream() << transaction.stringify() << std::endl;
    mman2VendorBuffer.resize(0);
}

void BirdTransferManager::flushVendor2VendorTransactions() {
    for ( auto& transaction: vendor2VendorBuffer )
        outFile.getStream() << transaction.stringify() << std::endl;
    vendor2VendorBuffer.resize(0);
}

void BirdTransferManager::flushVendor2CommunityTransactions() {
    for ( auto& transaction: vendor2CommunityBuffer )
        outFile.getStream() << transaction.stringify() << std::endl;
    vendor2CommunityBuffer.resize(0);
}

void BirdTransferManager::flushEverything() {
    if ( !outFile.isClosed() ) {
        flushFarm2MmanTransactions();
        flushMman2VendorTransactions();
        flushVendor2VendorTransactions();
        flushVendor2CommunityTransactions();
    }
}

//====== Transfer functions (without logging) ======//

void BirdTransferManager::_transferBirdsFarm2Mman( NodeFarm* nf, NodeMman* nmm, const BirdType& bt, const uint& marketDst, const uint& nbirds, Random& randGen ) {
        
    Farm* farm = nf->getContext();
    Middleman* mman = nmm->getContext();
    
    std::deque<Crop*>& crops = farm->getReadyCrops( bt );
    BirdUrn& urn = mman->getBirdUrn( bt, marketDst );
    
    if ( crops.empty() )
        return; // no crops
    
    bool isDone = false;
    uint nbirds_moved = 0;
    
    while( !isDone ) { // loop over 'ready' crops
        
        Crop& crop = *( crops.front() );
        uint cropSize = crop.size();
        uint nbirdsTmp = std::min( cropSize, nbirds - nbirds_moved ); // maximum number of birds that can be moved
        
        for ( uint i = 0; i < nbirdsTmp; ++i ) {
            
            // remove and update chicken
            uint birdIdx = crop.removeLast();
            pBird bird = farm->removeBird( birdIdx );
            bird->marketDestination = marketDst;
            bird->movements.increaseMcount();
            
            // insert bird
            nmm->addFarmTradedBird( bird ) ;
            uint new_index = mman->insertBird( std::move( bird ) ).index;
            urn.insertBird( new_index );
            
            nbirds_moved++;
            if ( nbirds_moved == nbirds )
                isDone = true;

        }
        
        // check if crop is empty
        if ( crop.empty() ) {

            crops.pop_front();
            completedCropTracker->processCompletedCrop( *nf, &crop, *clock );
            delete &crop;
            isDone = true; // stop if no crops at all
            
        }
        
        // check if order is fulfilled
        //if ( nbirds_moved == nbirds )
        //    isDone = true; // stop if enough birds have been moved
        
    }
    
}

void BirdTransferManager::_transferBirdsMman2Market( NodeMman *nmm, NodeMarket *nm, const BirdType &bt, const uint &nbirds, Random &randGen ) {
    
    uint tCurr = clock->getCurrTime();
    
    uint nbirdsWS = randGen.getBinom( nm->getPropSoldMmenToWholesalers( bt ), nbirds );
    uint nbirdsR = nbirds - nbirdsWS;
    uint nbirdsParcel = 0;
    
    Middleman* mman = nmm->getContext();
    Market* market = nm->getContext();
    uint marketId = market->getId();
    
    BirdUrn& urn = mman->getBirdUrn( bt, marketId );
    
    // sell to wholesalers in first layer
    if ( nbirdsWS > 0 ) {
        std::vector<NodeVendor*>& nvs_ws = nm->getBuyersWholesalers( bt, 0 );
        
        for ( auto& nv: nvs_ws ) {
            nbirdsParcel = nv->processBatchFromMmanRequest( nbirdsWS );
            
            if ( nbirdsParcel > 0 ) {
                
                Vendor* vendor = nv->getContext();
                
                for ( uint i = 0; i < nbirdsParcel; ++i ) {
                
                    uint birdIndex = urn.pop_front();
                    if ( birdIndex != BirdUrn::INVALID ) {
                        
                        pBird bird = mman->removeBird( birdIndex );
                        bird->movements.increaseVcount();
                        bird->t_market = tCurr;
                        nm->addMmanTradedBird( bird ) ;
                        market->insertNewBird( std::move( bird ), vendor );
                        
                    }
                    else {
                        break;
                    }
                }
                
                nv->increaseBirdBoughtCount( nbirdsParcel );
                
                nbirdsWS -= nbirdsParcel;
                if ( nbirdsWS == 0 )
                    break;
            }
        }
    }
    
    if ( nbirdsWS > 0 ) { // if any remnant birds, sell them to retailers
        nbirdsR += nbirdsWS;
    }
    
    // sell to retailers in first layer
    if ( nbirdsR > 0 ) {
        std::vector<NodeVendor*>& nvs_r = nm->getBuyersRetailers( bt, 0 );
        for ( auto& nv: nvs_r ) {
            nbirdsParcel = nv->processBatchFromMmanRequest( nbirdsR );
            
            if ( nbirdsParcel > 0 ) {
                Vendor* vendor = nv->getContext();
                
                for ( uint i = 0; i < nbirdsParcel; ++i ) {
                    uint birdIndex = urn.pop_front();
                    if ( birdIndex != BirdUrn::INVALID ) {
                        
                        pBird bird = mman->removeBird( birdIndex );
                        bird->movements.increaseVcount();
                        bird->t_market = tCurr;
                        nm->addMmanTradedBird( bird ) ;
                        market->insertNewBird( std::move( bird ), vendor );
                        
                    }
                    else {
                        break;
                    }
                }
                
                nv->increaseBirdBoughtCount( nbirdsParcel );
                
                nbirdsR -= nbirdsParcel;
                if ( nbirdsR == 0 )
                    break;
            }
        }
    }
    
}

void BirdTransferManager::_transferBirdsVendor2Vendor( NodeVendor *nv1, NodeVendor *nv2, NodeMarket *nm, uint nbirds, Random &randGen ) {
    Vendor* src = nv1->getContext();
    Vendor* dst = nv2->getContext();
    Market* market = nm->getContext();
        
    // compute number of birds to source from either repurposed or newly bought bird pools
    std::pair<uint, uint> cts = nv1->getTransactionDetails( nbirds );
    
    uint birdIdx = 0;
    for ( uint i = 0, nrep = cts.first; i < nrep; ++i ) {       // move repurposed birds
        birdIdx = src->popRepurposedBirdIndex();
        pBird& bird = market->getBirdSparse( birdIdx );
        bird->vendorPtr = dst; // update bird vendor owner
        bird->movements.increaseVcount();
        dst->addNewBirdIndex( birdIdx );
    }
    
    for ( uint i = 0, nnew = cts.second; i < nnew; ++i ) {      // move newly bought birds
        birdIdx = src->popNewBirdIndex();
        pBird& bird = market->getBirdSparse( birdIdx );
        bird->vendorPtr = dst; // update bird vendor owner
        bird->movements.increaseVcount();
        dst->addNewBirdIndex( birdIdx );
    }
    
    nv2->increaseBirdBoughtCount( nbirds );

}

void BirdTransferManager::_transferBirdsVendor2Community( NodeVendor *nv, NodeMarket *nm, uint nbirds, Random &randGen ) {
    
    Vendor* vendor = nv->getContext();
    Market* market = nm->getContext();
    
    uint birdIdx = 0;
    
    std::pair<uint, uint> cts = nv->getTransactionDetails( nbirds );
    
    for ( uint i = 0, nrep = cts.first; i < nrep; ++i ) {           // move repurposed birds
        birdIdx = vendor->popRepurposedBirdIndex();
        pBird bird = market->removeBird( birdIdx );
        birdExitTracker->birdExitsFromMarketOrVendor( bird, *clock );
        birdMngr->return_bird( std::move( bird ) );
    }
    
    for ( uint i = 0, nnew = cts.second; i < nnew; ++i ) {           // move new birds
        birdIdx = vendor->popNewBirdIndex();
        pBird bird = market->removeBird( birdIdx );
        birdExitTracker->birdExitsFromMarketOrVendor( bird, *clock );
        birdMngr->return_bird( std::move( bird ) );
    }
    
}

//====== Transfer functions (with logging) ======//

void BirdTransferManager::_transferBirdsFarm2MmanLog( NodeFarm* nf, NodeMman* nmm, const BirdType& bt, const uint& marketDst, const uint& nbirds, Random& randGen ) {
        
    Farm* farm = nf->getContext();
    Middleman* mman = nmm->getContext();
    
    std::deque<Crop*>& crops = farm->getReadyCrops( bt );
    BirdUrn& urn = mman->getBirdUrn( bt, marketDst );
    
    if ( crops.empty() )
        return; // no crops
    
    bool isDone = false;
    uint nbirds_moved = 0;
        
    while( !isDone ) {
        Crop& crop = *( crops.front() );
        
        uint cropSize = crop.size();
        uint nbirdsTmp = std::min( cropSize, nbirds - nbirds_moved );
        
        for ( uint i = 0; i < nbirdsTmp; ++i ) {
            
            // remove and update chicken
            uint birdIdx = crop.removeLast();
            pBird bird = farm->removeBird( birdIdx );
            bird->marketDestination = marketDst;
            bird->movements.increaseMcount();
            
            // insert bird
            nmm->addFarmTradedBird( bird ) ;
            uint new_index = mman->insertBird( std::move( bird ) ).index;
            urn.insertBird( new_index );
            
            nbirds_moved++;
            if ( nbirds_moved == nbirds )
                isDone = true;
            
        }

        // check if crop is empty
        if ( crop.empty() ) {
            
            crops.pop_front();
            completedCropTracker->processCompletedCrop( *nf, &crop, *clock );
            delete &crop;
            if ( crops.empty() ) {
                addFarm2MmanTransaction( nf, nmm, marketDst, bt, nbirds_moved );
                return;
            }
            
        }
        
    }
    
    addFarm2MmanTransaction( nf, nmm, marketDst, bt, nbirds_moved );
    
}

void BirdTransferManager::_transferBirdsMman2MarketLog( NodeMman *nmm, NodeMarket *nm, const BirdType &bt, const uint &nbirds, Random &randGen ) {
    
    uint tCurr = clock->getCurrTime();
    
    uint nbirdsWS = randGen.getBinom( nm->getPropSoldMmenToWholesalers( bt ), nbirds );
    uint nbirdsR = nbirds - nbirdsWS;
    uint nbirdsParcel = 0;
    
    Middleman* mman = nmm->getContext();
    Market* market = nm->getContext();
    uint marketId = market->getId();
    
    BirdUrn& urn = mman->getBirdUrn( bt, marketId );
    
    // sell to wholesalers in first layer
    if ( nbirdsWS > 0 ) {
        
        std::vector<NodeVendor*>& nvs_ws = nm->getBuyersWholesalers( bt, 0 );
        
        for ( auto& nv: nvs_ws ) {
            nbirdsParcel = nv->processBatchFromMmanRequest( nbirdsWS );
            
            if ( nbirdsParcel > 0 ) {
                
                Vendor* vendor = nv->getContext();
                
                for ( uint i = 0; i < nbirdsParcel; ++i ) {
                    
                    uint birdIndex = urn.pop_front();
                    if ( birdIndex != BirdUrn::INVALID ) {
                        
                        pBird bird = mman->removeBird( birdIndex );
                        bird->movements.increaseVcount();
                        bird->t_market = tCurr;
                        nm->addMmanTradedBird( bird ) ;
                        market->insertNewBird( std::move( bird ), vendor );
                        
                    }
                    else {
                        break;
                    }
                    
                   
                }
                
                addMman2VendorTransaction( nmm, nv, nm, bt, nbirdsParcel );
                nv->increaseBirdBoughtCount( nbirdsParcel );

                nbirdsWS -= nbirdsParcel;
                if ( nbirdsWS == 0 )
                    break;
            }
        }
    }
        
    // any unsold birds to wholesalers?
    if ( nbirdsWS > 0 )
        nbirdsR += nbirdsWS;
    
    // sell to retailers in first layer
    if ( nbirdsR > 0 ) {
        std::vector<NodeVendor*>& nvs_r = nm->getBuyersRetailers( bt, 0 );
        for ( auto& nv: nvs_r ) {
            
            nbirdsParcel = nv->processBatchFromMmanRequest( nbirdsR );
            
            if ( nbirdsParcel > 0 ) {
                Vendor* vendor = nv->getContext();
                
                for ( uint i = 0; i < nbirdsParcel; ++i ) {
                    
                    uint birdIndex = urn.pop_front();
                    if ( birdIndex != BirdUrn::INVALID ) {
                        
                        pBird bird = mman->removeBird( birdIndex );
                        bird->movements.increaseVcount();
                        bird->t_market = tCurr;
                        nm->addMmanTradedBird( bird ) ;
                        market->insertNewBird( std::move( bird ), vendor );
                    }
                    else {
                        break;
                    }
                
                }
                
                addMman2VendorTransaction( nmm, nv, nm, bt, nbirdsParcel );
                nv->increaseBirdBoughtCount( nbirdsParcel );

                nbirdsR -= nbirdsParcel;
                if ( nbirdsR == 0 )
                    break;
                
            }
        }
    }
        
}

void BirdTransferManager::_transferBirdsVendor2VendorLog( NodeVendor *nv1, NodeVendor *nv2, NodeMarket *nm, uint nbirds, Random &randGen ) {
    
    uint birdIdx = 0;
    Vendor* src = nv1->getContext();
    Vendor* dst = nv2->getContext();
    Market* market = nm->getContext();
        
    // compute number of birds to source from either repurposed or newly bought bird pools
    std::pair<uint, uint> cts = nv1->getTransactionDetails( nbirds );
    
    for ( uint i = 0, nrep = cts.first; i < nrep; ++i ) {       // move repurposed birds
        birdIdx = src->popRepurposedBirdIndex();
        pBird& bird = market->getBirdSparse( birdIdx );
        bird->vendorPtr = dst; // update bird vendor owner
        bird->movements.increaseVcount();
        dst->addNewBirdIndex( birdIdx );
    }
    
    for ( uint i = 0, nnew = cts.second; i < nnew; ++i ) {      // move newly bought birds
        birdIdx = src->popNewBirdIndex();
        pBird& bird = market->getBirdSparse( birdIdx );
        bird->vendorPtr = dst; // update bird vendor owner
        bird->movements.increaseVcount();
        dst->addNewBirdIndex( birdIdx );
    }
    
    nv2->increaseBirdBoughtCount( nbirds );
    
    addVendor2VendorTransaction( nv1, nv2, nm, nv1->getContext()->getTradedBirdTypes()[0], nbirds ); // WARNING: specify types of bird; may not work in the future if vendor trades multiple species
}

void BirdTransferManager::_transferBirdsVendor2CommunityLog( NodeVendor *nv, NodeMarket *nm, uint nbirds, Random &randGen ) {
    
    uint birdIdx = 0;
    Vendor* vendor = nv->getContext();
    Market* market = nm->getContext();
    
    std::pair<uint, uint> cts = nv->getTransactionDetails( nbirds );
    
    for ( uint i = 0, nrep = cts.first; i < nrep; ++i ) {           // move repurposed birds
        
        birdIdx = vendor->popRepurposedBirdIndex();
        pBird bird = market->removeBird( birdIdx );
        birdExitTracker->birdExitsFromMarketOrVendor( bird, *clock );
        birdMngr->return_bird( std::move( bird ) );
        
    }
    
    for ( uint i = 0, nnew = cts.second; i < nnew; ++i ) {           // move new birds
        
        birdIdx = vendor->popNewBirdIndex();
        pBird bird = market->removeBird( birdIdx );
        birdExitTracker->birdExitsFromMarketOrVendor( bird, *clock );
        birdMngr->return_bird( std::move( bird ) );
        
    }
    
    addVendor2CommunityTransaction( nv, nm, nv->getContext()->getTradedBirdTypes()[0], nbirds ); // WARNING: specify types of bird; may not work in the future if vendor trades multiple species
    
}

void BirdTransferManager::transferBirdsFarm2Nothing( NodeFarm* nf, Crop& crop ) {
    
    while( !crop.empty() ) {
        
        uint idx = crop.removeLast( false, true ) ;
        pBird bird = nf->getUnderlyingSetting()->removeBird( idx ) ;
        birdExitTracker->birdRemovedFromFarm( bird, *clock ) ;
        birdMngr->return_bird( std::move( bird ) ) ;
    
    }
    
}


void BirdTransferManager::transferBirdsFarm2FieldExperimenter( NodeFarm* nf, NodeFieldExperimenter* nfe, const BirdType& bt, const uint& nbirds, Random& randGen ) {
    
    
    Farm* farm = nf->getContext();
    FieldExperimenter* experimenter = nfe->getContext();
    
    std::deque<Crop*>& crops = farm->getReadyCrops( bt );
    
    if ( crops.empty() )
        return; // no crops
    
    bool isDone = false;
    uint nbirds_moved = 0;
    
    while( !isDone ) { // loop over 'ready' crops
        
        Crop& crop = *( crops.front() );
        uint cropSize = crop.size();
        uint nbirdsTmp = std::min( cropSize, nbirds - nbirds_moved ); // maximum number of birds that can be moved
        
        for ( uint i = 0; i < nbirdsTmp; ++i ) {
            
            // remove and update chicken
            uint birdIdx = crop.removeLast();
            pBird bird = farm->removeBird( birdIdx );
            
            // record id of experiment chickens
            experimenter->registerFarmRecruitedBird( *bird ) ;
            
            // insert bird
            experimenter->insertBird( std::move( bird ) ) ;

                        
            nbirds_moved++;
            if ( nbirds_moved == nbirds )
                isDone = true;

        }
        
        // check if crop is empty
        if ( crop.empty() ) {

            crops.pop_front();
            completedCropTracker->processCompletedCrop( *nf, &crop, *clock );
            delete &crop;
            isDone = true; // stop if no crops at all
            
        }
        
    }
    
}


void BirdTransferManager::transferBirdsVendor2FieldExperimenter( NodeVendor* nv, NodeFieldExperimenter* nfe, NodeMarket* nm, const BirdType& bt, const uint& nbirds, Random& randGen ) {
    
    uint birdIdx = 0 ;
    Vendor* src             = nv->getContext() ;
    FieldExperimenter* dst  = nfe->getContext() ;
    Market* market          = nm->getContext() ;
        
    // compute number of birds to source from either repurposed or newly bought bird pools
    std::pair<uint, uint> cts = nv->getTransactionDetails( nbirds );
    
    for ( uint i = 0, nrep = cts.first; i < nrep; ++i ) {       // move repurposed birds
        
        birdIdx = src->popRepurposedBirdIndex() ;
        pBird& bird = market->getBirdSparse( birdIdx ) ;
        bird->vendorPtr = nullptr ; // update bird vendor owner
        dst->getBirdsMarketIndex().insert( birdIdx ) ;
        dst->registerMarketRecruitedBird( *bird ) ; // record id of experiment chickens
        
    }
    
    for ( uint i = 0, nnew = cts.second; i < nnew; ++i ) {      // move newly bought birds
        
        birdIdx = src->popNewBirdIndex();
        pBird& bird = market->getBirdSparse( birdIdx );
        bird->vendorPtr = nullptr; // update bird vendor owner
        bird->movements.increaseVcount() ;
        dst->getBirdsMarketIndex().insert( birdIdx ) ;
        dst->registerMarketRecruitedBird( *bird ) ; // record id of experiment chickens
        
    }
        
}
