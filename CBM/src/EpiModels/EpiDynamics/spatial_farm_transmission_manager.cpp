//
//  spatial_farm_transmission_manager.cpp
//  CBM
//
//

#include "spatial_farm_transmission_manager.hpp"


//====== EpiFarmCell ======//

EpiFarmCell::EpiFarmCell(): ID( 0 ), cell(), maxSusc( 0. ), farms(), suscFarms( 1 ), infectedFarms( 1 ) {};

EpiFarmCell::EpiFarmCell( const uint& ID_, const Geom::Cell& cell_ ): ID( ID_ ), cell( cell_ ), maxSusc( 0. ), farms(), suscFarms( 1 ), infectedFarms( 1 ) {};

void EpiFarmCell::reset() {
    
    suscFarms.clear();
    
}

bool EpiFarmCell::containsFarm( NodeFarm* nf ) {
    
    return cell.contains_point( nf->getPos() );
    
}

void EpiFarmCell::registerFarm( NodeFarm* nf ) {
    
    if ( containsFarm( nf ) )
        farms.push_back( nf );
    
}

void EpiFarmCell::checkFarms() {
    
    if ( suscFarms.capacity() < farms.size() )
        suscFarms.increaseCapacity( static_cast<int>( farms.size() ) );
    
    if ( infectedFarms.capacity() < farms.size() )
        infectedFarms.increaseCapacity( static_cast<int>( farms.size() ) );
    
    suscFarms.clear();
    infectedFarms.clear();
    
    for ( NodeFarm* nf : farms ) {
        
        if ( nf->getSizeBirds() > 0 ) {
            
            suscFarms.insert( nf );
            if ( nf->getSizeInfectedBirds() > 0 )
                infectedFarms.insert( nf );
        
        }
        
    }
    
}

// computes max{N^expon}, where N is the average size of the largest farm
void EpiFarmCell::computeMaxSusc( const double& expon ) {

    maxSusc = 0.;
    for ( NodeFarm* nf : farms ) {
    
        double susceptibility = pow( nf->getAverageFarmSize(), expon );
        if ( susceptibility > maxSusc )
            maxSusc = susceptibility;
        
    }
    
}


//====== SpatialFarmTransmissionManager ======//

SpatialFarmTransmissionManager::SpatialFarmTransmissionManager( const double& lam, std::vector<NodeFarm*>* farmVec_, std::function<double(const double&)> kernel, const double& expon_a_, const double& expon_b_, BaseInfectionTracker* tracker_ ): farmVec( farmVec_ ), infectedCells( 1 ), expon_a( expon_a_ ), expon_b( expon_b_ ) {
    
    // set up infection tracker
    
    tracker = tracker_;
    if ( tracker == nullptr ) {
        this->applyTransmission2Settings = std::bind( &sparkInfectionTwoSettings, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5 );
    }
    else {
        this->applyTransmission2Settings = std::bind( sparkInfectionTwoSettingsWithTracking, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, tracker );
    }
    
    // set up an adaptive grid
    
    double xmin = std::numeric_limits<double>::max();
    double ymin = std::numeric_limits<double>::max();
    double xmax = std::numeric_limits<double>::min();
    double ymax = std::numeric_limits<double>::min();
    
    for ( NodeFarm* nf : *farmVec ) {
        
        Geom::Point pos = nf->getPos();
        double x = pos.getX();
        double y = pos.getY();
        
        if ( x < xmin )
            xmin = x;
        
        if ( x > xmax )
            xmax = x;
        
        if ( y < ymin )
            ymin = y;
        
        if ( y > ymax )
            ymax = y;
        
    }
    
    xmin = 0.999 * xmin;
    xmax = 1.001 * xmax;
    ymin = 0.999 * ymin;
    ymax = 1.001 * ymax;

    double side = std::max( xmax - xmin, ymax - ymin );
    Geom::pCell boundingCell = std::make_shared<Geom::Cell>( xmin, ymin, side );
    
    // add farm locations to bounding cell
    for ( NodeFarm* nf : *farmVec ) {
        
        Geom::Point pos = nf->getPos();
        assert( boundingCell->contains_point( pos ) );
        boundingCell->addPoint( pos );
        
    }
    
    std::vector<Geom::pCell> grid = {};
    grid.reserve( 100 );
    
    splitCellsAdaptive( grid, boundingCell, lam );
    
    // set up epiFarmCells
    nCells = static_cast<uint>( grid.size() );
    epiCells = {};
    epiCells.reserve(  nCells );
    
    uint tmpID = 0;
    for ( Geom::pCell cell : grid ) {
        
        epiCells.emplace_back( tmpID, *cell );
        auto& epiCell = epiCells[tmpID];
        for ( NodeFarm* nf : *farmVec )
            epiCell.registerFarm( nf );
        
        epiCell.computeMaxSusc( expon_b );
        
        ++tmpID;
        
    }
    
    // bind kernel function
    evalKernel = std::bind( kernel, std::placeholders::_1 );
    
    // compute K(d) between pairs of farms
    Kcells = {};
    Kcells.resize( nCells * nCells );
    for ( int i = 0; i < nCells; ++i ) {
        
        Kcells[ i * nCells + i ] = 0.;
        
        for ( int j = i + 1; j < nCells; ++j ) {

            double dist = epiCells[i].cell.computeDistanceFromCell( epiCells[j].cell );
            double K = evalKernel( dist );
            
            Kcells[ i * nCells + j ] = K;
            Kcells[ j * nCells + i ] = K;

        }
        
    }
    
    // reserve enough space for infectedCells
    infectedCells.increaseCapacity( nCells );
    
}

