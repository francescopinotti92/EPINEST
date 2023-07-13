//
//  birdID.cpp
//  CBM
//
//

#include <stdio.h>
#include "birdID.hpp"


std::string BirdID::toString() const {
    return std::to_string(idFarm) + "-" + std::to_string(idBird);
}


bool operator==(const BirdID& lhs, const BirdID& rhs) {
    if (lhs.idFarm != rhs.idFarm)
        return false;
    if (lhs.idBird != rhs.idBird)
        return false;
    return true;
}



std::size_t hashBirdID::operator()( const BirdID& id ) const {

    return std::hash<uint>()(id.idFarm) ^ (std::hash<uint>()(id.idBird) );

}
              
