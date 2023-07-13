//
//  WorldSimulator.cpp
//  CBM
//
//

#include "WorldSimulator.hpp"
#include "cross_sectional_tracker.hpp"
#include "lineage_tracker.hpp"
#include "CompletedCropTracker.hpp"


WorldSimulator::WorldSimulator( WorldBuilder& wb, std::string model_params_path, std::string epi_params_path, Random& randGen, std::string scenarioID_, uint simID0 ): scenarioID( scenarioID_ ), simID( simID0 ), infectedFarmIdxs( 1 ) {
    
    // set up bird exit trackers
    
    placeVec  = wb.getNodeVec();
    farmVec   = wb.getFarmVec();
    mmanVec   = wb.getMmanVec();
    marketVec = wb.getMarketVec();
    vendorVec = wb.getVendorVec();
    fieldExperimenterVec = new std::vector<NodeFieldExperimenter*>();
    
    farmStates   = wb.getFarmStates();
    mmanStates   = wb.getMmanStates();
    marketStates = wb.getMarketStates();
    vendorStates = wb.getVendorStates();
    fieldExperimenterStates = new FieldExperimenterStateManager() ;
    
    nVendorLayers = wb.getNoVendorLayers();
        
    // assign AreaManager
    areaMngr = wb.getAreaManager();
    
    // assign farm transmission manager
    farmTransmissionMngr = nullptr;
    
    // set up bird manager
    birdMngr = new ChickenPool( 1000000 ); // set 1000000 pointers to begin with
    
    // set up transfer manager
    bool logAny = false;
    uint transactionLogsBufferSize = 0;
    
    std::ifstream readFileModelParams( model_params_path );
    nlohmann::json paramsModel = nlohmann::json::parse( readFileModelParams );
    
    requireJsonKey( paramsModel, "outputParams" );
    requireJsonKey( paramsModel["outputParams"], "birdTransactions" );
    requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "logTransactionsFarm2Mman" );
    requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "logTransactionsMman2Vendor" );
    requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "logTransactionsVendor2Vendor" );
    requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "logTransactionsVendor2Community" );
        
    bool logFarmsMman = paramsModel["outputParams"]["birdTransactions"]["logTransactionsFarm2Mman"].get<bool>();
    bool logMman2Vendor = paramsModel["outputParams"]["birdTransactions"]["logTransactionsMman2Vendor"].get<bool>();
    bool logVendor2Vendor = paramsModel["outputParams"]["birdTransactions"]["logTransactionsVendor2Vendor"].get<bool>();
    bool logVendor2Comm = paramsModel["outputParams"]["birdTransactions"]["logTransactionsVendor2Community"].get<bool>();

    uint dayStartLogging = 0;
    uint dayStopLogging = 10000000;

    if ( logFarmsMman )
        logAny = true;
    if ( logMman2Vendor )
        logAny = true;
    if ( logVendor2Vendor )
        logAny = true;
    if ( logVendor2Comm )
        logAny = true;
    
    lngTrackerMngr = nullptr ;
    
    if ( logAny ) {
        // initialize transferManager with transaction recording feature
        
        requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "transactionBufferSize" );
        requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "logTransactionDayStart" );
        requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "logTransactionDayStop" );
        requireJsonKey( paramsModel["outputParams"]["birdTransactions"], "transactionOutputFolderPath" );

        transactionLogsBufferSize = paramsModel["outputParams"]["birdTransactions"]["transactionBufferSize"].get<uint>();
        
        dayStartLogging = paramsModel["outputParams"]["birdTransactions"]["logTransactionDayStart"].get<uint>();
        dayStopLogging = paramsModel["outputParams"]["birdTransactions"]["logTransactionDayStop"].get<uint>();
        
        std::string path_to_output_folder = paramsModel["outputParams"]["birdTransactions"]["transactionOutputFolderPath"].get<std::string>();
        std::string fileName = "birdTransactions";
        std::string fileExtension = "txt";
        
        NamedOfstream outFile( path_to_output_folder, fileName, fileExtension, scenarioID, simID );
        
        transferManager = new BirdTransferManager( outFile, &(BaseSimulator::clock), birdMngr, dayStartLogging, dayStopLogging, logFarmsMman, logMman2Vendor, logVendor2Vendor, logVendor2Comm, transactionLogsBufferSize );
        
    }
    else {
        // initialize transferManager without transaction recording feature
        NamedOfstream outFile = NamedOfstream(); // inert file stream
        transferManager = new BirdTransferManager( outFile, &(BaseSimulator::clock), birdMngr, dayStartLogging, dayStopLogging, logFarmsMman, logMman2Vendor, logVendor2Vendor, logVendor2Comm, transactionLogsBufferSize );
    }
    
    // set up individual cases tracker
    {
        requireJsonKey( paramsModel["outputParams"], "individualInfectionOutput" );

        auto& info = paramsModel["outputParams"]["individualInfectionOutput"];
        
        requireJsonKey( info, "trackIndividualInfections" );
        
        bool tracking = info["trackIndividualInfections"].get<bool>();
        
        if ( tracking ) {
            
            // create tracker
            requireJsonKey( info, "trackIndividualInfectionMode" );
            std::string mode = info["trackIndividualInfectionMode"].get<std::string>();
            
            if ( mode == "WIW" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );

                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // can also read buffer size?
                requireJsonKey( info, "tStartTracking" );
                requireJsonKey( info, "tStopTracking" );

                tracker = new WIWTracker( outFile );
                uint tStartTracking = info["tStartTracking"].get<uint>();
                uint tStopTracking = info["tStopTracking"].get<uint>();
                tracker->setTrackingWindow( tStartTracking, tStopTracking );
                
            }
            else if ( mode == "LineageTracking" ) {
                
                // get epi parameters
                std::ifstream readEpiParams( epi_params_path ) ;
                nlohmann::json epi_settings = nlohmann::json::parse( readEpiParams ) ;
                readEpiParams.close();
                
                requireJsonKey( epi_settings, "nStrains" ) ;
                requireJsonKey( epi_settings, "maxStrains" ) ;
                
                uint nStrains = epi_settings["nStrains"] ;
                uint maxStrains = epi_settings["maxStrains"] ;
                
                lngTrackerMngr = std::make_shared<LineageTrackerManager>( nStrains, maxStrains ) ;
                tracker = new NewLineageTracker( lngTrackerMngr ) ;
                
            }

        }
        else{
            tracker = nullptr;
        }
    }
    
    // set up cross-sectional trackers
    {
        
        requireJsonKey( paramsModel["outputParams"], "crossSectionalAnalyses" );

        trackerCSManager = new CrossSectionalTrackerManager();

        // loop over trackers
        for ( auto& analysis: paramsModel["outputParams"]["crossSectionalAnalyses"].items() ) {
            
            std::string nameAnalysis = analysis.key();
            auto& info = analysis.value();
            
            // tracks prevalence by setting type, periodically and at specific hours of the day
            if ( nameAnalysis == "birdCountBySettingType") {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );

                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                std::shared_ptr<AggregateBirdCountSettingTracker> trackerCS = std::make_shared<AggregateBirdCountSettingTracker>( outFile, periodDays, hoursTracking, tmin );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
                
            }
            else if ( nameAnalysis == "prevalenceBySettingType" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );

                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                std::shared_ptr<AggregatePrevalenceSettingTracker> trackerCS = std::make_shared<AggregatePrevalenceSettingTracker>( outFile, periodDays, hoursTracking, tmin );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
            }
            else if ( nameAnalysis == "immunityBySettingType" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );

                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<AggregateImmunitySettingTracker> trackerCS = std::make_shared<AggregateImmunitySettingTracker>( outFile, periodDays, hoursTracking, tmin );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
                
            }
            else if ( nameAnalysis == "infectionMultiplicityBySettingType" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );

                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<AggregateInfectionMultiplicityBySettingTracker> trackerCS = std::make_shared<AggregateInfectionMultiplicityBySettingTracker>( outFile, periodDays, hoursTracking, tmin );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
                
            }
            else if ( nameAnalysis == "infectedFarms" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                std::shared_ptr<InfectedFarmsTracker> trackerCS = std::make_shared<InfectedFarmsTracker>( outFile, periodDays, hoursTracking, "," );
                
                trackerCSManager->addTracker( trackerCS );

            }
            else if ( nameAnalysis == "birdMixingMarket") {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                requireJsonKey( info, "infectedOnly" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                bool infectedOnly = info["infectedOnly"].get<bool>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<BirdMixingMarketTracker> trackerCS = std::make_shared<BirdMixingMarketTracker>( outFile, infectedOnly, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
    
            }
            
            else if ( nameAnalysis == "AggregateRichnessMarkets") {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<AggregateRichnessMarkets> trackerCS = std::make_shared<AggregateRichnessMarkets>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
    
            }
            
            else if ( nameAnalysis == "AggregateSimpsonDiversityMarkets") {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<AggregateSimpsonDiversityMarkets> trackerCS = std::make_shared<AggregateSimpsonDiversityMarkets>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
    
            }
            
            else if ( nameAnalysis == "RichnessSingleMarkets") {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<RichnessSingleMarkets> trackerCS = std::make_shared<RichnessSingleMarkets>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
    
            }
            
            else if ( nameAnalysis == "RichnessSingleAreas") {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<RichnessSingleAreas> trackerCS = std::make_shared<RichnessSingleAreas>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
    
            }
            
            else if ( nameAnalysis == "strainMixingMarket" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<StrainMixingMarketTracker> trackerCS = std::make_shared<StrainMixingMarketTracker>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
                
            }
            
            else if ( nameAnalysis == "FarmTradedBirdsInfectedTracker" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                // optional: tmin, checkExposed
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0 ;
                bool checkExposed = ( info.contains( "checkExposed" ) ) ? info["checkExposed"].get<bool>() : true ;
                
                std::shared_ptr<FarmTradedBirdsInfectedTracker> trackerCS = std::make_shared<FarmTradedBirdsInfectedTracker>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
                                
                for ( auto& nmm : *mmanVec )
                    nmm->doRecordIncomingInfectedBirds( checkExposed ) ; // tell markets to record infectious and/or exposed birds
            }
            
            else if ( nameAnalysis == "MmanTradedBirdsByFarmTracker" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<MmanTradedBirdsByFarmTracker> trackerCS = std::make_shared<MmanTradedBirdsByFarmTracker>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
                                
                for ( auto& nm : *marketVec )
                    nm->doRecordBirdSource() ; // tell markets to record bird source
                
            }
            
            else if ( nameAnalysis == "MmanTradedBirdsInfectedTracker" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                requireJsonKey( info, "hours" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::vector<uint> hoursTracking = info["hours"].get<std::vector<uint>>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0 ;
                bool checkExposed = ( info.contains( "checkExposed" ) ) ? info["checkExposed"].get<bool>() : true ;
                
                std::shared_ptr<MmanTradedBirdsInfectedTracker> trackerCS = std::make_shared<MmanTradedBirdsInfectedTracker>( outFile, periodDays, hoursTracking, tmin );
                
                trackerCSManager->addTracker( trackerCS );
                                
                for ( auto& nm : *marketVec )
                    nm->doRecordIncomingInfectedBirds( checkExposed ) ; // tell markets to record infectious and/or exposed birds
                
            }
            
            
            else if ( nameAnalysis == "InfectedTradedBirdsTracker" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );

                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                uint tmin = ( info.contains( "tmin" ) ) ? info["tmin"].get<uint>() : 0;
                
                std::shared_ptr<InfectedTradedBirdsTracker> trackerCS = std::make_shared<InfectedTradedBirdsTracker>( outFile, periodDays, tmin );
                
                trackerCSManager->addTracker( trackerCS, false ); // track after movements
                                
                
            }
            
            else if ( nameAnalysis == "unsoldStockMmen" ) {
               
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::shared_ptr<UnsoldMmanStockTracker> trackerCS = std::make_shared<UnsoldMmanStockTracker>( outFile, periodDays );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
                
            }
            else if ( nameAnalysis == "unsoldStockVendors" ) {
               
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::shared_ptr<UnsoldVendorStockTracker> trackerCS = std::make_shared<UnsoldVendorStockTracker>( outFile, periodDays );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
                
            }
            else if ( nameAnalysis == "farmTradeRolloutDuration" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                std::shared_ptr<FarmRolloutTimeTracker> trackerCS = std::make_shared<FarmRolloutTimeTracker>( outFile );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
            }
            else if ( nameAnalysis == "farmDiscardedBirds" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "period" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                uint periodDays = info["period"].get<uint>();
                std::shared_ptr<DiscardedBirdsTracker> trackerCS = std::make_shared<DiscardedBirdsTracker>( outFile, periodDays );

                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
            }
            else if ( nameAnalysis == "initialVendorDemography" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                std::shared_ptr<InitialVendorDemographyPrinter> trackerCS = std::make_shared<InitialVendorDemographyPrinter>( outFile );

                // register tracker to tracker managers
                trackerCSManager->addTrackerT0( trackerCS );
            }
            else if ( nameAnalysis == "initialMiddlemanDemography" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                std::shared_ptr<InitialMiddlemanDemographyPrinter> trackerCS = std::make_shared<InitialMiddlemanDemographyPrinter>( outFile );

                // register tracker to tracker managers
                trackerCSManager->addTrackerT0( trackerCS );
                
            }
            else if ( nameAnalysis == "lineagePersistence" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                requireJsonKey( info, "tmin" );
                requireJsonKey( info, "tmax" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                
                // create tracker
                
                uint tmin = info["tmin"].get<uint>() ;
                uint tmax = info["tmax"].get<uint>() ;
                uint minSize = ( info.contains( "minSize" ) ) ? info["minSize"].get<uint>() : 0;

                
                std::shared_ptr<NewMarketLineagesTracker> trackerCS = std::make_shared<NewMarketLineagesTracker>( outFile, tmin, tmax, minSize ) ;
                
                // register tracker to tracker managers
                trackerCSManager->addTracker( trackerCS );
                
                // This tracker requires a custom infection tracker
                assert( tracker == nullptr ) ; // another tracker was provided!
                
                // create tracker
               
                tracker = new NewMarketLineagesWIWHelperTracker( trackerCS ) ;
                tracker->setTrackingWindow( tmin, tmax ) ;
                                

            }
            else if ( nameAnalysis == "homochronousMarketPhylogeny") {
                
                requireJsonKey( info, "pathToFile" ) ;
                requireJsonKey( info, "fileName" ) ;
                requireJsonKey( info, "fileExtension" ) ;
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>() ;
                std::string fileName = info["fileName"].get<std::string>() ;
                std::string fileExtension = info["fileExtension"].get<std::string>() ;
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID ) ;
                
                requireJsonKey( info, "tSample" ) ;
                requireJsonKey( info, "chickensSampledPerMarket" ) ;
                requireJsonKey( info, "marketsSampled" ) ;
                
                uint tSample = info["tSample"].get<uint>() ;
                uint chickensSampledPerMarket = info["chickensSampledPerMarket"].get<uint>() ;
                std::vector<uint> marketsSampled = info["marketsSampled"].get<std::vector<uint>>() ;
                
                std::shared_ptr<WritePhylogenyHomochronousMarketSampling> trackerCS = std::make_shared<WritePhylogenyHomochronousMarketSampling>( outFile, tSample, &randGen, chickensSampledPerMarket, marketsSampled ) ;
                
                trackerCSManager->addTracker( trackerCS ) ;
                                
            }
            else if ( nameAnalysis == "heterochronousMarketPhylogeny" ) {
                
                requireJsonKey( info, "pathToFile" ) ;
                requireJsonKey( info, "fileName" ) ;
                requireJsonKey( info, "fileExtension" ) ;
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>() ;
                std::string fileName = info["fileName"].get<std::string>() ;
                std::string fileExtension = info["fileExtension"].get<std::string>() ;
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID ) ;
                
                
                requireJsonKey( info, "samplingTimes" ) ;
                requireJsonKey( info, "marketsSampledIdxs" ) ;
                requireJsonKey( info, "birdSampledAmounts" ) ;
                
                std::vector<uint> samplingTimes = info["samplingTimes"].get<std::vector<uint>>() ;
                std::vector<uint> marketsSampledIdxs = info["marketsSampledIdxs"].get<std::vector<uint>>() ;
                std::vector<uint> birdSampledAmounts = info["birdSampledAmounts"].get<std::vector<uint>>() ;
                
                std::shared_ptr<WritePhylogenyHeterochronousMarketSampling> trackerCS = std::make_shared<WritePhylogenyHeterochronousMarketSampling>( outFile, &randGen, samplingTimes, marketsSampledIdxs, birdSampledAmounts ) ;
                
                trackerCSManager->addTracker( trackerCS ) ;
                
            }
            
            
        }

        
    }
    
    // setup bird exit trackers
    {
        
        requireJsonKey( paramsModel["outputParams"], "birdExitAnalyses" );

        trackerExitManager = new BirdExitTrackerManager();
        
        for ( auto& analysis: paramsModel["outputParams"]["birdExitAnalyses"].items() ) {
            std::string nameAnalysis = analysis.key();
            auto& info = analysis.value();
                        
            // tracks prevalence by setting type, periodically and at specific hours of the day
            if ( nameAnalysis == "marketingTimeDuration" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                                
                // create tracker
                
                std::shared_ptr<BirdMarketingDurationTracker> tracker = std::make_shared<BirdMarketingDurationTracker>( outFile, HOURS_DAY * 20 );

                // register tracker to tracker managers
                trackerExitManager->addMarketVendorTracker( tracker );

            }
            
        }
        
        // check lineage analysis
        {
            
            auto& info = paramsModel["outputParams"]["individualInfectionOutput"];
            bool tracking = info["trackIndividualInfections"].get<bool>();
            
            if ( tracking ) {

                requireJsonKey( info, "trackIndividualInfectionMode" ) ;
                std::string mode = info["trackIndividualInfectionMode"].get<std::string>();
                
                if ( mode == "LineageTracking" ) {
            
                    std::shared_ptr<LineageBirdRemovalTracker> trackerTmp = std::make_shared<LineageBirdRemovalTracker>( lngTrackerMngr ) ;
                    
                    trackerExitManager->addFarmRemovalTracker( trackerTmp ) ;
                    
                }
            
            }
            
        }
        
        // transfer manager now takes care of trackerExitManager
        transferManager->setBirdExitTrackerManager( trackerExitManager ) ;

        
    }
    
    
    // setup completed crop trackers
    {
        
        requireJsonKey( paramsModel["outputParams"], "completedCropAnalyses" );

        trackerCropManager = new CompletedCropTrackerManager();
        
        for ( auto& analysis: paramsModel["outputParams"]["completedCropAnalyses"].items() ) {
            
            std::string nameAnalysis = analysis.key();
            auto& info = analysis.value();
                        
            // tracks prevalence by setting type, periodically and at specific hours of the day
            if ( nameAnalysis == "epiStatsCrops" ) {
                
                requireJsonKey( info, "pathToFile" );
                requireJsonKey( info, "fileName" );
                requireJsonKey( info, "fileExtension" );
                
                // create stream
                std::string pathToFile = info["pathToFile"].get<std::string>();
                std::string fileName = info["fileName"].get<std::string>();
                std::string fileExtension = info["fileExtension"].get<std::string>();
                
                NamedOfstream outFile( pathToFile, fileName, fileExtension, scenarioID, simID );
                                
                // create tracker
                EpiStatsCropTracker* tracker = new EpiStatsCropTracker( outFile );

                // register tracker to tracker managers
                trackerCropManager->addTracker( tracker );

            }
        }
                
        // transfer manager now takes care of trackerExitManager
        transferManager->setCompletedCropTrackerManager( trackerCropManager );
        
    }
    
    
    // make middlemen patrol areas for the first time
    for ( NodeMman* nmm: *mmanVec )
        nmm->forceMoveToNewAreasRequest( randGen, *areaMngr );
       
    //=== other stuff
    
    // initialize functors to null functions
    sparkInfectionUpdate = []( HoldsBirds&, const StrainIdType&, const double&, const Clock&, Random& ) {};
    forceSparkInfectionUpdate = []( HoldsBirds&, const StrainIdType&, const double&, const Clock&, Random& ) {};
    
    // prepare lists of indices (in order to randomize order of actors when needed)
    {
        uint nFarms = static_cast<uint>( farmVec->size() );
        farmIdxs = {};
        farmIdxs.reserve( nFarms );
        for ( uint i = 0; i < nFarms; ++i )
            farmIdxs.push_back( i );
    }
    
    {
        uint nMmen = static_cast<uint>( mmanVec->size() );
        mmanIdxs = {};
        mmanIdxs.reserve( nMmen );
        for ( uint i = 0; i < nMmen; ++i )
            mmanIdxs.push_back( i );
    }
    
    {
        uint nVendors = static_cast<uint>( vendorVec->size() );
        vendorIdxs = {};
        vendorIdxs.reserve( nVendors );
        for ( uint i = 0; i < nVendors; ++i )
            vendorIdxs.push_back( i );
    }
    
    // get maximum farm size
    maxFarmSize = 0;
    for ( auto& nf: *farmVec ) {
        uint farmSize = nf->getContext()->getCapacity();
        if ( farmSize > maxFarmSize )
            maxFarmSize = farmSize;
    }
    
    // call one-shot cross-sectional trackers here
    trackerCSManager->collectInfoT0( this );
    
}