void SpatialFarmTransmissionManager::reset() {
    
    infectedCells.clear();
    
    for ( auto& epiCell : epiCells )
        epiCell.reset();
    
}


void SpatialFarmTransmissionManager::doTransmissionStep( const Clock& clock, Random& randGen, BaseCompartmentalModel* compModel ) {
    
    // update lists of susceptibles and infected farms
    
    infectedCells.clear();
    
    for ( auto& epiCell : epiCells ) {
        epiCell.checkFarms();
        if ( epiCell.hasInfectedFarms() )
            infectedCells.insert( &epiCell );
    }
        
    // loop over infected farms
    
    for ( auto it = infectedCells.begin(), end = infectedCells.end(); it < end; ++it ) {
        
        auto& sourceCell = *it;
        auto& infectedFarms = sourceCell->getInfectedFarms();
        
        for ( auto itnf = infectedFarms.begin(), endnf = infectedFarms.end(); itnf < endnf; ++itnf ) {
            
            NodeFarm* src = *itnf;
            
            for ( auto& targetCell : epiCells  ) {
                
                if ( &targetCell != sourceCell ) {
                    
                    doConditionalEntryTransmission( src, sourceCell, &targetCell, randGen, clock, compModel );
                    
                }
                else {
                    
                    doPairwiseTransmissionWithinCell( src, &targetCell, randGen, clock, compModel );
                    
                }
                
            }
            
        }
        
    }
    
}


void SpatialFarmTransmissionManager::doPairwiseTransmissionWithinCell( NodeFarm* src, EpiFarmCell* targetCell, Random& randGen, const Clock& clock, BaseCompartmentalModel* compModel ) {
    
    if ( !targetCell->containsFarm( src ) )
        return ; // farm is not found in targetCell
    
    auto& suscFarms = targetCell->getSusceptibleFarms();
    
    for ( auto itnf = suscFarms.begin(), endnf = suscFarms.end(); itnf < endnf; ++itnf ) {
        
        NodeFarm* tgt = *itnf; // target farm
        
        if ( src == tgt )
            continue; // same farm, pass
        
        double Na = pow( src->getAverageFarmSize(), expon_a );
        double Nb = pow( tgt->getAverageFarmSize(), expon_b );
        double dist = Geom::euclidean_distance( src->getPos(), tgt->getPos() );
        double kernel = evalKernel( dist );
        double p_inf = 1. - exp( -Na * Nb * kernel );
        
        if ( randGen.getBool( p_inf ) ) {
            
            applyTransmission2Settings( *( src->getUnderlyingSetting() ), *( tgt->getUnderlyingSetting() ), clock, *compModel, randGen );
            
        }
        
    }

}
void SpatialFarmTransmissionManager::doConditionalEntryTransmission( NodeFarm* src, EpiFarmCell* sourceCell, EpiFarmCell* targetCell, Random& randGen, const Clock& clock, BaseCompartmentalModel* compModel ) {
    
    if ( sourceCell == targetCell )
        return ; // farm is in targetCell
    
    double Na = pow( src->getAverageFarmSize(), expon_a );

    auto& suscFarms = targetCell->getSusceptibleFarms();
    uint nsusc = suscFarms.size();
    
    double v = 1. - exp( - Na * ( targetCell->getMaxSusc() ) * getKcells( sourceCell, targetCell )  );
    double w = 1. - pow( 1. - v, nsusc );
    
    if ( randGen.getBool( w ) ) {
        uint j = 0;
        double Nb;
        double dist;
        double kernel;
        double r = 0;
        double q = 0;
        double p = 0;
        bool flag = false;
        
        for ( auto& tgt : suscFarms ) {
            
            if ( flag )
                q = 1.;
            else
                q = 1. - pow( 1. - v, nsusc - j );
            
            if ( r < v / q ) {
                flag = true;
                Nb = pow( tgt->getAverageFarmSize(), expon_b );
                dist = Geom::euclidean_distance( src->getPos(), tgt->getPos() );
                kernel = evalKernel( dist );
                p = 1. - exp( -Na * Nb * kernel );
                
                if ( r < p / q ) {
                   
                    applyTransmission2Settings( *( src->getUnderlyingSetting() ), *( tgt->getUnderlyingSetting() ), clock, *compModel, randGen );
                
                }
                
            }
        
            ++j;
        }
        
    }
    
}

double SpatialFarmTransmissionManager::getKcells( EpiFarmCell* c1, EpiFarmCell* c2 ) {
    
    return Kcells[ nCells * ( c1->ID ) + c2->ID ];
    
}
