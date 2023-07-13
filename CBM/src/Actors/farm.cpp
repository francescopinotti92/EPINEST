//
//  farm.cpp
//  CBM
//
//

#include <stdio.h>

#include "completedCrop_tracker_manager.hpp"
#include "farm.hpp"

//=== Constructor ===//

Farm::Farm(const uint& id_, const uint& initialCapacityBirds, const std::vector<BirdType>& birdTypes_): HoldsBirds( birdTypes_, initialCapacityBirds ), id( id_ ) {
    
    nBirdsCreated = 0;
    
    for ( const auto& bt: birdTypes ) {
        
        cropList[bt] = {}; // empty queues
        readyCropList[bt] = {};
        
    }
    
}

//=== HoldsBirds functions ===//

Bird& Farm::insertBird( pBird&& bird ) {
    Bird& bird_tmp = birds.insert( std::move( bird ) );
    return bird_tmp;
}

pBird Farm::removeBird( const uint& index ) {
    pBird bird = birds.remove( index );
    return bird;
}

pBird Farm::removeDeadBird( const uint& index ) {
    pBird bird = birds.remove( index );
    bird->cropPtr->remove( index );
    return bird;
}

void Farm::notifySuccessfulExposure( Bird& bird ) {
    
    // increase number of cumulative infections in flock (read exposures)
    ( bird.cropPtr )->nInfectionsCumulative++;
    
}

std::string Farm::getStringID() {
    std::string res = "F";
    res += std::to_string( id );
    return res;
}

NodeType Farm::getSettingType() {
    return NodeType::farm;
}

//=== Farm specific functions ===//

// returns true if there are crops that can be solved
bool Farm::hasReadyCrops() {
    for ( auto& bt: birdTypes ) {
        if ( readyCropList[bt].size() > 0 )
            return true;
    }
    return false;
}

// add a single crop
void Farm::addCrop( const BirdType& birdType_, const uint& cropSize_, const Clock& clock, Random& randGen, ChickenPool* birdMngr ) {
    
    Crop* newCrop = new Crop( cropSize_, clock ); // remember to delete pointer later
    
    for ( uint i = 0; i < cropSize_; ++i ) {
        
        pBird newBird = birdMngr->request_bird();
        newBird->setBird( 0, birdType_, BirdIdType( id, nBirdsCreated ), clock.getCurrTime(), randGen.getExpo(), newCrop, this );
        
        Bird& tmp = birds.insertSusceptible( std::move( newBird ) );
        newCrop->insert( tmp.index );
        nBirdsCreated++;
        
    }
    
    cropList[birdType_].push_back( newCrop );
}

// stage crops that are ready in the appropriate list
void Farm::moveReadyCrops( const Clock& clock, CompletedCropTrackerManager* cropMngr ) {
    
    for ( auto& bt: birdTypes ) {
        
        if ( cropList[bt].size() == 0 )
            return;
            
        while ( cropList[bt].size() > 0 ) {
            
            Crop* crop = cropList[bt].front();
            
            if ( crop->size() == 0 ) {
                cropMngr->processCompletedCrop( *this, crop, clock );
                delete crop; // clean empty crops (could be empty because of birds dying)
                cropList[bt].pop_front();
                continue;
            }
            
            if ( clock.getCurrDay() - crop->dayCreate >= constants::harvestTimes[bt] ) {
                readyCropList[bt].push_back( crop );
                cropList[bt].pop_front();
            }
            else
                break; // no more harvestable crops
        }
    
    }
    
}

std::deque<Crop*>& Farm::getReadyCrops( const BirdType& bt ) {
    
    return readyCropList[bt];
    
}


// clear all crops from croplist
// does not track crop stats
void Farm::clearCropList() {
    
    for ( auto& bt: birdTypes ) {
        //cropList.clear( bt );
        
        // release memory on the heap
        auto& crops = cropList[bt];
        for ( auto& crop : crops ) {

            delete crop;

        }
        crops.clear();
            

        for ( Crop* crop: readyCropList[bt] ) {
            
            //cropMngr->processCompletedCrop( *farm, crop, clock );
            delete crop;
            
        }
        readyCropList[bt].clear();
        
    }
    
}

// clear all crops from croplist
void Farm::clearCropList( const Clock& clock, CompletedCropTrackerManager* cropMngr ) {
   
    for ( auto& bt: birdTypes ) {
            
        // release memory on the heap
        auto& crops = cropList[bt];
        for ( auto& crop : crops ) {

            // ?? add flag?
            cropMngr->processCompletedCrop( *this, crop, clock );
            delete crop;

        }
        crops.clear();
            

        for ( Crop* crop: readyCropList[bt] ) {
            
            // ?? add flag?
            cropMngr->processCompletedCrop( *this, crop, clock );
            delete crop;
            
        }
        readyCropList[bt].clear();
        
    }
    
}

void Farm::reset( ChickenPool* birdMngr ) {
    nBirdsCreated = 0;
    clearCropList(); // does not store cleared crops
    birds.clear( birdMngr );
    contEnv->clear();
}