void WorldSimulator::setEpiModel( BaseCompartmentalModel* compModel_ ) {
    compModel = compModel_;
    
    if ( compModel == nullptr )
        return;
    
    uint nStrains = compModel->getNstrains();
    assert( nStrains > 0 );
    
    // modify base transmission weights by place
    
    for ( auto& node: *placeVec ) {
        
        node->setBaseWeight( compModel->getNodeWeight( node->getNodeType() ) );
        
    }
    
    
    // tracks individual infection events?
    bool tracksIndividualEvents = false;
    if ( tracker != nullptr ) {
        
        tracksIndividualEvents = true;
        
        // also tracking individual lineages?
        
        if ( lngTrackerMngr != nullptr )
            compModel->setLineageTrackerManager( lngTrackerMngr ) ;
        
    }
    
    //=== attach functions modifying single place
    bool doesDirectTransmit = false;
    for ( uint strain = 0; strain < nStrains; ++strain ) {
        double tr = compModel->getTransmissibility( strain );
        if ( tr > 0. ) {
            doesDirectTransmit = true;
            break;
        }
    }
    if ( doesDirectTransmit ) {
        if ( tracksIndividualEvents )
            singleSettingUpdates.push_back( std::bind( &runBirdTransmissionSingleSettingWithTracking, std::placeholders::_1, std::ref( clock ), std::ref( *compModel ), std::placeholders::_2, std::ref( tracker ) ) );
        else
            singleSettingUpdates.push_back( std::bind( &runBirdTransmissionSingleSetting, std::placeholders::_1, std::ref( clock ), std::ref( *compModel ), std::placeholders::_2 ) );
    }
    
    
    bool doesEnvironment = false;
    if ( compModel->getEnvTransmissibility() > 0 or compModel->getBirdSheddingProb() > 0 or compModel->getEnvDecayProb() > 0 )
        doesEnvironment = true;

    
    if ( doesEnvironment ) { // set up everything needed to run Environment dynamics
        if ( tracksIndividualEvents ) {
            singleSettingUpdates.push_back( std::bind( &runEnvTransmissionSingleSettingWithTracking, std::placeholders::_1, std::ref( clock ), std::ref( *compModel ), std::placeholders::_2, std::ref( tracker ) ) );
            
            uint initialCapacityEnvironment = 1000;
            // set up contaminated environments (must allow tracking infections)
            for ( auto place: *placeVec )
                place->getUnderlyingSetting()->setContEnv( new ContEnvDetailed( nStrains, initialCapacityEnvironment ) );
            
        }
        else {

            singleSettingUpdates.push_back( std::bind( &runEnvTransmissionSingleSetting, std::placeholders::_1, std::ref( clock ), std::ref( *compModel ), std::placeholders::_2 ) );

            // set up contaminated environments (does not allow tracking infections)            
            
            for ( auto place: *placeVec ) {
                
                place->getUnderlyingSetting()->setContEnv( new ContEnv( nStrains ) );
            
            }
            
        }
        
    }
    
    if ( !tracksIndividualEvents ) {    // no tracking
        sparkInfectionUpdate = std::bind( &sparkInfectionSingleSetting, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::ref( this->clock ), std::ref( *compModel ), std::placeholders::_5 );
        forceSparkInfectionUpdate = std::bind( &forceSparkInfectionSingleSetting, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::ref( this->clock ), std::ref( *compModel ), std::placeholders::_5 );
    }
    else {                              // tracking
        sparkInfectionUpdate = std::bind( &sparkInfectionSingleSettingWithTracking, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::ref( getClock() ), std::ref( *compModel ), std::placeholders::_5, std::ref( tracker ) );
        forceSparkInfectionUpdate = std::bind( &forceSparkInfectionSingleSettingWithTracking, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::ref( getClock() ), std::ref( *compModel ), std::placeholders::_5, std::ref( tracker ) );
    }
   

    // attach functions performing different analyses
    if ( singleSettingUpdates.size() > 0 )
        this->doTransmissionWithinSingleSetting = std::bind( &WorldSimulator::executeAllSingleSettingUpdates, this, std::placeholders::_1 );
    else
        this->doTransmissionWithinSingleSetting = std::bind( &WorldSimulator::doNothing, this, std::placeholders::_1 );
    
    //=== between farm transmission
    
    // set SpatialFarmTransmissionManager
    farmTransmissionMngr = nullptr;
    
    std::function<double (const double&)> farmTransmissionKernel = compModel->getFarmTransmissionKernel();
    
    if ( farmTransmissionKernel != nullptr ) {
        
        BaseInfectionTracker* infTracker = nullptr ;
        if ( tracksIndividualEvents )
            infTracker = tracker ;
        
        double lam     = compModel->getAdaptiveGridLambda();
        double expon_a = compModel->getFarmExponentA();
        double expon_b = compModel->getFarmExponentB();
        
        farmTransmissionMngr = new SpatialFarmTransmissionManager( lam, farmVec, farmTransmissionKernel, expon_a, expon_b, infTracker );
                
    }
    

}

