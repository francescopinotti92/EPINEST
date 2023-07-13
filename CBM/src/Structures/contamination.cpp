//
//  contamination.cpp
//  CBM
//
//

#include <stdio.h>
#include "contamination.hpp"

//====== ContEnv ======//
/*
 This class manages strains present in the environment as well as their amount.
 Does not track identity of birds shedding contaminated material.
 */

// constructor
ContEnv::ContEnv( const uint& nStrains ): BaseContEnv() {
    nUnits = std::vector<uint>( nStrains, 0 );
    currentStrains = {};
    totalUnits = 0;
}

// destructor
ContEnv::~ContEnv() {};

// clears environment from contaminated material: eliminats all contaminated units
void ContEnv::clear() {
    for ( uint& elem: nUnits )
        elem = 0;
    totalUnits = 0;
    currentStrains.clear();
}

// adds 'amount' new contaminated units of 'strain'
void ContEnv::addUnits( const BirdID& birdID, const StrainIdType& strain, const uint& amount ) {
    if ( nUnits[strain] == 0 )
        currentStrains.push_back( strain );
    nUnits[ strain ] += amount;
    totalUnits += amount;
}

// remove contaminated material
void ContEnv::removeUnits( Random& randGen, const double& decayRate ) {
    for ( auto it = currentStrains.begin(); it != currentStrains.end(); ) {
        
        uint amount =   randGen.getBinom( decayRate, nUnits[*it] );     // draw number of removed units
        nUnits[*it] -=  amount;
        totalUnits  -=  amount;
    
        if (nUnits[*it] == 0)                                           // if hits 0, remove strain from list of currently present strains
            currentStrains.erase( it );
        else
            ++it;
    }
}

// get total number of contaminated units
uint ContEnv::getTotalUnits() const {
    return totalUnits;
}

// get total number of contaminated units relative to 'strain'
uint ContEnv::getTotalUnits( const StrainIdType& strain ) const {
    return nUnits[strain];
}

// get BirdID corresponding to a random contaminated unit of 'strain'
BirdID ContEnv::getRandomContaminatedUnitID( const StrainIdType &strain, Random& randGen ) {
    return BirdID( UINT_MAX, 0 );
}

//====== ContEnvDetailed ======//
/*
 This class manages strains present in the environment as well as their amount.
 Tracks identity of birds shedding contaminated material.
 */

// constructor
ContEnvDetailed::ContEnvDetailed( const uint& nStrains, const uint& initialCapacity ): BaseContEnv() {
    units = std::vector<std::vector<BirdID>>( nStrains, std::vector<BirdID>() );
    
    for ( uint strain = 0; strain < nStrains; ++strain )
        units[strain].reserve( initialCapacity );

    currentStrains = {};
    totalUnits = 0;
}

// destructor
ContEnvDetailed::~ContEnvDetailed() {};

// clears environment from contaminated material: eliminats all contaminated units
void ContEnvDetailed::clear() {
    for ( auto& elem: units )
        elem.clear();
    totalUnits = 0;
    currentStrains.clear();
}

// adds 'amount' new contaminated units of 'strain'
void ContEnvDetailed::addUnits( const BirdID& birdID, const StrainIdType& strain, const uint& amount ) {
    
    auto& units_tmp = units[strain];
    
    if ( units_tmp.size() == 0 )
        currentStrains.push_back( strain );
    
    units_tmp.insert( units_tmp.end(), amount, birdID );
    totalUnits += amount;
}

// remove contaminated material
void ContEnvDetailed::removeUnits( Random& randGen, const double& decayRate ) {
    
    for ( auto it = currentStrains.begin(); it != currentStrains.end(); ) {
        uint nUnits =   static_cast<uint>( units[*it].size() );
        uint amount =   randGen.getBinom( decayRate, nUnits );          // draw number of removed units

        randGen.removeRandomElementsFromVec( units[*it], amount );      // remove contaminated units
        totalUnits -= amount;
    
        if ( units[*it].empty() )                                       // if empty, remove strain from list of currently present strains
            currentStrains.erase( it );
        else
            ++it;
    }
}

// get total number of contaminated units
uint ContEnvDetailed::getTotalUnits() const {
    return totalUnits;
}

// get total number of contaminated units relative to 'strain'
uint ContEnvDetailed::getTotalUnits( const StrainIdType& strain ) const {
    return static_cast<uint>( units[strain].size() );
}

BirdID ContEnvDetailed::getRandomContaminatedUnitID( const StrainIdType &strain, Random& randGen ) {
    if ( !units[strain].empty() )
        return randGen.getRandomElemFromVec( units[strain] );
    else
        return BirdID( UINT_MAX, 0 );
}
