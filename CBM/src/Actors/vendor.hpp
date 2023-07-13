//
//  vendor.hpp
//  CBM_ECS
//
//

#ifndef vendor_h
#define vendor_h

#include "types.hpp"
#include "holdsbirds.hpp"
#include "market.hpp"


class Market;

class Vendor: public HoldsBirds {
public:
    //=== Constructor ===//

    Vendor(const uint& id_,
           const uint& initialCapacityBirds,
           const std::vector<BirdType>& birdTypes_ );

    //=== Destructor ===//
    
    ~Vendor() {};
    
    //=== HoldsBirds specific functions ===//

    Bird& insertBird(pBird&& bird) override;
    pBird removeBird(const uint& index) override;
    pBird removeDeadBird(const uint& index) override;
    void notifySuccessfulExposure( Bird& bird ) override;
    std::string getStringID() override;
    NodeType getSettingType() override;
       
    //=== Vendor specific functions ===//
    
    void reset( ChickenPool* birdMngr );
    
    // manipulate birds in market
    void transferBirds( Market* market_ );
    void addNewBirdIndex( const uint& index );
    void removeBirdIndex( const uint& index );
    bool removeNewBirdIndex( const uint& index );
    bool removeRepurposedBirdIndex( const uint& index );
    uint popNewBirdIndex();
    uint popRepurposedBirdIndex();
    
    // getters
    std::vector<uint>& getMarketedRepurposedBirdsIndexes();
    std::vector<uint>& getMarketedNewBirdsIndexes();
    uint getSizeBirdsMarket() const;
    uint getSizeRepurposedBirdsMarket() const;
    uint getSizeNewBirdsMarket() const;
    uint getId();
private:
    uint id;
    std::vector<uint> newBirdsMarketIndex; // indexes of birds in market
    std::vector<uint> repurposedBirdsMarketIndex; // indexes of repurposed birds (in market or anywhere else)
};

//typedef std::shared_ptr<Vendor> pVendor;


#endif /* vendor_h */