void WorldSimulator::doEpiTransitionEvents( Random& randGen ) {
    compModel->applyTransitionEvents( clock, randGen );
}

 
void WorldSimulator::endSimulation( bool resetWorld ) {

    infectedFarmIdxs.clear();
    
    // flush outputs
    transferManager->flushEverything();
        
    // reset clock
    clock.reset();
    
    if ( resetWorld ) {                         // reset entire simulator for new simulation
        
        resetSimulator() ;
        simID++ ;

        // open new output files
                
        // reset output
        transferManager->reset( true );
        
        // update incidence tracker
        if ( tracker != nullptr )
            tracker->reset( true );
        
        // update cross-sectional trackers
        if ( trackerCSManager != nullptr )
            trackerCSManager->reset( true );

        if ( lngTrackerMngr != nullptr )
            lngTrackerMngr->reset() ;
        
        // update bird-exit trackers // !!! is this ok?
        //if ( trackerExitManager != nullptr )
        //    trackerExitManager->reset( true );
    }
    else {

        // reset output
        transferManager->reset( false ) ;
        
        // update incidence tracker
        if ( tracker != nullptr )
            tracker->reset( false ) ;
        
        // update cross-sectional trackers
        if ( trackerCSManager != nullptr )
            trackerCSManager->reset( false ) ;
        
        if ( lngTrackerMngr != nullptr )
            lngTrackerMngr->reset() ;

        // update bird-exit trackers
        //if ( trackerExitManager != nullptr )
        //    trackerExitManager->reset( false );
    }
}

