//
//  AreaManager.cpp
//  CBM
//
//

#include "AreaManager.hpp"

//==== AreaManager ====//

// constructor

AreaManager::AreaManager( std::vector<BirdType> bts, uint Na_, uint Nm_ ): Na( Na_ ), Nm( Nm_ ), farmProduction() {

    nTotalFarmsByArea = std::vector<uint>( Na, 0 ) ;

    for ( BirdType bt: bts ) {
        
        tradingFarms.insert( { bt, std::vector<std::vector<NodeFarm*>>( Na, std::vector<NodeFarm*>() ) } );
        P_a.insert( { bt, std::vector<double>( Na, 0. ) } );
        P_al.insert( { bt, std::vector<std::vector<double>>( Na, std::vector<double>( Nm, 0. ) ) } );
        Q_al.insert( { bt, std::vector<std::vector<double>>( Na, std::vector<double>( Nm, 0. ) ) } );
        
        std::vector<double> tmpVec( Na, 1./Na );
        farmProduction[bt] = DiscreteDistribution( tmpVec ) ;
    }
    
    neighboringAreasNetwork = net_utils::uw_adjacency_list( Na, 10 );

}


// Re-compute production distribution by area

void AreaManager::updateFarmProductionDistribution() {
    for ( auto& elem: farmProduction ) {
        BirdType bt = elem.first;
        auto& distr = elem.second;
        std::vector<double> tmp( Na, 0. );
        double Z = 0.;
        for ( uint a = 0; a < Na; ++a ) {
            auto& farms = tradingFarms[bt][a];
            for ( auto& nf: farms ) {
                uint birds4Sale = nf->getSizeAvailBirds( bt );
                tmp[a] += birds4Sale;
                Z += birds4Sale;
            }
        }
        
        if ( Z > 0. ) { // normalize weights and set distr
            for ( double& val: tmp )
                val /= Z;
            distr = DiscreteDistribution( tmp );
        }
        else {  // no active farms: set to uniform distribution
            tmp = std::vector<double>( Na, 1. / Na );
            distr = DiscreteDistribution( tmp );
        }
    }
}


// reset

void AreaManager::reset() {
    // remove active farms
    for ( auto& elem: tradingFarms ) {
        for ( uint a = 0; a < Na; ++a ) {
            (elem.second)[a].clear();
        }
    }
    
    // re-compute distribution
    for ( auto& elem: farmProduction ) {
        elem.second = DiscreteDistribution( std::vector<double>( Na, 1. / Na ) );
    }

}

// add pair of neighboring areas

void AreaManager::addAreaNbr( const uint &a1, const uint &a2 ) {
    neighboringAreasNetwork.addNbr( a1, a2 );
    neighboringAreasNetwork.addNbr( a2, a1 );
}

// register farms that are trading

void AreaManager::registerTradingFarm( NodeFarm* nf ) {
    const uint& areaId = nf->getAreaId();
    const std::vector<BirdType>& bts = nf->getContext()->getTradedBirdTypes();
    
    for ( const auto& bt: bts ) {
        auto& tmp = tradingFarms[bt][areaId];
        if ( std::find( tmp.begin(), tmp.end(), nf ) == tmp.end() )
            tmp.push_back( nf );
    }
    
    //
    /*
    if ( areaId == 32 ) {
        std::cout << "register farm " << nf->getContext()->getId() << std::endl;
        for ( auto nf: tradingFarms[BirdType::broiler][areaId] )
            std::cout << nf->getContext()->getId() << " ";
        std::cout << std::endl;
    }
     */
}

