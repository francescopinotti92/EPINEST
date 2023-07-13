//
//  sampling.cpp
//  CBM
//
//

#include "sampling.hpp"


// gives back (sparse) index of 'nBirds' randomly sampled infected birds.
// N.B. 'sampleLatent' defaults to 'false', so that only infectious can be sampled
// This is used in conjunction with homochronous sampling; does not modify chicken
std::vector<uint> sampleInfectedChickensFromSetting( HoldsBirds* setting, Random& randGen, const uint& nBirds, bool sampleLatent ) {
    
    std::vector<uint> idxsSampled = {} ;
    idxsSampled.reserve( nBirds ) ;
    
    std::vector<uint> idxsViable = {} ;
    auto& birds = setting->getBirds() ;
    
    if ( sampleLatent ) { // sample both infectious and latent birds
        
        uint nInfs = birds.size_infected() ;
        idxsViable.reserve( nInfs ) ;
        for ( auto it = birds.cbegin() ; it != birds.cend() ; ++it ) {
            if ( (*it)->isInfectious() or (*it)->isLatent() )
                idxsViable.push_back( (*it)->index ) ;
        }
        
    }
    else { // sample only infectious birds
        
        uint nViable = birds.size_infected() ;
        if ( nViable == 0 )
            return {} ; // no infectious birds
        idxsViable.reserve( nViable ) ;
        for ( auto it = birds.cbegin() ; it != birds.cendInf() ; ++it )
            idxsViable.push_back( (*it)->index ) ;
            
        
    }
    
    uint nViable = static_cast<uint>( idxsViable.size() ) ;
    if ( nViable <= nBirds )
        return idxsViable ; // get all viable chickens (there are less than requested)
    else {

        randGen.shuffleVector( idxsViable ) ;
        idxsSampled.reserve( nBirds ) ;
        for ( uint i = 0 ; i < nBirds ; ++i ) {
            
            idxsSampled.push_back( idxsViable[i] ) ;
            
        }
        
    }
    
    return idxsSampled ;
    
}