void WorldSimulator::doTimeStep( Random &randGen, bool doEpi ) {
    
    // analyse output ( cross sectional analysis BEFORE movements )
    this->doCrossSectionalTrackingStep( true );
    
    // run epidemic dynamics
    if ( doEpi )
        this->doEpiStep( randGen );
    
    // run actor dynamics
    this->doPDNStep( randGen );
    
    // analyse output ( cross sectional analysis AFTER movements )
    this->doCrossSectionalTrackingStep( false );
    
    // internal clock ticks
    clock.advance();
    
}

void WorldSimulator::doPDNStep( Random &randGen ) {
   
    //uint day = clock.getCurrDay();
    uint h = clock.getCurrHour();
    
    if ( h == MARKET_OPENING_HOUR ) {
        
        // --> open markets
        for ( NodeMarket* nm: *marketVec ) {
            nm->openMarket( clock );
        }
        
        // shuffle vendors
        shuffleVendorIdxs( randGen );
        
        // --> notify vendors
        for ( uint idx: vendorIdxs ) {
            NodeVendor* nv = ( *vendorVec )[ idx ];
            nv->goToMarketRequest( randGen );
        }
        
        /*
        for ( NodeMarket* nm: *marketVec ) {
            nm->printBuyersDemography();
        }*/
         
        
        // -> notify middlemen
        // ----> middlemen sell birds to vendors in markets
        
        // shuffle middlemen
        shuffleMmanIdxs( randGen ); 
        
        //std::cout << "birds held by middlemen (before trade): " << computeTotalBirdsMiddlemen() << std::endl;
        
        for ( uint& idx: mmanIdxs ) {
            NodeMman* nmm = ( *mmanVec )[ idx ];
            nmm->sellToMarketsRequest( randGen, *transferManager, *marketVec );
        }
    
        //std::cout << "unsold mman birds: " << computeTotalBirdsMiddlemen() << std::endl;
        
        //std::cout << "birds held by middlemen (after trade): " << computeTotalBirdsMiddlemen() << std::endl;
        //std::cout << "birds in markets: " << computeTotalBirdsInMarkets() << std::endl;
        //std::cout << "infected birds market: " << computeTotalInfectiousBirdsInMarkets() << std::endl;

        
        
        // do wholesaling and vendor movement step
        for ( uint layer = 0; layer < nVendorLayers; ++layer ) {
            for ( auto& nm: ( *marketVec ) ) {

                //std::cout << "market " << nm->getContext()->getId() << " " << nm->getSizeBirds() << std::endl;
                nm->doVendorDisplacementStep( layer, randGen );
                nm->doWholesaleTradingStep( layer, clock, randGen, *transferManager );

            }
        
        }
        
    }
    // if t == 12 p.m.
    if ( h == FARM_DAILY_CHECK_HOUR ) {
        
        shuffleFarmIdxs( randGen );
        
        // -> notify farms
                
        for ( uint& idx: farmIdxs ) {
            NodeFarm* nf = (*farmVec)[idx];
            nf->isEmptyRequest( *areaMngr, clock, randGen, trackerCropManager );                    // check if farm is empt
            nf->refillRequest( clock, randGen, birdMngr );                // farms add new batches to own flock
            nf->stageCropRequest( clock, randGen, *areaMngr, trackerCropManager );  // check for ready batches
            nf->scheduleTodayTradeRequest( clock, randGen, transferManager, trackerCropManager ); // schedule how many birds to trade today
            
        }
        
        // -> update areaMngr
        areaMngr->updateFarmProductionDistribution();
       
        
        // -> notify middlemen
        for ( uint& idx: mmanIdxs ) {
            
            NodeMman* nmm = (*mmanVec)[idx];
                        
            // ----> middlemen might change region
            nmm->moveToNewAreasRequest( randGen, *areaMngr );
            
            // ----> middlemen ask farms for chickens ( task creation   )
            nmm->buyFromFarmsRequest( randGen, *areaMngr );
            
            // ----> middlemen schedule tasks         ( task scheduling )
            nmm->executeScheduleTasksRequest( clock, randGen );

        }
       
    }
    
    if ( h == MARKET_CLOSING_HOUR ) {
        
        // -> close markets
        for ( NodeMarket* nm: *marketVec ) {
            nm->closeMarket();
        }
        //assert( computeTotalBirdsInMarkets() == 0 );
        //std::cout << "returned birds: " << computeVendorTotalStashedBirds() << std::endl;
    }
    
    // from 6 a.m. to 17 p.m.
    // -> notify vendors
    // ----> sellers trade with either buyers or the community
    
    if ( h > MARKET_OPENING_HOUR and h < MARKET_CLOSING_HOUR ) {
        
        // ?? was h >= MARKET_OPENING_HOUR (done so that I can track all chickens entering market)
        
        /*
        for ( NodeMarket* nm: *marketVec ) {
            nm->printBuyersDemography();
        }
        */
        
        for ( NodeMarket* nm: *marketVec ) {
            
            /*
            if ( h <= MARKET_OPENING_HOUR + 1  )
                std::cout << nm->getContext()->getId() << " " << nm->getBuyers( BirdType::broiler ).size() << std::endl;
            */
            
            nm->doTradingStepRequest( clock, randGen, *transferManager );
            
        }
        
    }

    // from 12 p.m. to 5 a.m.
    // -> notify middlemen
    // ----> middlemen perform collection tasks
    // ----> middlemen might sell part of their cargo to other middlemen
    
    for ( NodeMman* nmm: *mmanVec ) {
        
        //uint nbirds_b = nmm->getSizeBirds();

        nmm->executeCollectionTasksRequest( clock, randGen, *transferManager );
        
        /*
        uint nbirds_a = nmm->getSizeBirds();
        
        if ( nbirds_a != nbirds_b ) {
            std::cout << "mman " << nmm->getContext()->getId() << " collected at h = " << h << ", " << nbirds_a << "<-" << nbirds_b << std::endl;
        }*/

    }
    //std::cout << "birds held by middlemen h: " << h << " " << computeTotalBirdsMiddlemen() << std::endl;
    
    // update field experimenters (if any)
    for ( NodeFieldExperimenter* nfe : *fieldExperimenterVec )
        nfe->goToNextStageExperimentRequest( clock, randGen, *transferManager ) ;
    
}

