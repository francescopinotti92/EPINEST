//
//  birdTransferManager.hpp
//  CBM
//
//

#ifndef birdTransferManager_hpp
#define birdTransferManager_hpp

#include "node.hpp"
#include "ChickenPool.hpp"
#include "completedCrop_tracker_manager.hpp"
#include "birdExit_tracker_manager.hpp"
#include "named_ofstream.hpp"
#include <fstream>
#include <iostream>

class NodeFarm;
class NodeMman;
class NodeMarket;
class NodeVendor;
class NodeFieldExperimenter;
class CompletedCropTrackerManager;

struct TransactionFarm2Mman {
    TransactionFarm2Mman();
    TransactionFarm2Mman( const uint& farmID, const uint& mmanID, const uint& marketID, const BirdType& bt, const uint& nbirds, const uint& t);
    std::string stringify();
    uint farmID;
    uint mmanID;
    uint marketID;
    BirdType bt;
    uint nbirds;
    uint t;
};

struct TransactionFarm2Vendor {
    TransactionFarm2Vendor();
    TransactionFarm2Vendor( const uint& farmID, const uint& vendorID, const uint& marketID, const BirdType& bt, const uint& nbirds, const uint& t);
    std::string stringify();
    uint farmID;
    uint vendorID;
    uint marketID;
    BirdType bt;
    uint nbirds;
    uint t;
};

struct TransactionMman2Vendor {
    TransactionMman2Vendor();
    TransactionMman2Vendor( const uint& mmanID, const uint& vendorID, const uint& marketID, const BirdType& bt, const uint& nbirds, const uint& t);
    std::string stringify();
    uint mmanID;
    uint vendorID;
    uint marketID;
    BirdType bt;
    uint nbirds;
    uint t;
};

struct TransactionVendor2Vendor {
    TransactionVendor2Vendor();
    TransactionVendor2Vendor( const uint& vendorID1, const uint& vendorID2, const uint& marketID, const BirdType& bt, const uint& nbirds, const uint& t);
    std::string stringify();
    uint vendorID1;
    uint vendorID2;
    uint marketID;
    BirdType bt;
    uint nbirds;
    uint t;
};

struct TransactionVendor2Community {
    TransactionVendor2Community();
    TransactionVendor2Community( const uint& vendorID, const uint& marketID, const BirdType& bt, const uint& nbirds, const uint& t);
    std::string stringify();
    uint vendorID;
    uint marketID;
    BirdType bt;
    uint nbirds;
    uint t;
};

struct TransactionMman2VendorDetailed {
    TransactionMman2VendorDetailed();
    TransactionMman2VendorDetailed( const uint& mmanID, const uint& vendorID, const uint& marketID, const BirdType& bt, const uint& nbirds, const uint& t);
    std::string stringify();
    uint mmanID;
    uint vendorID;
    uint marketID;
    BirdType bt;
    uint nbirds;
    uint t;
};


class BirdTransferManager {
public:
    BirdTransferManager( NamedOfstream outFile, Clock* clock, ChickenPool* pool, uint dayObsStart = 0, uint dayObsStop = 10000000, bool logFarm2Mman = false, bool logMman2Vendor = false, bool logVendor2Vendor = false, bool logVendor2Community = false, uint maxTransactionsBuffer = 10000 );
    
    void reset( bool updateStream = true );
    void setBirdExitTrackerManager( BirdExitTrackerManager* trackerManager );
    void setCompletedCropTrackerManager( CompletedCropTrackerManager* trackerManager );
    
    std::function< void ( NodeFarm*, NodeMman*, const BirdType&, const uint& marketDst, const uint&, Random&) > transferBirdsFarm2Mman;
    std::function< void ( NodeMman*, NodeMarket*, const BirdType&, const uint&, Random&) > transferBirdsMman2Market;
    std::function< void ( NodeVendor*, NodeVendor*, NodeMarket*, uint, Random& ) > transferBirdsVendor2Vendor;
    std::function< void ( NodeVendor*, NodeMarket*, uint, Random& ) > transferBirdsVendor2Community;
    
    void transferBirdsFarm2Nothing( NodeFarm* nf, Crop& crop ) ;
    void transferBirdsFarm2FieldExperimenter( NodeFarm* nf, NodeFieldExperimenter* nfe, const BirdType& bt, const uint& nbirds, Random& randGen ) ;
    void transferBirdsVendor2FieldExperimenter( NodeVendor* nv, NodeFieldExperimenter* nfe, NodeMarket* nm, const BirdType& bt, const uint& nbirds, Random& randGen ) ;
    
    //std::function< void ( NodeFarm*, Crop& ) > transferBirdsFarm2Nothing ;

    
    void addFarm2MmanTransaction( NodeFarm* nf, NodeMman* nmm, const uint& marketID, const BirdType& bt, const uint& nbirds );
    void addMman2VendorTransaction( NodeMman* nmm, NodeVendor* nv, NodeMarket* nm, const BirdType& bt, const uint& nbirds );
    void addVendor2VendorTransaction( NodeVendor* nv1, NodeVendor* nv2, NodeMarket* nm, const BirdType& bt, const uint& nbirds );
    void addVendor2CommunityTransaction( NodeVendor* nv1, NodeMarket* nm, const BirdType& bt, const uint& nbirds );
    
    
    // data-flush functions
    void flushFarm2MmanTransactions();
    void flushMman2VendorTransactions();
    void flushVendor2VendorTransactions();
    void flushVendor2CommunityTransactions();
    void flushEverything();
    
private:
    
    // transfers bird
    
    // actual transfer functions ( without logging )
    void _transferBirdsFarm2Mman( NodeFarm* nf, NodeMman* nmm, const BirdType& bt, const uint& marketDst, const uint& nbirds, Random& randGen );
    void _transferBirdsMman2Market( NodeMman* nmm, NodeMarket* nm, const BirdType& bt, const uint& nbirds, Random& randGen );
    void _transferBirdsVendor2Vendor( NodeVendor* nv1, NodeVendor* nv2, NodeMarket* nm, uint nbirds, Random& randGen );
    void _transferBirdsVendor2Community( NodeVendor* nv, NodeMarket* nm, uint nbirds, Random& randGen );
    
    
    // actual transfer functions ( with logging )

    void _transferBirdsFarm2MmanLog( NodeFarm* nf, NodeMman* nmm, const BirdType& bt, const uint& nbirds, const uint& marketDst, Random& randGen );
    void _transferBirdsMman2MarketLog( NodeMman* nmm, NodeMarket* nm, const BirdType& bt, const uint& nbirds, Random& randGen );
    void _transferBirdsVendor2VendorLog( NodeVendor* nv1, NodeVendor* nv2, NodeMarket* nm, uint nbirds, Random& randGen );
    void _transferBirdsVendor2CommunityLog( NodeVendor* nv, NodeMarket* nm, uint nbirds, Random& randGen );
    
    // transaction buffers
    std::vector<TransactionFarm2Vendor> farm2VendorBuffer;
    std::vector<TransactionFarm2Mman> farm2MmanBuffer;
    std::vector<TransactionMman2Vendor> mman2VendorBuffer;
    std::vector<TransactionVendor2Vendor> vendor2VendorBuffer;
    std::vector<TransactionVendor2Community> vendor2CommunityBuffer;
    
    Clock* clock;
    BirdExitTrackerManager* birdExitTracker;
    CompletedCropTrackerManager* completedCropTracker;
    ChickenPool* birdMngr;
    uint dayObsStart;
    uint dayObsStop;
    NamedOfstream outFile;
    uint maximumTransactions;
        
};

#endif /* birdTransferManager_hpp */
