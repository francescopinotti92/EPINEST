//
//  farm.hpp
//  CBM_ECS
//
//

#ifndef farm_hpp
#define farm_hpp

#include <stdio.h>
#include <vector>
#include <deque>
#include <unordered_map>
#include "ChickenPool.hpp"
#include "crop.hpp"
#include "types.hpp"
#include "constants.hpp"
#include "holdsbirds.hpp"
#include "random.hpp"
#include "bird.hpp"

class BirdCropList;
class CompletedCropTrackerManager;

// !!! correct epi model stuff

class Farm: public HoldsBirds {
public:
    
    //====== Constructor ======//
    Farm( const uint& id_, const uint& initialCapacityBirds, const std::vector<BirdType>& birdTypes_ );
    
    //====== Destructor ======//
    ~Farm() {};
    
    //====== HoldsBirds functions ======//
    Bird& insertBird(pBird&& bird) override;
    pBird removeBird(const uint& index) override;
    pBird removeDeadBird(const uint& index) override;
    void notifySuccessfulExposure( Bird& bird ) override;
    std::string getStringID() override;
    NodeType getSettingType() override;
    
    //====== Farm specific functions ======//
    
    void reset( ChickenPool* birdMngr );
    bool hasReadyCrops();
    void addCrop( const BirdType& birdType_, const uint& cropSize_, const Clock& clock, Random& randGen, ChickenPool* birdMngr );
    std::deque<Crop*>& getReadyCrops( const BirdType& bt );
    bool hasReadyCrops( const BirdType& bt );
    void moveReadyCrops( const Clock& clock, CompletedCropTrackerManager* cropMngr );
    void clearCropList();
    void clearCropList( const Clock& clock, CompletedCropTrackerManager* cropMngr );
        
    const uint& getId() const { return id ; }
    const uint& getNBirdsCreated() const { return nBirdsCreated ; }
    
private:
    uint id;
    uint nBirdsCreated;
    
    std::unordered_map<BirdType, std::list<Crop*> > cropList;      // holds crops (not yet staged for sale)
    std::unordered_map<BirdType, std::deque<Crop*> > readyCropList;
    
};

#endif /* farm_hpp */