// runs epi dynamics for a single timestep
void WorldSimulator::doEpiStep( Random &randGen ) {
    
    // perform within-setting transmission and environmental dynamics
    doTransmissionWithinSingleSetting( randGen );
    
    // perform epi transmission between farms
    if ( checkTimePeriodic( clock, 24, 0 ) ) // triggers every 24h at h = 0 // ??? might change this
        doBetweenFarmTransmission( randGen );
    
    // perform other epi events (incubation, recovery, disease-induced mortality)
    doEpiTransitionEvents( randGen );
    
}

// enable cross sectional analyses
void WorldSimulator::doCrossSectionalTrackingStep( bool preMovements ) {
    
    if ( preMovements )
        trackerCSManager->collectInfoPreMovements( this ) ;
    else
        trackerCSManager->collectInfoPostMovements( this ) ;
    
}

// run transmission between farms
void WorldSimulator::doBetweenFarmTransmission( Random &randGen ) {
    
    if ( farmTransmissionMngr != nullptr ) {
        
        farmTransmissionMngr->doTransmissionStep( clock, randGen, compModel );
        
    }
    
}

// N.B. does not affect output ( which is managed by 'endSimulation' )
void WorldSimulator::resetSimulator() {
    
    // reset places
    
    for ( NodeFarm* nf: *farmVec )
        nf->resetRequest( birdMngr );
    
    for ( NodeMman* nmm: *mmanVec )
        nmm->resetRequest( birdMngr );
    
    for ( NodeMarket* nm: *marketVec )
        nm->resetRequest( birdMngr );
    
    for ( NodeVendor* nv: *vendorVec )
        nv->resetRequest( birdMngr );
    
    for ( NodeFieldExperimenter* nfe: *fieldExperimenterVec )
        nfe->resetRequest( birdMngr ) ;
    
    // reset epi stuff
    
    if ( compModel != nullptr )
        compModel->reset();
    
    if ( farmTransmissionMngr != nullptr )
        farmTransmissionMngr->reset();
    
}

