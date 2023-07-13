//
//  middleman.hpp
//  CBM
//
//

#ifndef middleman_h
#define middleman_h

#include "types.hpp"
#include "holdsbirds.hpp"
#include "bird_urn.hpp"

class Middleman: public HoldsBirds {
public:
    //=== Constructor ===//
    
    Middleman( const uint& id_, const std::vector<BirdType>& birdTypes_, const uint& initialCapacityBirds );
    
    //=== Destructor ===//
    
    ~Middleman() {};
    
    //=== HoldsBirds specific functions ===//
    
    Bird& insertBird( pBird&& bird ) override;
    pBird removeBird( const uint& index ) override;
    pBird removeDeadBird( const uint& index ) override;
    void notifySuccessfulExposure( Bird& bird ) override;
    std::string getStringID() override;
    NodeType getSettingType() override;

    //=== Middleman specific functions ===//
    
    void reset( ChickenPool* birdMngr );
    const uint getId() const                                        { return id; }
    const uint& getMaxCapacity() const                              { return maxCapacity; }
    BirdUrn& getBirdUrn( const BirdType& bt, const uint& marketIndex ) { return birdUrnMngr.getBirdUrn( bt, marketIndex ); }
    
    std::vector<uint>* getNonEmptyMarketIndexes( const BirdType& bt ) {
        return birdUrnMngr.getNonEmptyUrnIds( bt );
    }
    
    /*
    uint* getIndexBatch(const BirdType& bt, const uint& marketId, const uint& amount) {
        return birdUrn.getIndexBatch( bt, marketId, amount );
    }*/
    
    
    
    uint  getBirdSizeType( const BirdType& bt );
    uint  getBirdSizeType( const BirdType& bt, const uint& marketId );
    
private:
    uint id;
    uint maxCapacity;
    BirdUrnMngr birdUrnMngr;
    
};



#endif /* middleman_h */