// unregister farms that are not trading anymore
void AreaManager::unregisterTradingFarm( NodeFarm *nf ) {
    const uint& areaId = nf->getAreaId();
    const std::vector<BirdType>& bts = nf->getContext()->getTradedBirdTypes();
    for ( const auto& bt: bts ) {
        auto& tmp = tradingFarms[bt][areaId];
        /*
        for ( auto it = tmp.begin(); it != tmp.end(); ) {
            if ( *it == nf ) {
                std::iter_swap( it, tmp.end() - 1 );
                tmp.pop_back();
                break;
            }
            ++it;
        }
         */
        // remove element, preserving farm order according to time farm started rollout
        tmp.erase(std::remove( std::begin( tmp ), std::end( tmp ), nf ), std::end( tmp ) );
        //auto pr = std::equal_range( std::begin( tmp ), std::end( tmp ), nf );
        //tmp.erase( pr.first, pr.second );
    }
}

std::vector<uint> AreaManager::getNeighboringAreas( const uint &areaId, const uint &nAreas, Random& randGen ) {
    
    if ( nAreas < 1 ) {
        return {}; // nothing to return
    }
    
    std::vector<uint> areasIds = {};
    areasIds.reserve( nAreas );
    
    uint nAreasFound = 0;
    
    // first loop over the neighbors of the focal region
    auto& nbrs_a = neighboringAreasNetwork[areaId];
    randGen.shuffleVector( nbrs_a );
    for ( uint nbr: nbrs_a ) {
        // check if id there or not
        if ( ( nbr != areaId ) and ( std::find( areasIds.begin(), areasIds.end(), nbr ) == areasIds.end() ) ) {
            areasIds.push_back( nbr );
            nAreasFound++;
            if ( nAreasFound == nAreas )
                return areasIds;
        }
    }
    
    // if not enough, look among the neighboring areas' neighboring areas
    auto areaIt = areasIds.begin();
    while ( areaIt != areasIds.end() ) {
        auto& nbrs_a = neighboringAreasNetwork[ *areaIt ];
        randGen.shuffleVector( nbrs_a );
        for ( uint nbr: nbrs_a ) {
            if ( ( nbr != areaId ) and ( std::find( areasIds.begin(), areasIds.end(), nbr ) != areasIds.end() ) ) {
                areasIds.push_back( nbr );
                nAreasFound++;
                if ( nAreasFound == nAreas )
                    return areasIds;
            }
        }
        ++areaIt; // at some point no more areas can be added
    }
        
    return areasIds;
}

uint AreaManager::getRandomAreaUsingFarmProduction( const BirdType& bt, Random& randGen ) {
    auto& distr = farmProduction[bt];
    return randGen.getCustomDiscrete( distr );
}



void AreaManager::set_P_a( const BirdType &bt, const std::vector<double> &N_a ) {
    
    double Z = 0.;
    Z = std::accumulate( std::begin( N_a ), std::end( N_a ), 0. );
    
    assert( N_a.size() == Na );
    assert( Z > 0. );

    this->P_a[bt] = N_a;
        
    for ( uint a = 0; a < Na; ++a ) {
        this->P_a[bt][a] = this->P_a[bt][a] / Z;
    }
    
}

void AreaManager::set_P_al( const BirdType& bt, const std::vector<std::vector<double>>& P_al_ ) {
    
    // check format
    assert( P_al_.size() == Na );
    for ( uint a = 0; a < Na; ++a ) {
        assert( P_al_[a].size() == Nm );
    }
    
    // assign P_al
    this->P_al[bt] = P_al_;
    
    // assign Q_al = P_a * P_al
    for ( uint a = 0; a < Na; ++a ) {
        double pa = this->P_a[bt][a];
        for ( uint l = 0; l < Nm; ++l ) {
            this->Q_al[bt][a][l] = this->P_al[bt][a][l] * pa;
        }
    }
    
}

std::vector<double>& AreaManager::get_P_a( const BirdType &bt ) {
    return P_a[bt];
}

std::vector<double>& AreaManager::get_P_al( const BirdType& bt, const uint& a ) {
    return P_al[bt][a];
}