// execute all required setting updates
void WorldSimulator::executeAllSingleSettingUpdates( Random& randGen ) {
    for ( auto& place: *placeVec ) {
        for ( auto& updateFunc: singleSettingUpdates ) {
            updateFunc( *( place->getUnderlyingSetting() ), randGen );
        }
    }
}

//====== Methods to seed infection artificially ======//

// selects a random farm and infects up to 'nbirds' naive birds with pathogen 'strain'
void WorldSimulator::forceSparkInfectionRandomFarm( const uint& nInfections, const StrainIdType& strain, Random& randGen ) {
    
    if ( farmVec->size() == 0 or nInfections == 0 )
        return; // no farms to infect, or no infections at all

    uint nAttempts = 0;
    while ( nAttempts < 50 ) {
        uint farmIdx = randGen.getUniInt( static_cast<uint>( farmVec->size() - 1 ) );
        HoldsBirds* setting = farmVec->at( farmIdx )->getUnderlyingSetting();   // select random farm
        
        if ( setting->getSizeBirds() > 0 )
            forceSparkInfectionUpdate( *setting, strain, nInfections, clock, randGen );
        else    // increase attempt counter
            ++nAttempts;
    }

}

// selects a random bird within each farm, if occupied, and updates hazard of
// that bird (hence this method accounts for a bird's susceptibility)
void WorldSimulator::sparkInfectionFarms( const StrainIdType &strain, const double &sparkRate, Random &randGen ) {
        
    for ( NodeFarm* nf: *farmVec ) {
        Farm& farm = *(nf->getContext());
        sparkInfectionUpdate( farm, strain, sparkRate, clock, randGen );
    }
    
}

