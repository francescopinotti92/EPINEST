//
//  WorldSimulator.hpp
//  CBM
//
//

#ifndef WorldSimulator_hpp
#define WorldSimulator_hpp

#include "infection_tracer.hpp"
#include "birdExit_tracker.hpp"
#include "birdExit_tracker_manager.hpp"
#include "completedCrop_tracker_manager.hpp"
#include "cross_sectional_tracer_manager.hpp"
#include "spatial_farm_transmission_manager.hpp"
#include "epidemic_dynamics.hpp"
#include "BaseSimulator.hpp"
#include "worldBuilder.hpp"
#include "ChickenPool.hpp"
#include "contamination.hpp"
#include <memory>

class CrossSectionalTrackerManager;

class WorldSimulator: public BaseSimulator {
    typedef short UnderlyingOutputFileType;
    enum class OutputFileType: UnderlyingOutputFileType {
        transactions = 0
    };
public:
    WorldSimulator( WorldBuilder& wb, std::string model_params_path, std::string epi_params_path, Random& randGen, std::string scenarioID, uint simID0 = 0 );
    
    void setEpiModel( BaseCompartmentalModel* compModel ) override;
    void endSimulation( bool resetWorld = false );
    void doTimeStep( Random& randGen, bool doEpi = true );
    void doPDNStep( Random& randGen );
    void doEpiStep( Random& randGen );
    void doCrossSectionalTrackingStep( bool preMovements = true );
    void setupFieldExperiment( nlohmann::json experiment_args );
    void resetSimulator();
    
    void doBetweenFarmTransmission( Random& randGen );
    void forceSparkInfectionRandomFarm( const uint& nInfections, const StrainIdType& strain, Random& randGen );
    void sparkInfectionFarms( const StrainIdType& strain, const double& sparkRate, Random& randGen );
    void sparkInfectionFarmsAreaSpecific( std::map<uint,StrainIdType>& area2strain, const double& sparkRate, Random& randGen );
    
    std::shared_ptr<LineageTrackerManager> getLngTrackerMngr() { return lngTrackerMngr ; }
    
    uint computeTotalBirdsMiddlemen();
    uint computeTotalBirdsInMarkets();
    uint computeVendorTotalStashedBirds();
    
    uint computeTotalInfectiousBirdsInMarkets() ;
    
    std::vector<NodeFarm*>& getFarmList()      { return *farmVec ; }
    std::vector<NodeMman*>& getMiddlemanList() { return *mmanVec ; }
    std::vector<NodeMarket*>& getMarketList()  { return *marketVec ; }
    std::vector<NodeVendor*>& getVendorList()  { return *vendorVec ; }
    uint getNumberOfAreas() { return areaMngr->getNumberOfAreas() ; }
    
    
    
    // testing
    void checkViralMixingFarms( std::map<uint,StrainIdType>& area2strain );
    
private:
    std::vector<NodeFarm*>* farmVec;
    std::vector<NodeMman*>* mmanVec;
    std::vector<NodeMarket*>* marketVec;
    std::vector<NodeVendor*>* vendorVec;
    std::vector<NodeFieldExperimenter*>* fieldExperimenterVec;
    
    ChickenPool* birdMngr;
    AreaManager* areaMngr;
    SpatialFarmTransmissionManager* farmTransmissionMngr;
    
    FarmStateManager* farmStates;
    MiddlemanStateManager* mmanStates;
    MarketStateManager* marketStates;
    VendorStateManager* vendorStates;
    FieldExperimenterStateManager* fieldExperimenterStates;
    
    BirdTransferManager* transferManager;
    CrossSectionalTrackerManager* trackerCSManager;
    BirdExitTrackerManager* trackerExitManager;
    CompletedCropTrackerManager* trackerCropManager;
    std::shared_ptr<LineageTrackerManager> lngTrackerMngr;
    
    uint nVendorLayers;
    
    std::string scenarioID;
    uint simID;
    
    std::string outputTransactionName;
    
    void shuffleFarmIdxs( Random& randGen ) { randGen.shuffleVector( farmIdxs ); }
    void shuffleMmanIdxs( Random& randGen ) { randGen.shuffleVector( mmanIdxs ); }
    void shuffleVendorIdxs( Random& randGen ) { randGen.shuffleVector( vendorIdxs ); }
    
    // epi dynamics function placeholders
    std::function<void ( Random& )> doTransmissionWithinSingleSetting;
    void doEpiTransitionEvents( Random& randGen );

    // actual epi functions
    void executeAllSingleSettingUpdates( Random& randGen );
    void executeBetweenFarmTransmission( Random& randGen );
    void doNothing( Random& randGen ) {};

    // vectors of operations
    std::vector< std::function< void ( HoldsBirds&, Random& ) > > singleSettingUpdates;
    std::function< void ( HoldsBirds&, const StrainIdType&, const double&, const Clock&, Random& )> sparkInfectionUpdate;
    std::function< void ( HoldsBirds&, const StrainIdType&, const uint&, const Clock&, Random& )> forceSparkInfectionUpdate;

    // auxiliary variables
    std::vector<uint> farmIdxs;
    std::vector<uint> mmanIdxs;
    std::vector<uint> vendorIdxs;
    ElementPool<uint> infectedFarmIdxs; // this is used when performing between farm transmission
    uint maxFarmSize;
};

#endif /* WorldSimulator_hpp */
