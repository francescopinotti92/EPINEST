//
//  cross_sectional_tracker.cpp
//  CBM
//
//

#include "cross_sectional_tracker.hpp"

//====== AggregateBirdCountSettingTracker ======//

AggregateBirdCountSettingTracker::AggregateBirdCountSettingTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    if ( outFile.isClosed() )
        this->flush = std::bind( &AggregateBirdCountSettingTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &AggregateBirdCountSettingTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &AggregateBirdCountSettingTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
    countBySetting.clear();
    
}

void AggregateBirdCountSettingTracker::collectInfo( WorldSimulator* simulator ) {

    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& places = simulator->getPlaces();
    
    countBySetting.clear();
    for ( auto& place: places ) {
        NodeType settingType = place->getNodeType();
        countBySetting[settingType] += place->getSizeBirds();
    }
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}

void AggregateBirdCountSettingTracker::reset( bool updateStream ) {
    tLastOutputDay = 0;
    countBySetting.clear();
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

void AggregateBirdCountSettingTracker::flushWithPrint( Clock &clock ) {
    for ( auto& elem: countBySetting ) {
        outFile.getStream() << fmt::format("{},{},{}\n", clock.getCurrTime(),
                                constants::getStringFromNodeType( elem.first ),
                                elem.second );
    }
}


//====== AggregatePrevalenceSettingTracker ======//

AggregatePrevalenceSettingTracker::AggregatePrevalenceSettingTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    if ( outFile.isClosed() )
        this->flush = std::bind( &AggregatePrevalenceSettingTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &AggregatePrevalenceSettingTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &AggregatePrevalenceSettingTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
    countBySetting.clear();
    countTotBySetting.clear();
    
}

void AggregatePrevalenceSettingTracker::collectInfo( WorldSimulator* simulator ) {

    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& places = simulator->getPlaces();
    
    countBySetting.clear();
    countTotBySetting.clear();

    for ( auto& place: places ) {
        NodeType settingType = place->getNodeType();
        countBySetting[settingType] += place->getSizeInfectedBirds();
        countTotBySetting[settingType] += place->getSizeBirds();
    }

    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}

void AggregatePrevalenceSettingTracker::reset( bool updateStream ) {
    tLastOutputDay = 0;
    countBySetting.clear();
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    countTotBySetting.clear();

}

void AggregatePrevalenceSettingTracker::flushWithPrint( Clock &clock ) {
    for ( auto& elem: countBySetting ) {
        outFile.getStream() << fmt::format("{},{},{},{}\n", clock.getCurrTime(),
                                constants::getStringFromNodeType( elem.first ),
                                elem.second, countTotBySetting[elem.first] );
    }
}

//====== InfectedFarmsTracker ======//

InfectedFarmsTracker::InfectedFarmsTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const std::string& sep_ ): BaseCrossSectionalTracker( outFile, deltaTdays, hs ), sep( sep_ ), farmIdxs( 1000 ) {
    
    if ( outFile.isClosed() )
        this->flush = std::bind( &InfectedFarmsTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &InfectedFarmsTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &InfectedFarmsTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void InfectedFarmsTracker::collectInfo( WorldSimulator* simulator ) {

    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& farms = simulator->getFarmList();
    
    farmIdxs.clear();
    farmIdxs.increaseCapacity( static_cast<uint>( farms.size() ) ); //
    
    for ( auto& farm: farms ) {
        if ( farm->getSizeInfectedBirds() > 0 )
            farmIdxs.insert( farm->getContext()->getId() );
    }
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
}

void InfectedFarmsTracker::reset( bool updateStream ) {
    tLastOutputDay = 0;
    farmIdxs.clear();
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

void InfectedFarmsTracker::flushWithPrint( Clock &clock ) {
    if ( !farmIdxs.empty() )
        outFile.getStream() << clock.getCurrTime() << sep << stringify_vec( farmIdxs, sep ) << "\n";
}

//====== AggregateSusceptibilitySettingTracker ======//


AggregateImmunitySettingTracker::AggregateImmunitySettingTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
        
    if ( outFile.isClosed() )
        this->flush = std::bind( &AggregateImmunitySettingTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &AggregateImmunitySettingTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &AggregateImmunitySettingTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
    countByStateBySetting.clear();
    countChickensBySetting.clear();
    
}

void AggregateImmunitySettingTracker::reset( bool updateStream ) {
        
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    countByStateBySetting.clear();
    countChickensBySetting.clear();

    
}

void AggregateImmunitySettingTracker::collectInfo( WorldSimulator* simulator ) {
    
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& places = simulator->getPlaces();
    
    countByStateBySetting.clear();
    for ( NodeType nt : allNodeTypes )
        countByStateBySetting[nt] = std::map<StateType,uint>();
    countChickensBySetting.clear();
    
    
    for ( auto& place: places ) {
        
        auto& birds = place->getUnderlyingSetting()->getBirds();
        NodeType settingType = place->getNodeType();
        
        countChickensBySetting[settingType] += place->getSizeBirds();
        auto& tmp =countByStateBySetting[settingType];

        for ( auto it = birds.cbegin(); it < birds.cend(); ++it ) {
            
            StateType immState = (*it)->immState;
            tmp[immState]++;
            
        }
        
    }
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}


void AggregateImmunitySettingTracker::flushWithPrint( Clock &clock ) {
    
    uint t = clock.getCurrTime();
    for ( auto& elem: countByStateBySetting ) {
        
        std::string nt = constants::getStringFromNodeType( elem.first );
        auto& countByState = elem.second;
        
        for ( auto& elem2: countByState )
            outFile.getStream() << fmt::format("{},{},{},{},{}\n", t, nt, elem2.first, elem2.second, countChickensBySetting[elem.first] );
        
    }
    
}

//====== BirdMixingMarketTracker ======//

AggregateInfectionMultiplicityBySettingTracker::AggregateInfectionMultiplicityBySettingTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    if ( outFile.isClosed() )
        this->flush = std::bind( &AggregateInfectionMultiplicityBySettingTracker::doNothing, this, std::placeholders::_1 ) ;
    else
        this->flush = std::bind( &AggregateInfectionMultiplicityBySettingTracker::flushWithPrint, this, std::placeholders::_1 ) ;
    
    checkTime = std::bind( &AggregateInfectionMultiplicityBySettingTracker::checkTimePeriodic, this, std::placeholders::_1 ) ;
    
    countByMultiplicityBySetting.clear() ;
    
}

void AggregateInfectionMultiplicityBySettingTracker::collectInfo( WorldSimulator* simulator ) {
    
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& places = simulator->getPlaces();
    
    countByMultiplicityBySetting.clear();
    for ( NodeType nt : allNodeTypes )
        countByMultiplicityBySetting[nt] = std::map<uint,uint>();
    countByMultiplicityBySetting.clear();
    
    BaseCompartmentalModel& epiModel = simulator->getCompModel() ;
    
    for ( auto& place: places ) {
        
        auto& birds = place->getUnderlyingSetting()->getBirds();
        NodeType settingType = place->getNodeType();
        
        auto& tmp = countByMultiplicityBySetting[settingType];

        for ( auto it = birds.cbegin(); it < birds.cendInf(); ++it ) {
            
            StateType infState = (*it)->infState ;
            uint multip   = static_cast<uint>( epiModel.getStrainList( infState ).size() ) ;
            //std::cout << infState << " " << (*it)->immState << " " << multip << std::endl ;
            //assert( multip > 0 ) ;
            
            tmp[multip]++;
            
        }
        
    }
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}

void AggregateInfectionMultiplicityBySettingTracker::reset( bool updateStream )  {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    countByMultiplicityBySetting.clear() ;

    
}

void AggregateInfectionMultiplicityBySettingTracker::flushWithPrint( Clock& clock ) {
    
    uint t = clock.getCurrTime() ;
    
    for ( auto& elem: countByMultiplicityBySetting ) {
        
        std::string nt = constants::getStringFromNodeType( elem.first );
        auto& countByMultip = elem.second;
        
        for ( auto& elem2: countByMultip )
            // print time, nodetype, multiplicity, count
            outFile.getStream() << fmt::format("{},{},{},{}\n", t, nt, elem2.first, elem2.second );
        
    }
    
}


//====== AggregateRichnessMarkets ======//

AggregateRichnessMarkets::AggregateRichnessMarkets( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    strainsFound = {} ;
    
    // flush protocol
    if ( outFile.isClosed() )
        this->flush = std::bind( &AggregateRichnessMarkets::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &AggregateRichnessMarkets::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &AggregateRichnessMarkets::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void AggregateRichnessMarkets::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    strainsFound = {} ;
    
}

void AggregateRichnessMarkets::collectInfo( WorldSimulator* simulator ) {

    strainsFound = {} ;
    
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return ;
   
    auto& markets = simulator->getMarketList(); // get market list
    BaseCompartmentalModel& epiModel = simulator->getCompModel() ; // get epi model
    
    for ( auto& market : markets ) {
    
        auto& birds = market->getContext()->getBirds() ;
        
        for ( auto it = birds.cbegin(); it < birds.cendInf(); ++it ) {
            
            StateType infState = (*it)->infState ;
            auto& strains = epiModel.getStrainList( infState ) ;
            for ( auto& strain : strains )
                strainsFound.insert( strain ) ;
            
        }
    
    }
   
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}


void AggregateRichnessMarkets::flushWithPrint( Clock& clock ) {
    
    auto& out = outFile.getStream();
    out << fmt::format( "{},{}\n", clock.getCurrTime(), strainsFound.size() );
        
}


//====== AggregateSimpsonDiversityMarkets ======//

AggregateSimpsonDiversityMarkets::AggregateSimpsonDiversityMarkets( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    strainAbundance = {} ;
    
    // flush protocol
    if ( outFile.isClosed() )
        this->flush = std::bind( &AggregateSimpsonDiversityMarkets::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &AggregateSimpsonDiversityMarkets::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &AggregateSimpsonDiversityMarkets::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void AggregateSimpsonDiversityMarkets::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    strainAbundance = {} ;
    
}

void AggregateSimpsonDiversityMarkets::collectInfo( WorldSimulator* simulator ) {

    strainAbundance = {} ;
    
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return ;
   
    auto& markets = simulator->getMarketList(); // get market list
    BaseCompartmentalModel& epiModel = simulator->getCompModel() ; // get epi model
    
    for ( auto& market : markets ) {
    
        auto& birds = market->getContext()->getBirds() ;
        
        for ( auto it = birds.cbegin(); it < birds.cendInf(); ++it ) {
            
            StateType infState = (*it)->infState ;
            auto& strains = epiModel.getStrainList( infState ) ;
            for ( auto& strain : strains )
                strainAbundance[ strain ]++ ;
            
        }
    
    }
   
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}


void AggregateSimpsonDiversityMarkets::flushWithPrint( Clock& clock ) {
    
    auto& out = outFile.getStream();
    
    // count total number of observations
    uint nObs = 0 ;
    for ( auto& tmp : strainAbundance )
        nObs += tmp.second ;
    
    if ( nObs > 0 ) { // something found
        double simpsonEvenness = 0. ;
        for ( auto& tmp : strainAbundance ) {
            uint ni = tmp.second ;
            simpsonEvenness += pow( ni * 1. / nObs, 2. ) ;
            
        }
        double simpsonDiversity = 1. - simpsonEvenness ;
        out << fmt::format( "{},{}\n", clock.getCurrTime(), simpsonDiversity ) ;

        
        
    }
    else // nothing found
        out << fmt::format( "{},{}\n", clock.getCurrTime(), 0. ) ;
        
}


//====== RichnessSingleMarkets ======//

RichnessSingleMarkets::RichnessSingleMarkets( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    strainsFoundByMarket = {} ;
    
    // flush protocol
    if ( outFile.isClosed() )
        this->flush = std::bind( &RichnessSingleMarkets::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &RichnessSingleMarkets::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &RichnessSingleMarkets::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void RichnessSingleMarkets::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    strainsFoundByMarket = {} ;
    
}

void RichnessSingleMarkets::collectInfo( WorldSimulator* simulator ) {

    strainsFoundByMarket = {} ;
    
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return ;
   
    auto& markets = simulator->getMarketList(); // get market list
    BaseCompartmentalModel& epiModel = simulator->getCompModel() ; // get epi model
    
    for ( auto& market : markets ) {
    
        strainsFoundByMarket[ market->getContext()->getId() ] = std::unordered_set<StrainIdType>() ;
        std::unordered_set<StrainIdType>& strainsFound = strainsFoundByMarket[ market->getContext()->getId() ] ;
        auto& birds = market->getContext()->getBirds() ;
        
        for ( auto it = birds.cbegin(); it < birds.cendInf(); ++it ) {
            
            StateType infState = (*it)->infState ;
            auto& strains = epiModel.getStrainList( infState ) ;
            for ( auto& strain : strains )
                strainsFound.insert( strain ) ;
            
        }
    
    }
   
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}


void RichnessSingleMarkets::flushWithPrint( Clock& clock ) {
    
    auto& out = outFile.getStream();
    
    for ( auto& tmp : strainsFoundByMarket )
        out << fmt::format( "{},{},{}\n", clock.getCurrTime(), tmp.first, tmp.second.size() ) ;
        
}

//====== RichnessSingleAreas ======//

RichnessSingleAreas::RichnessSingleAreas( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    strainsFoundByArea = {} ;
    
    // flush protocol
    if ( outFile.isClosed() )
        this->flush = std::bind( &RichnessSingleAreas::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &RichnessSingleAreas::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &RichnessSingleAreas::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void RichnessSingleAreas::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
    strainsFoundByArea = {} ;
    
}

void RichnessSingleAreas::collectInfo( WorldSimulator* simulator ) {

    strainsFoundByArea = {} ;
    for ( uint a = 0, Na = simulator->getNumberOfAreas() ; a < Na ; ++a )
        strainsFoundByArea[a] = std::unordered_set<StrainIdType>() ;
    
    
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return ;
   
    auto& farms = simulator->getFarmList(); // get market list
    BaseCompartmentalModel& epiModel = simulator->getCompModel() ; // get epi model
    
    
    for ( auto& farm : farms ) {
    
        std::unordered_set<StrainIdType>& strainsFound = strainsFoundByArea[ farm->getAreaId() ] ;
    
        auto& birds = farm->getContext()->getBirds() ;
        
        for ( auto it = birds.cbegin(); it < birds.cendInf(); ++it ) {
            
            StateType infState = (*it)->infState ;
            auto& strains = epiModel.getStrainList( infState ) ;
            for ( auto& strain : strains )
                strainsFound.insert( strain ) ;
            
        }
    
    }
   
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}


void RichnessSingleAreas::flushWithPrint( Clock& clock ) {
    
    auto& out = outFile.getStream();
    
    for ( auto& tmp : strainsFoundByArea )
        out << fmt::format( "{},{},{}\n", clock.getCurrTime(), tmp.first, tmp.second.size() ) ;
        
}



//====== BirdMixingMarketTracker ======//

BirdMixingMarketTracker::BirdMixingMarketTracker( NamedOfstream outFile, bool infectedOnly, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ), infectedOnly( infectedOnly ) {
    
    // flush protocol
    if ( outFile.isClosed() )
        this->flush = std::bind( &BirdMixingMarketTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &BirdMixingMarketTracker::flushWithPrint, this, std::placeholders::_1 );
    
    // collection protocol
    if ( infectedOnly )
        this->getMixing = std::bind( &BirdMixingMarketTracker::getMixingByFarmInfected, this, std::placeholders::_1 );
    else
        this->getMixing = std::bind( &BirdMixingMarketTracker::getMixingByFarmAll, this, std::placeholders::_1 );
    
    checkTime = std::bind( &BirdMixingMarketTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void BirdMixingMarketTracker::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    birdCts = {};
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
}


void BirdMixingMarketTracker::collectInfo( WorldSimulator* simulator ) {

    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    birdCts = std::vector<std::map<uint,uint>> ( simulator->getMarketList().size(), std::map<uint,uint>() );
    auto& markets = simulator->getMarketList();
    
    for ( auto& market : markets )
        getMixing( market );
   
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}

// counts chickens per farm (considers all chickens)
void BirdMixingMarketTracker::getMixingByFarmAll( NodeMarket* market ) {
    
    auto& birds = market->getContext()->getBirds();
    
    std::map<uint,uint>& cts = birdCts[ market->getContext()->getId() ];;
    
    for ( auto it = birds.cbegin(); it < birds.cend(); ++it ) {
        
        BirdID id = (*it)->id;
        uint idFarm = id.idFarm;
        cts[idFarm]++;
        
    }
        
}

// counts chickens per farm (considers infected chickens only)
void BirdMixingMarketTracker::getMixingByFarmInfected( NodeMarket* market ) {
    
    auto& birds = market->getContext()->getBirds();
    
    std::map<uint,uint>& cts = birdCts[ market->getContext()->getId() ];

    for ( auto it = birds.cbegin(); it < birds.cendInf(); ++it ) {
        
        BirdID id = (*it)->id;
        uint idFarm = id.idFarm;
        cts[idFarm]++;
        
    }
        
}

void BirdMixingMarketTracker::flushWithPrint( Clock& clock ) {
    
    auto& out = outFile.getStream();

    uint nMarkets = static_cast<uint>( birdCts.size() );
    
    for ( uint i = 0; i < nMarkets; ++i ) {
        
        if ( !birdCts[i].empty() ) {
            
            for ( auto& elem : birdCts[i] ) {
                // prints current time, market id, farm id, bird count
                out << fmt::format( "{},{},{},{}\n", clock.getCurrTime(), i, elem.first, elem.second );
                
            }
            
        }
        
    }
        
}

//====== FarmTradedBirdsInfectedTracker ======//

FarmTradedBirdsInfectedTracker::FarmTradedBirdsInfectedTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    checkTime = std::bind( &MmanTradedBirdsByFarmTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void FarmTradedBirdsInfectedTracker::collectInfo( WorldSimulator* simulator ) {

    if ( !checkTime( simulator->getClock() ) ) // checks if it is ok to compute output
        return ;
    
    if ( outFile.isClosed() )
        return ;
    
    
    uint t = simulator->getClock().getCurrTime() ;
    auto& nmms = simulator->getMiddlemanList() ;
    
    for ( auto& nmm : nmms ) {
        
        uint mmanId = nmm->getContext()->getId() ;
        uint nInfectedBirds = nmm->getNIncomingInfectedBirds() ;
        outFile.getStream() << fmt::format("{},{},{}\n", t, mmanId, nInfectedBirds ) ;
        
    }
    
    tLastOutputDay = simulator->getClock().getCurrDay() ;
    
    
}


void FarmTradedBirdsInfectedTracker::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
}


//====== MmanTradedBirdsByFarmTracker ======//

MmanTradedBirdsByFarmTracker::MmanTradedBirdsByFarmTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    checkTime = std::bind( &MmanTradedBirdsByFarmTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void MmanTradedBirdsByFarmTracker::collectInfo( WorldSimulator* simulator ) {

    if ( !checkTime( simulator->getClock() ) ) // checks if it is ok to compute output
        return ;
    
    if ( outFile.isClosed() )
        return ;
    
    
    uint t = simulator->getClock().getCurrTime() ;
    auto& nms = simulator->getMarketList() ;
    
    for ( auto& nm : nms ) {
        uint marketId = nm->getContext()->getId() ;
        std::map<uint,uint> cts = nm->getNMmanTradedBirdsByFarm() ;
        for ( auto& elem : cts ) {
            
            outFile.getStream() << fmt::format("{},{},{},{}\n", t, marketId, elem.first, elem.second ) ;
            
        }
        
    }
    
    tLastOutputDay = simulator->getClock().getCurrDay() ;
    
    
}


void MmanTradedBirdsByFarmTracker::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
}

//====== MmanTradedBirdsInfectedTracker ======//

MmanTradedBirdsInfectedTracker::MmanTradedBirdsInfectedTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    checkTime = std::bind( &MmanTradedBirdsByFarmTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void MmanTradedBirdsInfectedTracker::collectInfo( WorldSimulator* simulator ) {

    if ( !checkTime( simulator->getClock() ) ) // checks if it is ok to compute output
        return ;
    
    if ( outFile.isClosed() )
        return ;
    
    
    uint t = simulator->getClock().getCurrTime() ;
    auto& nms = simulator->getMarketList() ;
    
    for ( auto& nm : nms ) {
        
        uint marketId = nm->getContext()->getId() ;
        uint nInfectedBirds = nm->getNIncomingInfectedBirds() ;
        outFile.getStream() << fmt::format("{},{},{}\n", t, marketId, nInfectedBirds ) ;
        
    }
    
    tLastOutputDay = simulator->getClock().getCurrDay() ;
    
    
}


void MmanTradedBirdsInfectedTracker::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
}

//====== InfectedTradedBirdsTracker ======//


InfectedTradedBirdsTracker::InfectedTradedBirdsTracker( NamedOfstream outFile, const uint& deltaTdays, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, { MARKET_OPENING_HOUR }, tmin ) {
    
    checkTime = std::bind( &InfectedTradedBirdsTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void InfectedTradedBirdsTracker::collectInfo( WorldSimulator* simulator ) {

    if ( !checkTime( simulator->getClock() ) ) // checks if it is ok to compute output
        return ;
    
    if ( outFile.isClosed() )
        return ;
    
    
    uint t = simulator->getClock().getCurrTime() ;
    auto& nms = simulator->getMarketList() ;
    
    uint nBirdsE   = 0 ; // latent birds
    uint nBirdsI   = 0 ; // infectious birds
    uint nBirdsTot = 0 ; // total birds (only newly marketed)
    
    for ( auto& nm : nms ) {

        auto& birds = nm->getContext()->getBirds() ;
        
        for ( auto it = birds.cbegin() ; it < birds.cend() ; ++it ) {
            
            if ( (*it)->t_market == t ) { // select birds marketed now
                
                bool isE = (*it)->isLatent() ;
                bool isI = (*it)->isInfectious() ;
                
                if ( isI )
                    ++nBirdsI ;
                else if ( isE )
                    ++nBirdsE ;
                
                ++nBirdsTot ;
            }
            
        
        }

        
    }
    
    outFile.getStream() << fmt::format("{},{},{},{}\n", simulator->getClock().getCurrDay(), nBirdsE, nBirdsI, nBirdsTot ) ;

    
    tLastOutputDay = simulator->getClock().getCurrDay() ;
    
    
}


void InfectedTradedBirdsTracker::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
}



//====== BatchEpiStatsTracker ======//


StrainMixingMarketTracker::StrainMixingMarketTracker( NamedOfstream outFile, const uint& deltaTdays, const std::vector<uint>& hs, const uint& tmin ): BaseCrossSectionalTracker( outFile, deltaTdays, hs, tmin ) {
    
    // flush protocol
    if ( outFile.isClosed() )
        this->flush = std::bind( &StrainMixingMarketTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &StrainMixingMarketTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &StrainMixingMarketTracker::checkTimePeriodic, this, std::placeholders::_1 );
    
}

void StrainMixingMarketTracker::collectInfo( WorldSimulator* simulator ) {
    
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    strainCts = std::vector<std::map<uint,StrainIdType>> ( simulator->getMarketList().size(), std::map<uint,StrainIdType>() );
    auto& markets = simulator->getMarketList();
    
    for ( auto& market : markets )
        getMixing( market, simulator->getCompModel() );
   
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
    
}

void StrainMixingMarketTracker::reset( bool updateStream  ) {
    
    tLastOutputDay = 0;
    strainCts = {};
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
    
}

void StrainMixingMarketTracker::flushWithPrint( Clock& clock ) {
    
    auto& out = outFile.getStream();

    uint nMarkets = static_cast<uint>( strainCts.size() );
    
    for ( uint i = 0; i < nMarkets; ++i ) {
        
        if ( !strainCts[i].empty() ) {
            
            for ( auto& elem : strainCts[i] ) {
                // prints current time, market id, strain id, strain count
                out << fmt::format( "{},{},{},{}\n", clock.getCurrTime(), i, elem.first, elem.second );
                
            }
            
        }
        
    }
    
    
}

void StrainMixingMarketTracker::getMixing( NodeMarket* market, BaseCompartmentalModel& epiModel ) {
    
    auto& birds = market->getContext()->getBirds();
    
    std::map<uint,StrainIdType>& cts = strainCts[ market->getContext()->getId() ];;
    
    // loop over infected birds
    for ( auto it = birds.cbegin(); it < birds.cendInf(); ++it ) {
        
        auto& bird = *it ;
        
        for ( StrainIdType strainId : epiModel.getStrainList( bird->infState ) )
            cts[strainId]++ ;
        
    }
    
}


//====== FarmRolloutTimeTracker ======//

FarmRolloutTimeTracker::FarmRolloutTimeTracker( NamedOfstream outFile ): BaseCrossSectionalTracker( outFile, 1, { FARM_DAILY_CHECK_HOUR } ), buffer( 1000 ) {
    
    if ( outFile.isClosed() )
        this->flush = std::bind( &FarmRolloutTimeTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &FarmRolloutTimeTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &FarmRolloutTimeTracker::checkTimePeriodic, this, std::placeholders::_1 );
        
}

void FarmRolloutTimeTracker::collectInfo( WorldSimulator* simulator ) {
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& farms = simulator->getFarmList();
    
    for ( auto& farm: farms ) {
        bool doesRolloutTrade = farm->doesRolloutTrade();
        if ( doesRolloutTrade ) {
            bool hasEmptiedBatch = farm->hasEndedRolloutRequest();
            if ( hasEmptiedBatch ) {
                if ( buffer.isFull() )
                    flush( clock );
                buffer.insert( { farm->getUnderlyingSetting()->getStringID(), clock.getCurrDay() - farm->getDayRolloutBegin() } );
            }
        }
    }
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
}

void FarmRolloutTimeTracker::reset( bool updateStream  ) {
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

void FarmRolloutTimeTracker::flushWithPrint( Clock& clock ) {
    while( !buffer.empty() ) {
        auto& elem = buffer.peekLast();
        outFile.getStream() << fmt::format("{},{},{}\n", clock.getCurrTime(),
                                elem.first,
                                elem.second );
        buffer.removeLast();
    }
}

void FarmRolloutTimeTracker::doNothing( Clock& clock ) {
    buffer.clear();
}


//====== unsoldMmanStockTracker ======//

UnsoldMmanStockTracker::UnsoldMmanStockTracker( NamedOfstream outFile, const uint& deltaTdays ): BaseCrossSectionalTracker( outFile, deltaTdays, { ( MARKET_OPENING_HOUR + 1 ) % HOURS_DAY } ), buffer( 1000 ) {
    
    if ( outFile.isClosed() )
        this->flush = std::bind( &UnsoldMmanStockTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &UnsoldMmanStockTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &UnsoldMmanStockTracker::checkTimePeriodic, this, std::placeholders::_1 );
        
}

void UnsoldMmanStockTracker::collectInfo( WorldSimulator* simulator ) {
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& places = simulator->getPlaces();
    
    for ( auto& place: places ) {
        NodeType settingType = place->getNodeType();
        if ( settingType == NodeType::middleman ) {
            if ( buffer.isFull() )
                flush( clock );
            buffer.insert( { place->getUnderlyingSetting()->getStringID(), place->getSizeBirds() } );
        }
    }
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
}

void UnsoldMmanStockTracker::reset( bool updateStream  ) {
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

void UnsoldMmanStockTracker::flushWithPrint( Clock& clock ) {
    while( !buffer.empty() ) {
        auto& elem = buffer.peekLast();
        outFile.getStream() << fmt::format("{},{},{}\n", clock.getCurrTime(),
                                elem.first,
                                elem.second );
        buffer.removeLast();
    }
}

void UnsoldMmanStockTracker::doNothing( Clock& clock ) {
    buffer.clear();
}


//====== unsoldVendorStockTracker ======//

UnsoldVendorStockTracker::UnsoldVendorStockTracker( NamedOfstream outFile, const uint& deltaTdays ): BaseCrossSectionalTracker( outFile, deltaTdays, { ( MARKET_CLOSING_HOUR + 1 ) % HOURS_DAY } ), buffer( 1000 ) {
    
    if ( outFile.isClosed() )
        this->flush = std::bind( &UnsoldVendorStockTracker::doNothing, this, std::placeholders::_1 );
    else
        this->flush = std::bind( &UnsoldVendorStockTracker::flushWithPrint, this, std::placeholders::_1 );
    
    checkTime = std::bind( &UnsoldVendorStockTracker::checkTimePeriodic, this, std::placeholders::_1 );
        
}

void UnsoldVendorStockTracker::collectInfo( WorldSimulator* simulator ) {
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& places = simulator->getPlaces();
    
    for ( auto& place: places ) {
        NodeType settingType = place->getNodeType();
        if ( settingType == NodeType::vendor ) {
            if ( buffer.isFull() )
                flush( clock );
            buffer.insert( { place->getUnderlyingSetting()->getStringID(), place->getSizeBirds() } );
        }
    }
    flush( clock );
    tLastOutputDay = clock.getCurrDay();
}

void UnsoldVendorStockTracker::reset( bool updateStream  ) {
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

void UnsoldVendorStockTracker::flushWithPrint( Clock& clock ) {
    while( !buffer.empty() ) {
        auto& elem = buffer.peekLast();
        outFile.getStream() << fmt::format("{},{},{}\n", clock.getCurrTime(),
                                elem.first,
                                elem.second );
        buffer.removeLast();
    }
}

void UnsoldVendorStockTracker::doNothing( Clock& clock ) {
    buffer.clear();
}

//====== DiscardedBirdsTracker ======//

DiscardedBirdsTracker::DiscardedBirdsTracker( NamedOfstream outFile, const uint& deltaTdays): BaseCrossSectionalTracker( outFile, deltaTdays, { FARM_DAILY_CHECK_HOUR } ) {
    
    checkTime = std::bind( &DiscardedBirdsTracker::checkTimePeriodic, this, std::placeholders::_1 );
}

void DiscardedBirdsTracker::collectInfo( WorldSimulator* simulator ) {
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    auto& farms = simulator->getFarmList();
    
    long birdsProduced = 0;
    long birdsDiscarded = 0;
    
    for ( auto& farm: farms ) {
        birdsProduced += farm->getNumberBirdsProduced();
        birdsDiscarded += farm->getNumberBirdsDiscarded();
    }
    
    tLastOutputDay = clock.getCurrDay();
    if ( !outFile.isClosed() ) {
        outFile.getStream() << fmt::format("{},{},{}\n", tLastOutputDay, birdsDiscarded, birdsProduced );
    }
}

void DiscardedBirdsTracker::reset( bool updateStream  ) {
    tLastOutputDay = 0;
    
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

//====== TimeSinceMarketingTracker ======//

TimeSinceMarketingTracker::TimeSinceMarketingTracker( NamedOfstream outFile, const uint& dtMax_, const uint& deltaTdays ): BaseCrossSectionalTracker( outFile, deltaTdays, {MARKET_OPENING_HOUR + 1} ), dtMax( dtMax_ ) {};

void TimeSinceMarketingTracker::collectInfo( WorldSimulator* simulator ) {
    Clock& clock = simulator->getClock();
    if ( !checkTime( clock ) ) // checks if it is ok to compute output
        return;
    
    uint tCurr = clock.getCurrTime();
    std::vector<uint> histo = std::vector<uint>( dtMax, 0 );
    
    if ( ( clock.getCurrHour() > MARKET_OPENING_HOUR ) and ( clock.getCurrHour() <= MARKET_CLOSING_HOUR ) ) {
        auto& markets = simulator->getMarketList();
        for ( auto& market: markets ) {
            auto& birds = market->getContext()->getBirds();
            auto it = birds.begin();
            auto end = birds.end();
            while( it < end ) {
                uint dt = tCurr - (*it)->t_market;
                ++histo[dt];
                ++it;
            }
        }
    }
    else {
        auto& vendors = simulator->getVendorList();
        for ( auto& vendor: vendors ) {
            auto& birds = vendor->getContext()->getBirds();
            auto it = birds.begin();
            auto end = birds.end();
            while( it < end ) {
                uint dt = tCurr - (*it)->t_market;
                ++histo[dt];
                ++it;
            }
        }
    }
    
    if ( !outFile.isClosed() ) {    // flush results
        outFile.getStream() << stringify_vec( histo ) << "\n";
    }
    
}

void TimeSinceMarketingTracker::reset( bool updateStream ) {
    tLastOutputDay = 0;
    if ( updateStream )
        outFile.update();
    else
        outFile.close();
}

//====== InitialVendorDemographyPrinter ======//

InitialVendorDemographyPrinter::InitialVendorDemographyPrinter( NamedOfstream outFile ): BaseCrossSectionalTracker( outFile, UINT_MAX, {0} ) {
    checkTime = std::bind( []() { return false; } );
}

void InitialVendorDemographyPrinter::collectInfo( WorldSimulator* simulator ) {
    
    if ( outFile.isClosed() )
        return; // does not work if stream is closed
    
    auto& outstream = outFile.getStream();
    auto& vendors = simulator->getVendorList();
    for ( auto& vendor: vendors ) {
        uint id = vendor->getContext()->getId();
        uint marketSrc = vendor->getMarketBuying()->getContext()->getId();
        uint marketDst = vendor->getMarketSelling()->getContext()->getId();
        uint layer = vendor->getVendorLayer();
        
        std::string type;
        switch ( vendor->getVendorType() ) {
            case ActorType::retailer:
                type = "R";
                break;
            case ActorType::wholesaler:
                type = "W";
                break;
            default:
                type = "NA";
                break;
        }

        outstream << fmt::format( "{},{},{},{},{}\n", id, type, marketSrc, marketDst, layer );
    }
    
    outFile.close(); // closes stream after first use
}

void InitialVendorDemographyPrinter::reset( bool updateStream ) {}; // does nothing


//====== InitialMiddlemanDemographyPrinter ======//

InitialMiddlemanDemographyPrinter::InitialMiddlemanDemographyPrinter( NamedOfstream outFile ): BaseCrossSectionalTracker( outFile, UINT_MAX, {0} ) {
    checkTime = std::bind( []() { return false; } );
}

void InitialMiddlemanDemographyPrinter::collectInfo( WorldSimulator* simulator ) {
    
    if ( outFile.isClosed() )
        return; // does not work if stream is closed
    
    auto& outstream = outFile.getStream();
    auto& mmen = simulator->getMiddlemanList();

    for ( auto& mman: mmen ) {
        
        uint id = mman->getContext()->getId();
        uint deg_markets = mman->getExpectedNumberMarketsToVisit(); // expected degree

        outstream << fmt::format( "{},{}\n", id, deg_markets );
    }
    
    outFile.close(); // closes stream after first use
}

void InitialMiddlemanDemographyPrinter::reset( bool updateStream ) {}; // does nothing


//====== WritePhylogenyHomochronousMarketSampling ======//

WritePhylogenyHomochronousMarketSampling::WritePhylogenyHomochronousMarketSampling( NamedOfstream outFile, uint tSample, Random* randGen, uint chickensSampledPerMarket, std::vector<uint> marketsSampledIdxs ): BaseCrossSectionalTracker( outFile, UINT_MAX, {} ), tSample( tSample), randGen( randGen ), chickensSampledPerMarket( chickensSampledPerMarket ), marketsSampledIdxs( marketsSampledIdxs ) {
    
    checkLarger( static_cast<int>( chickensSampledPerMarket ), 0 ) ;
    checkLarger( static_cast<int>( marketsSampledIdxs.size() ), 0 ) ;
    
    checkTime = std::bind( &BaseCrossSectionalTracker::checkTimeFixedTiming, this, std::placeholders::_1, tSample ) ;
    
}

void WritePhylogenyHomochronousMarketSampling::collectInfo( WorldSimulator* simulator ) {
    
    if ( not checkTime( simulator->getClock() ) )
        return ;
    
    uint t = simulator->getClock().getCurrTime() ; // current time
    
    std::shared_ptr<LineageTrackerManager> lngTrackerMngr = simulator->getLngTrackerMngr() ;
    assert( lngTrackerMngr != nullptr ) ;
    
    // sample chickens within selected markets
    
    uint nStrains = lngTrackerMngr->getStrainNumber() ;
    std::unordered_map<StrainIdType, std::vector<BirdID>> sampledLngsByStrain = {} ;
    for ( uint i = 0 ; i < nStrains ; ++i )
        sampledLngsByStrain[i] = {} ;
    
    std::unordered_map<BirdID, DataLineageSampling, hashBirdID> sampledLngsSamplingInfo = {};
        
    std::vector<NodeMarket*>& markets = simulator->getMarketList() ;
    
    for ( uint marketIdx : marketsSampledIdxs ) { // iterate over markets
        
        // sample chickens from this market
        NodeMarket* nm = markets[marketIdx] ;
        std::vector<uint> sampledBirdsIdxs = sampleInfectedChickensFromSetting( nm->getUnderlyingSetting(), *randGen, chickensSampledPerMarket ) ;
        
        auto& birds = nm->getUnderlyingSetting()->getBirds() ;
        for ( uint birdIdx : sampledBirdsIdxs ) {
            
            auto& bird = birds.getBirdSparse( birdIdx ) ;
            BirdID id = bird->getID() ;
            StateType state = bird->infState ;
            std::vector<StrainIdType> strains = lngTrackerMngr->getStrainList( state ) ;
            
            // classify lineages found by strain and chicken
            for ( StrainIdType& strain : strains )
                sampledLngsByStrain[strain].push_back( id ) ;
            
            sampledLngsSamplingInfo[id] = DataLineageSampling( t, fmt::format( "M{}", marketIdx ) ) ;
            
        }
        
    }
    
    // extract and write phylogeny for each strain
    
    for ( auto& tmp : sampledLngsByStrain ) {
        
        // get sampled lineages
        StrainIdType strain = tmp.first ;
        std::vector<BirdID> sampledLngs = tmp.second ;
        //std::unordered_set<BirdID, hashBirdID> sampledLngsSet = {};
        //for ( auto& birdID : sampledLngs )
        //    sampledLngsSet.insert( birdID ) ;
        
        std::unordered_map<BirdID, DataLineageSampling, hashBirdID> sampledLngsSamplingInfoTmp = {};
        for ( auto& birdID : sampledLngs )
            sampledLngsSamplingInfoTmp[birdID] = sampledLngsSamplingInfo[birdID] ;

        
        if ( sampledLngs.size() > 0 ) {
                        
            auto tree = lngTrackerMngr->getTree( strain ) ;
            
            // find reduced transmission tree(s)
            // eventually accounting for non-homologous clusters
            
            auto trimmedSubTrees = tree->subSampleTree( sampledLngsSamplingInfoTmp ) ;
            for ( auto trimmedSubTreeRoot : trimmedSubTrees ) {
                
                // convert to NHX

                PhyloNode<BirdID,DataLineage>* phyloTree = getAncestralTree( trimmedSubTreeRoot ) ;
                std::string nhx = getNHX( phyloTree ) ;
                                
                // write to file
                outFile.getStream() << fmt::format( "{},{}\n", strain, nhx ) ;
                
                // free memory associated to reduced transmission and phylogenetic trees
                deletePhyloNodeTree( phyloTree ) ;
                deleteLineageTreeNodeTree( trimmedSubTreeRoot ) ;
                                
            }
            
        }
        
    }
    
    
}

void WritePhylogenyHomochronousMarketSampling::reset( bool updateStream ) {
    
    tLastOutputDay = 0;
    if ( updateStream )
        outFile.update() ;
    else
        outFile.close() ;
    
};

//====== WritePhylogenyHeterochronousMarketSampling ======//


WritePhylogenyHeterochronousMarketSampling::WritePhylogenyHeterochronousMarketSampling( NamedOfstream outFile, Random* randGen, std::vector<uint> samplingTimes, std::vector<uint> marketsSampledIdxs, std::vector<uint> birdSampledAmounts ): BaseCrossSectionalTracker( outFile, UINT_MAX, {} ), randGen( randGen ), currentEventIdx( 0 ), samplingTimes( samplingTimes ), marketsSampledIdxs( marketsSampledIdxs ), birdSampledAmounts( birdSampledAmounts ) {
    
    // make sure the number of sampling events makes sense
    checkEquality( samplingTimes.size(), marketsSampledIdxs.size() ) ;
    checkEquality( samplingTimes.size(), birdSampledAmounts.size() ) ;

    nEvents = static_cast<uint>( samplingTimes.size() ) ;
    checkLarger( nEvents, static_cast<uint>( 1 ) ) ;
    
    checkTime = std::bind( &BaseCrossSectionalTracker::checkTimeFixedTiming, this, std::placeholders::_1, samplingTimes[currentEventIdx] ) ;
    
}


void WritePhylogenyHeterochronousMarketSampling::collectInfo( WorldSimulator* simulator ) {
    
    // may perform multiple sampling events at the same time, hence the while loop
    while ( checkTime( simulator->getClock() ) ) {
    
        //== boilerplate code
        std::shared_ptr<LineageTrackerManager> lngTrackerMngr = simulator->getLngTrackerMngr() ;
        assert( lngTrackerMngr != nullptr ) ;
        
        uint nStrains = lngTrackerMngr->getStrainNumber() ;
        std::unordered_map<StrainIdType, std::vector<BirdID>> sampledLngsByStrain = {} ;
        for ( uint i = 0 ; i < nStrains ; ++i )
            sampledLngsByStrain[i] = {} ;
        
        std::unordered_map<BirdID, DataLineageSampling, hashBirdID> sampledLngsSamplingInfo = {};
        
        //== mark chickens as sampled
        uint t = simulator->getClock().getCurrTime() ; // current time
        uint marketIdx = marketsSampledIdxs[currentEventIdx] ; // current market idx
        uint chickensSampled = birdSampledAmounts[currentEventIdx] ; // current birds sampled amount
        NodeMarket* nm = simulator->getMarketList()[marketIdx] ;
        std::vector<uint> sampledBirdsIdxs = sampleInfectedChickensFromSetting( nm->getUnderlyingSetting(), *randGen, chickensSampled ) ; // number of sampled chickens
        
        auto& birds = nm->getUnderlyingSetting()->getBirds() ;
        for ( uint birdIdx : sampledBirdsIdxs ) { // loop over sampled chickens
            
            auto& bird = birds.getBirdSparse( birdIdx ) ;
            BirdID id = bird->getID() ;
            StateType state = bird->infState ;
            std::vector<StrainIdType> strains = lngTrackerMngr->getStrainList( state ) ;
            
            for ( StrainIdType s : strains ) {
                
                auto lngTree = lngTrackerMngr->getTree( s ) ;
                lngTree->sampleExtantLineage( id, t, fmt::format( "M{}", marketIdx ) ) ;
                
            }
           
        }
         
        //== update sampling system
        currentEventIdx++ ;
        if ( currentEventIdx < nEvents ) {
            //== set time
            checkTime = std::bind( &BaseCrossSectionalTracker::checkTimeFixedTiming, this, std::placeholders::_1, samplingTimes[currentEventIdx] ) ;
            
        }
        else { // no more sampling events, print phylogeny to file
            
            // extract and write phylogeny for each strain
            for ( uint iStrain = 0 ; iStrain < nStrains ; ++iStrain ) {
                
                StrainIdType strain = static_cast<StrainIdType>( iStrain ) ;
                auto tree = lngTrackerMngr->getTree( strain ) ; // get full tree
                
                // get trimmed subtree(s)
                // N.B. automatically gets previously sampled lineages
                auto trimmedSubTrees = tree->subSampleTree() ;
                
                // convert each subtree (if many different clusters) to NHX format
                for ( auto trimmedSubTreeRoot : trimmedSubTrees ) {
                    
                    // convert to NHX
                    PhyloNode<BirdID,DataLineage>* phyloTree = getAncestralTree( trimmedSubTreeRoot ) ;
                    std::string nhx = getNHX( phyloTree ) ;
                                    
                    // write to file
                    outFile.getStream() << fmt::format( "{},{}\n", strain, nhx ) ;
                    
                    // free memory associated to reduced transmission and phylogenetic trees
                    deletePhyloNodeTree( phyloTree ) ;
                    deleteLineageTreeNodeTree( trimmedSubTreeRoot ) ;
                                    
                }
                
            }
            
            // set sampling time to max
            checkTime = std::bind( &BaseCrossSectionalTracker::checkTimeFixedTiming, this, std::placeholders::_1, UINT32_MAX ) ;

        }
        
    }
    
}



void WritePhylogenyHeterochronousMarketSampling::reset( bool updateStream ) {
    
    currentEventIdx = 0 ;
    tLastOutputDay = 0 ;
    
    checkTime = std::bind( &BaseCrossSectionalTracker::checkTimeFixedTiming, this, std::placeholders::_1, samplingTimes[currentEventIdx] ) ; // reset time checker to first sampling event
    
    if ( updateStream )
        outFile.update() ;
    else
        outFile.close() ;
    
};