// uses a map to assign which strain can be introduced into which area
// introduction rate depends on number of farms in area
void WorldSimulator::sparkInfectionFarmsAreaSpecific( std::map<uint,StrainIdType>& area2strain, const double& sparkRate, Random& randGen ) {
        
    for ( NodeFarm* nf: *farmVec ) {
        
        uint areaId = nf->getAreaId() ;
        uint nFarms = areaMngr->getTotalFarms( areaId ) ;
        
        if ( area2strain.count( areaId ) > 0 ) {
        
            StrainIdType strain = area2strain[ areaId ];
            Farm& farm = *( nf->getContext() );
            sparkInfectionUpdate( farm, strain, sparkRate / nFarms, clock, randGen );
            
        }
    
    }
    
    
}


//====== Miscellanea ======//

// Compute total number of birds currently in the hands of middlemen
uint WorldSimulator::computeTotalBirdsMiddlemen() {
    uint res = 0;
    for ( auto& nm: *mmanVec ) {
        res += nm->getSizeBirds();
    }
    return res;
}

// Compute total number of birds currently within markets
uint WorldSimulator::computeTotalBirdsInMarkets() {
    uint res = 0;
    for ( auto& nm: *marketVec ) {
        res += nm->getSizeBirds();
    }
    return res;
}

