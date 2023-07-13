//
//  crop.cpp
//  CBM
//
//

#include "crop.hpp"


Crop::Crop( const uint& size ): pool( size ), dayCreate( 0 ), tCreate( 0 ), nBirdsInitial( 0 ), nInfectionsCumulative( 0 ), nBirdsDead( 0 ), nBirdsForceRemove( 0 ) {};
Crop::Crop( const uint& size, const Clock& clock ): pool( size ), dayCreate( clock.getCurrDay() ), tCreate( clock.getCurrTime() ), nBirdsInitial( 0 ), nInfectionsCumulative( 0 ), nBirdsDead( 0 ), nBirdsForceRemove( 0 ) {};

void Crop::insert( const uint& id ) {

    ++nBirdsInitial ;
    pool.insert( id ) ;

}

void Crop::remove( const uint& id, const bool& dead, const bool& forced ) {
    
    pool.remove( id ) ;
    
    if ( dead ) // !!! implement this
        ++nBirdsDead;
    if ( forced )
        ++nBirdsForceRemove;
    
}

bool Crop::empty() {
    
    return pool.empty() ;
    
}

uint Crop::size() {
    
    return pool.size() ;
    
}

uint Crop::removeLast( const bool& dead, const bool& forced ) {
    
    if ( dead ) // !!! implement this
        ++nBirdsDead;
    if ( forced )
        ++nBirdsForceRemove;
    
    return pool.removeLast() ;
    
}
