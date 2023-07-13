//
//  bird.hpp
//  CBM_ECS
//
//

#ifndef bird_h
#define bird_h

#include "birdTracker.hpp"
#include "crop.hpp"
#include "birdID.hpp"
#include "types.hpp"

/*
 "Bird" represents individual entities in this project.
 Each "Bird" instance belongs to a single "infected_set" at any time. "index" member allows to find its position in the set. "id" is a unique identifier.
 */

class HoldsBirds;
class Vendor;

typedef BirdID BirdIdType;

struct Bird {
    uint index;                 // allows to locate bird in infected_set
    BirdType birdType;          // bird species (Broiler, Deshi, Sonali, Duck, ...)
    BirdIdType id;              // unique bird ID
    uint t_birth;               // birth time
    uint t_market;              // marketing time;
    double hazard;              // quantity related to pathogen transmission
    HoldsBirds* owner;          // current physical owner
    union {                     // --union contains variables with non-overlapping lifetimes--
        Crop* cropPtr;          // pointer to crop (when inside Farm)
        uint marketDestination; // marks where bird will be sold (when carried by mman)
        Vendor* vendorPtr;      // pointer to vendor (when in Market/Vendor)
    };
    BirdMovements movements;    // contains stats about chicken movements along PDN
    StateType immState;      // set of previously seen strains
    StateType infState;      // set of currently carried strains
    short pendingInfections; // number of infections to be applied
    
    Bird( const uint& index_, const BirdType& birdType_, const BirdIdType& id_, const uint& t_birth_, const double& hazard_, Crop* cropPtr_, HoldsBirds* owner_ );     // constructor
    
    void setBird( const uint& index_, const BirdType& birdType_, const BirdIdType& id_, const uint& t_birth_, const double& hazard_, Crop* cropPtr_, HoldsBirds* owner_ ); // setter
    
    inline BirdIdType getID() const {
        
        return id ;
        
    }
    
    inline bool isInfectious() {
        return ( infState == 0 ) ? false : true;          // checks if bird is infectious or not
    }
    inline bool isLatent() {
        return ( pendingInfections > 0 ) ? true : false ;
    }
    inline bool isValid() {
        return ( owner != nullptr ) ? true : false;       // checks if chicken is valid
    }

};






#endif /* bird_h */
