//
//  market.hpp
//  CBM_ECS
//
//

#ifndef market_h
#define market_h

#include "constants.hpp"
#include "types.hpp"
#include "vendor.hpp"
#include "market.hpp"
#include "holdsbirds.hpp"

class Vendor;

class Market: public HoldsBirds {
public:
    //=== Constructor ===//
    Market( const uint& id_, const std::vector<BirdType>& birdTypes_, const uint& initialCapacityBirds );
    
    //=== Destructor ===//
    ~Market() {};
    
    //=== HoldsBirds specific functions ===//
    
    Bird& insertBird(pBird&& bird) override;
    pBird removeBird(const uint& index) override;
    pBird removeDeadBird(const uint& index) override;
    void notifySuccessfulExposure( Bird& bird ) override;
    std::string getStringID() override;
    NodeType getSettingType() override;
    Bird& insertNewBird( pBird&& bird, Vendor* vendor );
    
    //=== Market specific functions ===//
    void reset( ChickenPool* birdMngr );
    inline uint getId()         { return id; }
   
private:
    uint id;
};

#endif /* market_h */
