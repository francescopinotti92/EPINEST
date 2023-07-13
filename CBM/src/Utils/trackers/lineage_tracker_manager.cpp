//
//  lineage_tracker_manager.cpp
//  CBM
//
//

#include "lineage_tracker_manager.hpp"


LineageTrackerManager::LineageTrackerManager( const uint& S, const uint& maxStrains ): S( S )  {
    
    // create trees
    for ( uint s = 0 ; s < S; ++s )
        trees.push_back( new LineageTree<BirdID,DataLineage,hashBirdID>() ) ;
    
    
    // Enumerate combinations of strains for each infectious state
    strainList = {} ;
    
    // add fully naive state, indexed by 0
    strainList[0] = {} ;
    
    for ( int k = 1; k <= maxStrains; ++k ) {

        std::string inf_state( k, 1 ); // k leading 1's
        inf_state.resize( S, 0 ); // N-K trailing 0's
         
        // print integers and permute bitmask
        do {
            
            StateType r = 0 ;
            std::vector<StrainIdType> strainsTmp = {} ;
            strainsTmp.reserve( maxStrains ) ;

            for ( int i = 0; i < S; ++i ) {
                if ( inf_state[i] ) {
                    r += ( constants::ONE64 << i ) ;
                    strainsTmp.push_back( i ) ;
                }
            }
            
            strainList[r] = strainsTmp ;
            
        } while ( std::prev_permutation( inf_state.begin(), inf_state.end()) ) ;
    
    }
    
}

void LineageTrackerManager::reset() {
    
    for ( uint s = 0 ; s < S ; ++s ) {
     
        trees[s]->reset() ; // do not delete tree
        
    }
    
       
}

void LineageTrackerManager::addInfectionEvent( const Bird& infectee, const Bird& infector, const StrainIdType& strain, const Clock& clock ) {
    
    BirdID birdIDinfectee = infectee.getID() ;
    BirdID birdIDInfector = infector.getID() ;
    DataLineage data = DataLineage( ( infectee.owner )->getStringID() ) ;
    
    trees[strain]->addExtantLineage( clock.getCurrTime(), birdIDinfectee, data, birdIDInfector ) ;
    
}


void LineageTrackerManager::addExternalInfectionEvent( const Bird &infectee, const StrainIdType &strain, const Clock &clock ) {
    
    BirdID birdIDinfectee = infectee.getID() ;
    DataLineage data = DataLineage( ( infectee.owner )->getStringID() ) ;
    
    trees[strain]->addExtantLineageExternal( clock.getCurrTime(), birdIDinfectee, data ) ;
    
}

void LineageTrackerManager::addRecoveryEvent( const Bird& bird, const StrainIdType& strain ) {
    
    BirdID id = bird.getID() ;
    trees[strain]->removeExtantLineage( id ) ;

    
}

void LineageTrackerManager::addRemovalEvent( const Bird& bird ) {
    
    StateType state = bird.infState ;
    
    if ( state == 0 )
        return ; // bird is not carrying any strain
    
    BirdID id = bird.getID() ;
    auto& strains = strainList[state] ;
    
    for ( auto& strain : strains )
        trees[strain]->removeExtantLineage( id ) ;
    
}

uint LineageTrackerManager::getStrainNumber() {
    
    return S ;
    
}

LineageTree<BirdID, DataLineage, hashBirdID>* LineageTrackerManager::getTree( StrainIdType& strain ) {
    
    return trees[strain] ;
    
}

std::vector<StrainIdType>& LineageTrackerManager::getStrainList( const StateType& state ) {
    
    return strainList[ state ] ;
    
}

