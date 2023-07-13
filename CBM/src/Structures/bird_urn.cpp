//
//  bird_urn.cpp
//  CBM
//
//

#include "bird_urn.hpp"

BirdUrn::BirdUrn(): _size( 0 ) {};

void BirdUrn::insertBird( const uint& index ) {
    
    _indexPool.push_back( index );
    ++_size;
    
}

void BirdUrn::insertBird( const Bird &bird ) {
    
    _indexPool.push_back( bird.index );
    ++_size;
    
}

void BirdUrn::insertBird( const pBird &bird ) {
    
    _indexPool.push_back( bird->index );
    ++_size;
    
}

void BirdUrn::markInvalidBird( const uint &index ) {
    
    for ( auto& idx : _indexPool ) {
        
        if ( idx == index ) {
            idx = BirdUrn::INVALID;
            --_size;
            return;
            
        }
        
    }
    
}


uint BirdUrn::pop_front() {
    
    uint index = BirdUrn::INVALID;
    
    while( _size > 0  ) {
        
        index = _indexPool.front();
        if ( index != BirdUrn::INVALID ) {
            _indexPool.pop_front();
            --_size;
            return index;
        }
        else {
            _indexPool.pop_front();
        }
        
    }
    
    return index;
    
}

void BirdUrn::clear() {
    
    _indexPool.clear();
    _size = 0;
    
}



BirdUrnMngr::BirdUrnMngr( const std::vector<BirdType>& birdTypes ) {
    
    for (BirdType bt: birdTypes)
        urnList[bt] = std::unordered_map<uint, BirdUrn>();
    
}

// returns a vector with indexes of non-empty urns for a given type of birds
std::vector<uint>* BirdUrnMngr::getNonEmptyUrnIds( const BirdType& bt ) {
    
    std::vector<uint>* idxs = new std::vector<uint>();
    idxs->reserve( 10 );
    
    auto& partialUrnList = urnList[bt];
    
    for ( auto& urn: partialUrnList ) {
        if ( urn.second.size() > 0 )
            idxs->push_back( urn.first );
    }
    
    return idxs;
}

void BirdUrnMngr::clear() {
    for ( auto& btSetPair: urnList )
        btSetPair.second.clear();
}

void BirdUrnMngr::clear( const BirdType& birdType ) {
    for ( auto& elem: urnList[birdType] )
        elem.second.clear();
}

uint BirdUrnMngr::size( const BirdType& birdType, const uint& marketIndex ) {
    return ( urnList[birdType][marketIndex].size() );
}

uint BirdUrnMngr::size( const BirdType& birdType ) {
    uint count = 0;
    auto& partialUrnList = urnList[birdType];
    
    for ( auto& urn: partialUrnList )
        count += static_cast<uint>( urn.second.size() );
    
    return count;
}
