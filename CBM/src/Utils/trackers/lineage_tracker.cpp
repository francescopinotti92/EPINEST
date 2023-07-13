//
//  lineage_tracker.cpp
//  CBM
//
//

#include "lineage_tracker.hpp"


//====== NewMarketLineagesTracker ======//

LineageData::LineageData(): lineageID( 0 ), dayIntro( 0 ), size( 0 ) {} ;
LineageData::LineageData( const uint& lineageID_, Clock& clock ): lineageID( lineageID_ ), dayIntro( clock.getCurrDay() ), size( 0 ) {} ;
    

NewMarketLineagesTracker::NewMarketLineagesTracker( NamedOfstream outFile, const uint& tmin, const uint& tmax, const uint& minClusterSize ): BaseCrossSectionalTracker( outFile, 1, { MARKET_OPENING_HOUR + 1 }, tmin, tmax ), nCreatedLineages( 0 ), initialised( false ), minClusterSize( minClusterSize ) {
    
    /*
     Note: checks which strains are present at MARKET_OPENING_HOUR + 1, since this allows
     to check infected chickens after they are deposited in markets.
     */
    
    checkTime = std::bind( &NewMarketLineagesTracker::checkTimeFixedWindow, this, std::placeholders::_1 );
    
    bird2lineage.reserve( 10000 ) ;
    existingLineages.reserve( 10000 ) ;
    
    uint maxDays = (uint)( ( tmax - tmin ) / 24 ) + 1;
    
    lineageDurationHist = std::vector<uint>( maxDays, 0 ) ;
    if ( minClusterSize > 0 )
        lineageDurationMinSizeHist = std::vector<uint>( maxDays, 0 ) ;
    
    
}

void NewMarketLineagesTracker::reset( bool updateStream ) {
    
    // print histogram
    
    auto& out = outFile.getStream();
    
    if ( minClusterSize == 0 ) { // simple dump
        
        out << "dt,count\n" ;
        for ( int i = 0 ; i < static_cast<uint>( lineageDurationHist.size() ); ++i ) {
            out << i << "," << lineageDurationHist[i] << "\n";
        }
        
    }
    else { // print also histogram conditional on minimum cluster size
        
        out << "dt,count,countmin\n" ;
        for ( int i = 0 ; i < static_cast<uint>( lineageDurationHist.size() ); ++i ) {
            out << i << "," << lineageDurationHist[i] << "," << lineageDurationMinSizeHist[i] << "\n";
        }
        
    }
    
    /*for ( auto& elem : existingLineages ) {
        
        std::cout << "lng intro t: " << elem.second.dayIntro << " size: " << elem.second.size << std::endl ;
        
    }*/
    
    
    // reset
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    initialised = false ;
    bird2lineage.clear() ;
    existingLineages.clear() ;
    for ( uint& el : lineageDurationHist ) el = 0 ;
    for ( uint& el : lineageDurationMinSizeHist ) el = 0 ;

    
}

void NewMarketLineagesTracker::collectInfo( WorldSimulator* simulator ) {
    
    if ( !checkTime( simulator->getClock() ) )
        return ;
    
    //== loop over chickens in markets to detect new introductions
    
    std::unordered_map<BirdID, uint, hashBirdID> bird2lineageNEW ;
    std::unordered_set<uint> lineagesDetected ;
    lineagesDetected.reserve( 2000 ) ;
    
    auto& markets = simulator->getMarketList() ;
    for ( NodeMarket* market : markets ) {
        
        auto& birds = market->getContext()->getBirds();

        for ( auto it = birds.cbegin(); it < birds.cend(); ++it ) {
            
            // check if bird is infected OR latent
            if ( (*it)->isInfectious() or (*it)->isLatent() ) {
                
                // check if bird is already in list of
                BirdID birdID = (*it)->id ;
                auto it1 = bird2lineage.find( birdID ) ;
                if ( it1 != bird2lineage.end() ) { // old lineage
                    
                    uint lng = it1->second ; // get lineage from infector
                    bird2lineageNEW[birdID] = lng ;
                    lineagesDetected.insert( lng ) ;
                
                }
                else { // new lineage
                    
                    uint lng = nCreatedLineages ;
                    bird2lineageNEW[birdID] = lng ;
                    lineagesDetected.insert( lng ) ;
                    existingLineages[lng] = LineageData( lng, simulator->getClock() ) ;
                    ++nCreatedLineages ;
                    
                }
                
            }
            
        }
        
    }
    
    //std::cout << "lineages found " << lineagesDetected.size() << "\n";
    
    //== trim extinct lineages and get lineage duration
    
    uint currDay = simulator->getClock().getCurrDay() ;

    if ( initialised ) { // record only if initialised
        for ( auto it = existingLineages.cbegin(); it != existingLineages.cend() ; ) {
          
            if ( lineagesDetected.find( it->first ) == lineagesDetected.end() ) { // lineage died out
                
                uint lngDuration = currDay - (it->second).dayIntro ;
                ++lineageDurationHist[ lngDuration ] ; // update histogram
                if ( minClusterSize > 0 ) { // update histogram conditional on minimum size
                    if ( ( it->second ).size >= minClusterSize )
                        ++lineageDurationMinSizeHist[ lngDuration ] ;
                }
                
                existingLineages.erase( it++ ) ; // remove lineage
          
            }
            else {
                ++it;
            }
            
        }
    }
    else
        initialised = true ; // now it is initialised
    
    std::swap( bird2lineage, bird2lineageNEW ) ; // swap old list of infected chickens
    
}

void NewMarketLineagesTracker::addInfectionEvent( Bird& infector, Bird& infectee, const Clock& clock ) {
    
    BirdID birdID1 = infector.id ;
    
    auto it = bird2lineage.find( birdID1 ) ;
    if ( it != bird2lineage.end() ) {
        
        BirdID birdID2 = infectee.id ;
        uint lng = bird2lineage[birdID1] ;
        bird2lineage[birdID2] = lng ;
        ++existingLineages[lng].size ; 
        
        
    }
    
}

//====== NewMarketLineagesWIWHelperTracker ======//


NewMarketLineagesWIWHelperTracker::NewMarketLineagesWIWHelperTracker( std::shared_ptr<NewMarketLineagesTracker> lngTracker_ ): BaseInfectionTracker( NamedOfstream() ) {
    
    lngTracker = lngTracker_ ;
    
}

void NewMarketLineagesWIWHelperTracker::reset( bool updateStream ) {
    
    outFile.close() ;
    
}

void NewMarketLineagesWIWHelperTracker::addInfectionEvent( Bird &infector, Bird &infectee, const StrainIdType &strain, const Clock &clock ) {
    
    uint tCurr = clock.getCurrTime() ;
    if ( tCurr < tStartTracking )
        return ; // too early
    
    if ( tCurr > tStopTracking )
        return ; // too late
    
    NodeType settingType = infectee.owner->getSettingType() ;
    if ( settingType != NodeType::market )
        return ; // infection not happening in market
    

    lngTracker->addInfectionEvent( infector, infectee, clock ) ;
    
}

void NewMarketLineagesWIWHelperTracker::addEnvironmentInfectionEvent( const BirdID &infectorID, Bird &infectee, const StrainIdType &strain, const Clock &clock ) {
    
    // NOT IMPLEMENTED
    
}

void NewMarketLineagesWIWHelperTracker::addExternalInfectionEvent( Bird &infectee, const StrainIdType &strain, const Clock &clock ) {
    
    // NOT IMPLEMENTED
    
}