// Compute total number of birds currently stored away by vendors
// should return 0 if all vendors are currently working in markets
uint WorldSimulator::computeVendorTotalStashedBirds() {
    uint res = 0;
    for ( auto& nv: *vendorVec ) {
        res += nv->getSizeBirds();
    }
    return res;
}

// checks whether only desired strains circulate in farms in a given are
void WorldSimulator::checkViralMixingFarms( std::map<uint,StrainIdType>& area2strain ) {
    
    for ( auto& nf : *farmVec ) {
        
        uint areaId = nf->getAreaId() ;
        StrainIdType strainCheck = area2strain[areaId] ;
        
        auto& birds = nf->getContext()->getBirds() ;
        for ( auto it = birds.begin(); it < birds.endInf(); ++it ) {
            
            StateType infState =  (*it)->infState ;
            for ( StrainIdType s : compModel->getStrainList( infState ) ) {
                
                assert( s == strainCheck );
                
            }
            
        }
        
    }
    
}

uint WorldSimulator::computeTotalInfectiousBirdsInMarkets() {
    
    uint res = 0 ;
    
    for ( auto& nm: *marketVec ) {
    
        res += nm->getSizeInfectedBirds() ;
    
    }
    return res ;
    
}


void WorldSimulator::setupFieldExperiment( nlohmann::json experiment_args ) {
    
    uint nTotalExperiments = 0 ;
    
    for ( auto& elem: experiment_args.items() ) { // loop over experiments
        
        // parse experiment details
        nlohmann::json single_experiment_args = elem.value() ;
        
        requireJsonKey( single_experiment_args, "birdType" ) ;              // Bird type recruited
        requireJsonKey( single_experiment_args, "nRecruitsFarm" ) ;         // Number of birds recruited from farms
        requireJsonKey( single_experiment_args, "nRecruitsMarket" ) ;       // Number of birds recruited from markets
        requireJsonKey( single_experiment_args, "dtShelter" ) ;             // Duration of sheltering
        requireJsonKey( single_experiment_args, "dayExperiment" ) ;         // Day where market stage starts
        requireJsonKey( single_experiment_args, "dtExperiment" ) ;          // Duration of experiment
        requireJsonKey( single_experiment_args, "marketExperimentID" ) ;    // Id of market

        
        std::string birdTypeStr = single_experiment_args["birdType"].get<std::string>() ;
        uint nRecruitsFarm      = single_experiment_args["nRecruitsFarm"].get<uint>() ;
        uint nRecruitsMarket    = single_experiment_args["nRecruitsMarket"].get<uint>() ;
        uint dtShelter          = single_experiment_args["dtShelter"].get<uint>() ;
        uint dayExperiment      = single_experiment_args["dayExperiment"].get<uint>() ;
        uint dtExperiment       = single_experiment_args["dtExperiment"].get<uint>() ;
        uint marketExperimentID = single_experiment_args["marketExperimentID"].get<uint>() ;

        BirdType birdType   = constants::getBirdTypeFromString( birdTypeStr ) ;
        NodeMarket* nm      = marketVec->at( marketExperimentID ) ;
        
        // create a single experimenter object
        
        NodeFieldExperimenter* nfe = new NodeFieldExperimenter( fieldExperimenterStates->getStateRecruiting(), birdType, nRecruitsFarm, nRecruitsMarket, dtShelter, dayExperiment, dtExperiment, nm, farmVec ) ;
        
        // add experiment object to simulator
        fieldExperimenterVec->push_back( nfe ) ;
        
    }    
    
}

