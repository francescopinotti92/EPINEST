//
//  fieldexperimenter.hpp
//  CBM
//
//

#ifndef fieldexperimenter_hpp
#define fieldexperimenter_hpp

#include "holdsbirds.hpp"
#include "types.hpp"

class FieldExperimenter: public HoldsBirds {
public:
    //=== Constructor ===//
    FieldExperimenter( const BirdType& birdType_, const uint& initialCapacityBirds );
    
    //=== Destructor ===//
    ~FieldExperimenter() {};
    
    //=== HoldsBirds specific functions ===//
    
    Bird& insertBird(pBird&& bird) override;
    pBird removeBird(const uint& index) override;
    pBird removeDeadBird(const uint& index) override;
    void notifySuccessfulExposure( Bird& bird ) override;
    std::string getStringID() override;
    NodeType getSettingType() override;
    std::unordered_set<uint>& getBirdsMarketIndex() ;
    
    void registerFarmRecruitedBird( Bird& bird ) ;
    void registerMarketRecruitedBird( Bird& bird ) ;

    //Bird& insertNewBird( pBird&& bird, Vendor* vendor );
    
    //=== FieldExperimenter specific functions ===//
    void reset( ChickenPool* birdMngr );
private:
    std::unordered_set<uint> birdsMarketIndex ;
    std::unordered_map<BirdID,short,hashBirdID> experimentGroup ; // 0 for market, 1 for farm
    
} ;

#endif /* fieldexperimenter_hpp */