// 'average' P_al over areas visited by a single middleman
DiscreteDistribution AreaManager::synthesizeMarketChoiceDistribution( const BirdType &bt, const std::vector<uint> &areasVec ) {
    
    /* // old code
    std::vector<double> weights( Nm, 0. );
    
    double Z = 0.; // normalizing constant
    std::vector<double>& p_a_tmp = P_a[bt];
    for ( const uint& a: areasVec ) {
        Z += p_a_tmp[a];
        std::vector<double>& q_al_tmp = Q_al[bt][a];
        for ( uint l = 0; l < Nm; ++l ) {
            weights[l] += q_al_tmp[l];
        }
    }
    
    // finally divide by normalizing constant
    for ( uint l = 0; l < Nm; ++l ) {
        weights[l] = weights[l] / Z;
    }*/
    
    double Z = 0.; // normalizing constant
    std::vector<double> weights( Nm, 0. );
    
    for ( const uint& a : areasVec ) {
        auto& farms = tradingFarms[bt][a];
        double O_a_tmp = 0 ;
        for ( auto& nf: farms ) {
            O_a_tmp += nf->getSizeAvailBirds( bt );
        }
        
        Z += O_a_tmp ;
        std::vector<double>& p_al_tmp = P_al[bt][a] ;
        for ( uint l = 0; l < Nm; ++l ) {
            weights[l] += O_a_tmp * p_al_tmp[l];
        }

        
    }
    
    if ( Z == 0 ) // no production : return invalid distribution
        return getInvalidDiscreteDistribution() ;
    
    // finally divide by normalizing constant
    for ( uint l = 0; l < Nm; ++l ) {
        weights[l] = weights[l] / Z;
    }
    
        
    return DiscreteDistribution( weights );
    
}

// compute P_al, but assuming that l takes only a subset of possible values
DiscreteDistribution AreaManager::synthesize_P_al_Subset( const BirdType& bt, const uint areaId, const std::vector<uint>& marketsVec ) {
    std::vector<double> res = std::vector<double>( marketsVec.size(), 0. ); // vector storing results
    double Z = 0.;
    auto& P_al_tmp = this->get_P_al( bt, areaId );
    for ( uint il = 0; il < marketsVec.size(); ++il ) {
        uint l = marketsVec[il];
        double val = P_al_tmp[l];
        res[il] = val;
        Z += val;
    }
    
    // normalize distribution
    for ( double& val: res )
        val /= Z;
    
    return DiscreteDistribution( res );
}


// compute P_aj(l), i.e. the relative probability to end up in region a, starting from a market l, and given a particular subset of regions
std::vector<double> AreaManager::synthesize_P_ajl( const BirdType& bt, const uint marketId, const std::vector<uint>& areasVec ) {
    
    /* // old code
    double Z = 0.; // normalizing constant
    std::vector<double> P_ajl = std::vector<double>( areasVec.size(), 0. ); // vector storing results
    
    auto& q_al_tmp = this->Q_al[bt]; // TIP: would be better to use the transpose

    // put piecese together
    for ( uint ia = 0; ia < areasVec.size(); ++ia ) {
        uint a = areasVec[ia];
        double val = q_al_tmp[a][marketId];
        P_ajl[ia] = val;
        Z += val;
    }
    
    // normalize
    for ( double& val: P_ajl )
        val /= Z;
     
    */
    
    double Z = 0.; // normalizing constant
    std::vector<double> P_ajl = std::vector<double>( areasVec.size(), 0. ); // vector storing results
    auto& p_al_tmp = this->P_al[bt] ;
    for ( uint ia = 0; ia < areasVec.size(); ++ia ) {
        uint a = areasVec[ia];
        
        auto& farms = tradingFarms[bt][a];
        double O_a_tmp = 0 ;
        for ( auto& nf: farms ) {
            O_a_tmp += nf->getSizeAvailBirds( bt ) ;
        }
        
        double val = O_a_tmp * p_al_tmp[a][marketId] ;
        P_ajl[ia] = val ;
        Z += val ;
    }
    
    for ( double& val: P_ajl )
        val /= Z;
    
    return P_ajl;
}

std::vector<NodeFarm*>& AreaManager::getTradingFarmsArea( const BirdType& bt, const uint& a ) {
    return tradingFarms[bt][a];
}
