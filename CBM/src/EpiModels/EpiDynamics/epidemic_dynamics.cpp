//
//  epidemic_dynamics.cpp
//  CBM
//
//

#include <stdio.h>

#include "epidemic_dynamics.hpp"

// Check for disease transmission within a single setting:
// each infectious birds contacts a single, randomly chosen bird, which is then checked for transmission
void runBirdTransmissionSingleSetting( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen ) {
    
    uint tCurr = clock.getCurrTime();
    uint nbirds = setting.getSizeBirds();
    double hz;
    
    if ( nbirds < 2 )
        return;
    
    for (pBirdVecType::iterator birdIt = setting.begin(), birdItEnd = setting.endInf();
         birdIt < birdItEnd; ++birdIt ) {

        // select infector
        pBird& infector = *birdIt;
        
        // first select infectee and then apply hazard rates
        uint idxInfectee;
        do {
            idxInfectee = randGen.getUniInt( nbirds - 1 ); // choose a random infectee
        }
        while ( setting.getBirdDense( idxInfectee )->id == infector->id );
        
        pBird& infectee = setting.getBirdDense( idxInfectee );
        
        for ( auto& strain: compModel.getStrainList( infector->infState ) ) {
            hz = compModel.getHazard( *infector, *infectee, strain, tCurr, setting.getBaseWeight() );
            infectee->hazard -= hz;
            if ( infectee->hazard <= 0. ) { // handle successful exposure
                compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
                setting.notifySuccessfulExposure( *infectee );
            }
        }
    }
}

// Check for disease transmission within a single setting:
// each infectious birds contacts a single, randomly chosen bird, which is then checked for transmission
// also tracks individual transmission events
void runBirdTransmissionSingleSettingWithTracking( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker ) {
        
    uint tCurr = clock.getCurrTime();
    uint nbirds = setting.getSizeBirds();
    double hz;
    
    if ( nbirds < 2 )
        return;
    
    for (pBirdVecType::iterator birdIt = setting.begin(), birdItEnd = setting.endInf();
         birdIt < birdItEnd; ++birdIt ) {

        // select infector
        pBird& infector = *birdIt;
        
        // first select infectee and then apply hazard rates
        uint idxInfectee;
        do {
            idxInfectee = randGen.getUniInt( nbirds - 1 ); // choose a random infectee
        }
        while ( setting.getBirdDense( idxInfectee )->id == infector->id );
        
        pBird& infectee = setting.getBirdDense( idxInfectee );
        
        for ( auto& strain: compModel.getStrainList( infector->infState ) ) {
            hz = compModel.getHazard( *infector, *infectee, strain, tCurr, setting.getBaseWeight() );
            
            //if ( setting.getSettingType() == NodeType::market )
            //    std::cout << hz << " " << infectee->hazard << std::endl;
            
            infectee->hazard -= hz;
            
            if ( infectee->hazard <= 0. ) { // handle successful exposure
                compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
                setting.notifySuccessfulExposure( *infectee );
                tracker->addInfectionEvent( *infector, *infectee, strain, clock );
            }
        }
    }
}


// Checks for environmental transmission within a single setting
void runEnvTransmissionSingleSetting( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen ) {
    
    uint tCurr = clock.getCurrTime();
    double hz = 0.;
    double envTr = compModel.getEnvTransmissibility();
    BaseContEnv& contEnv = setting.getContEnv();
    std::vector<StrainIdType>& strainsEnvironment = setting.getContEnv().getCurrentStrains();
    randGen.shuffleVector( strainsEnvironment );
    
    for ( auto& strain: strainsEnvironment ) {
        double hz0 = envTr * contEnv.getTotalUnits( strain );
        for ( pBirdVecType::iterator birdIt = setting.begin(), birdItEnd = setting.end();
            birdIt < birdItEnd; ++birdIt ) {
            
            pBird& infectee = *birdIt;
            //hz = compModel.hazardify_prob( hz0 * compModel.getSusceptibility( *infectee, strain, tCurr ) );
            hz = hz0 * compModel.getSusceptibility( *infectee, strain, tCurr ); // ?? weight??
            
            infectee->hazard -= hz;
            if ( infectee->hazard <= 0. ) {
                compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
            }
            
        }
    }
}

// Checks for environmental transmission within a single setting
// also tracks individual transmission events
void runEnvTransmissionSingleSettingWithTracking( HoldsBirds& setting, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker ) {
    
    uint tCurr = clock.getCurrTime();
    double hz = 0.;
    double envTr = compModel.getEnvTransmissibility();
    BaseContEnv& contEnv = setting.getContEnv();
    std::vector<StrainIdType>& strainsEnvironment = setting.getContEnv().getCurrentStrains();
    randGen.shuffleVector(strainsEnvironment);
    
    for ( auto& strain: strainsEnvironment ) {
        double hz0 = envTr * contEnv.getTotalUnits( strain );
        for ( pBirdVecType::iterator birdIt = setting.begin(), birdItEnd = setting.end();
            birdIt < birdItEnd; ++birdIt ) {
            
            pBird& infectee = *birdIt;
            hz = hz0 * compModel.getSusceptibility( *infectee, strain, tCurr ); // ?? weight??
            //hz = compModel.hazardify_prob( hz0 * compModel.getSusceptibility( *infectee, strain, tCurr ) );
            
            infectee->hazard -= hz;
            if ( infectee->hazard <= 0. ) {
                compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
                tracker->addEnvironmentInfectionEvent( contEnv.getRandomContaminatedUnitID( strain, randGen ), *infectee, strain, clock );
            }
        }
    }
}

// randomly selects 'nContacts' birds from 'setting1' and tries to infect as many birds in 'setting2'
void runTransmissionTwoSettings( HoldsBirds& setting1, HoldsBirds& setting2, const uint& nContacts, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen ) {
    
    if ( &setting1 == &setting2 )
        return;  // prevent transmission from a setting to itself with this routine
    
    uint nInfBirds1 = setting1.getSizeBirdsInf();
    uint nbirds2 = setting2.getSizeBirds();
    
    if ( nInfBirds1 == 0 )
        return; // no infected birds in setting 1
    
    if ( nbirds2 == 0 )
        return; // no birds in setting 2

    uint tCurr = clock.getCurrTime();
    double hz = 0.;
    
    for ( uint k = 0; k < nContacts; ++k ) {
        // select infector and infectee
        uint idxInfector = randGen.getUniInt( nInfBirds1 - 1 );
        uint idxInfectee = randGen.getUniInt( nbirds2 - 1 );
        
        pBird& infector = setting1.getBirdDense( idxInfector );
        pBird& infectee = setting2.getBirdDense( idxInfectee );
        
        // loop over all strains carried by infector
        for ( auto& strain: compModel.getStrainList( infector->infState ) ) {
            // evaluate hazard reduction and update infected hazard
            hz = compModel.getHazard( *infector, *infectee, strain, tCurr, setting1.getBaseWeight() );
            infectee->hazard -= hz;
            if ( infectee->hazard <= 0. ) { // handle successful exposure
                compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
                setting2.notifySuccessfulExposure( *infectee );
            }
        }
    }
}


// randomly selects 'nContacts' birds from 'setting1' and tries to infect as many birds in 'setting2'
void runTransmissionTwoSettingsWithTracking( HoldsBirds& setting1, HoldsBirds& setting2, const uint& nContacts, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker ) {
    
    if ( &setting1 == &setting2 )
        return;  // prevent transmission from a setting to itself with this routine
    
    uint nInfBirds1 = setting1.getSizeBirdsInf();
    uint nbirds2 = setting2.getSizeBirds();
    
    if ( nInfBirds1 == 0 )
        return; // no infected birds in setting 1
    
    if ( nbirds2 == 0 )
        return; // no birds in setting 2

    uint tCurr = clock.getCurrTime();
    double hz = 0.;
    
    for ( uint k = 0; k < nContacts; ++k ) {
        // select infector and infectee
        uint idxInfector = randGen.getUniInt( nInfBirds1 - 1 );
        uint idxInfectee = randGen.getUniInt( nbirds2 - 1 );
        
        pBird& infector = setting1.getBirdDense( idxInfector );
        pBird& infectee = setting2.getBirdDense( idxInfectee );
        
        // loop over all strains carried by infector
        for ( auto& strain: compModel.getStrainList( infector->infState ) ) {
            // evaluate hazard reduction and update infected hazard
            hz = compModel.getHazard( *infector, *infectee, strain, tCurr, setting1.getBaseWeight() );
            infectee->hazard -= hz;
            if ( infectee->hazard <= 0. ) { // handle successful exposure
                compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
                setting2.notifySuccessfulExposure( *infectee );
                tracker->addInfectionEvent( *infector, *infectee, strain, clock );
            }
        }
    }
}


// infect a random bird at destination setting with a random strain from a random infectious birds from source setting
void sparkInfectionTwoSettings( HoldsBirds& setting1, HoldsBirds& setting2, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen ) {
    
    if ( &setting1 == &setting2 )
        return;  // prevent transmission from a setting to itself with this routine
    
    uint nInfBirds1 = setting1.getSizeBirdsInf();
    if ( nInfBirds1 == 0 )
        return; // no infected birds in setting 1
    
    uint nBirds2 = setting2.getSizeBirds();
    if ( nBirds2 == 0 )
        return; // no birds in setting 2

    uint tCurr = clock.getCurrTime();

    uint nAttempts = 0;
    while( nAttempts < 50 ) { // repeat until compatible pair is found
        
        uint idxInfector = randGen.getUniInt( nInfBirds1 - 1 );
        uint idxInfectee = randGen.getUniInt( nBirds2 - 1 );
    
        pBird& infector = setting1.getBirdDense( idxInfector );
        pBird& infectee = setting2.getBirdDense( idxInfectee );
        
        // select random strain from infector
        const std::vector<StrainIdType>& strains = compModel.getStrainList( infector->infState );
        StrainIdType strain = randGen.getRandomElemFromVec( strains );
    
        // check if infectee is not immune
        if ( compModel.isImmuneToStrain( *infectee, strain ) ) {
            nAttempts++;
        }
        else {
            // compatible pair found -> apply exposure
            compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
            setting2.notifySuccessfulExposure( *infectee );
            //compModel.applyDirectInfection( infectee, strain, tCurr, randGen );
            return;
        }
    }
}

// infect a random bird at destination setting with a random strain from a random infectious birds from source setting
// also tracks infections
void sparkInfectionTwoSettingsWithTracking( HoldsBirds& setting1, HoldsBirds& setting2, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker ) {
    
    if ( &setting1 == &setting2 )
        return;  // prevent transmission from a setting to itself with this routine
    
    uint nInfBirds1 = setting1.getSizeBirdsInf();
    if ( nInfBirds1 == 0 )
        return; // no infected birds in setting 1
    
    uint nBirds2 = setting2.getSizeBirds();
    if ( nBirds2 == 0 )
        return; // no birds in setting 2

    uint tCurr = clock.getCurrTime();

    uint nAttempts = 0;
    while( nAttempts < 50 ) { // repeat until compatible pair is found
        uint idxInfector = randGen.getUniInt( nInfBirds1 - 1 );
        uint idxInfectee = randGen.getUniInt( nBirds2 - 1 );
    
        pBird& infector = setting1.getBirdDense( idxInfector );
        pBird& infectee = setting2.getBirdDense( idxInfectee );
        
        // select random strain from infector
        const std::vector<StrainIdType>& strains = compModel.getStrainList( infector->infState );
        StrainIdType strain = randGen.getRandomElemFromVec( strains );
    
        // check if infectee is not immune
        if ( compModel.isImmuneToStrain( *infectee, strain ) ) {
            nAttempts++;
        }
        else {
            // compatible pair found -> apply exposure
            compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
            //compModel.applyDirectInfection( infectee, strain, tCurr, randGen );
            setting2.notifySuccessfulExposure( *infectee );
            tracker->addInfectionEvent( *infector, *infectee, strain, clock );
            return;
        }
    }
}


// Selects a single bird in 'setting', if any, and attempts an artificial infection with 'strain',
// with infection rate 'sparkRate' and accounting for target's bird susceptibility to 'strain'
void sparkInfectionSingleSetting( HoldsBirds& setting, const StrainIdType& strain, const double& sparkRate, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen ) {
    
    if ( !compModel.isStrainValid( strain ) )
        return; // strain ID not valid
    
    uint nBirds = setting.getSizeBirds();
    if ( nBirds == 0 )
        return; // no birds

    uint tCurr = clock.getCurrTime();

    uint idxInfectee = randGen.getUniInt( nBirds - 1 );
    pBird& infectee = setting.getBirdDense( idxInfectee );
    
    double hz = compModel.getSusceptibility( *infectee, strain, tCurr ) * sparkRate;
    infectee->hazard -= hz;
    
    if ( infectee->hazard <= 0. ) { // handle successful exposure
        compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
        setting.notifySuccessfulExposure( *infectee );
    }
    
}

// Same as above, but also tracks successful infections
void sparkInfectionSingleSettingWithTracking( HoldsBirds& setting, const StrainIdType& strain, const double& sparkRate, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker ) {
    
    if ( !compModel.isStrainValid( strain ) )
        return; // strain ID not valid
    
    uint nBirds = setting.getSizeBirds();
    if ( nBirds == 0 )
        return; // no birds

    uint tCurr = clock.getCurrTime();

    uint idxInfectee = randGen.getUniInt( nBirds - 1 );
    pBird& infectee = setting.getBirdDense( idxInfectee );
    
    double hz = compModel.getSusceptibility( *infectee, strain, tCurr ) * sparkRate;
    infectee->hazard -= hz; //
    
    if ( infectee->hazard <= 0. ) { // handle successful exposure
        compModel.applySuccessfulExposure( infectee, strain, tCurr, randGen );
        setting.notifySuccessfulExposure( *infectee );
        tracker->addExternalInfectionEvent( *infectee, strain, clock );
    }
}

// Randomly selects up to 'nInfections' birds in 'setting', and automatically infects them with 'strain';
// bypasses any innate immunity but not acquired immunity.
void forceSparkInfectionSingleSetting( HoldsBirds& setting, const StrainIdType& strain, const uint& nInfections, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen ) {
    
    if ( !compModel.isStrainValid( strain ) )
        return; // strain ID not valid
    
    uint nBirdsAvail = setting.getSizeBirds();
    if ( nBirdsAvail == 0 )
        return; // no birds
    
    uint nInfectionsToPerform = std::min( nInfections, nBirdsAvail );
    uint nAttempts = 0;
    
    while ( ( nInfectionsToPerform > 0 ) or ( nAttempts < 50 ) ) {
        uint i = randGen.getUniInt( nBirdsAvail - 1 );
        pBird& bird = setting.getBirdDense( i );
        if ( !compModel.isImmuneToStrain( *bird, strain ) ) {
            compModel.applyDirectInfection( bird, strain, clock.getCurrTime(), randGen );
            setting.notifySuccessfulExposure( *bird );
            --nInfectionsToPerform;
            nAttempts = 0;
        }
        else
            ++nAttempts;
    }
}

// Same as above, but also tracks successful infections
void forceSparkInfectionSingleSettingWithTracking( HoldsBirds& setting, const StrainIdType& strain, const uint& nInfections, const Clock& clock, BaseCompartmentalModel& compModel, Random& randGen, BaseInfectionTracker* tracker ) {
    
    if ( !compModel.isStrainValid( strain ) )
        return; // strain ID not valid
    
    uint nBirdsAvail = setting.getSizeBirds();
    if ( nBirdsAvail == 0 )
        return; // no birds
    
    uint nInfectionsToPerform = std::min( nInfections, nBirdsAvail );
    uint nAttempts = 0;
    
    while ( ( nInfectionsToPerform > 0 ) or ( nAttempts < 50 ) ) {
        uint i = randGen.getUniInt( nBirdsAvail - 1 );
        pBird& bird = setting.getBirdDense( i );
        if ( !compModel.isImmuneToStrain( *bird, strain ) ) {
            compModel.applyDirectInfection( bird, strain, clock.getCurrTime(), randGen );
            setting.notifySuccessfulExposure( *bird );
            tracker->addExternalInfectionEvent( *bird, strain, clock );
            --nInfectionsToPerform;
            nAttempts = 0;
        }
        else
            ++nAttempts;
    }
}


// update environment contamination status within a single setting
void runContaminationSetting( HoldsBirds& setting, BaseCompartmentalModel& compModel, const Clock& clock, Random& randGen ) {
    
    BaseContEnv& contEnv = setting.getContEnv();
    
    //=== i) check decay of contaminated material
    
    double pDecay = compModel.getBirdSheddingProb();
    if ( pDecay > 0.)
        contEnv.removeUnits(randGen, pDecay );
    
    //=== ii) check pathogen shedding from birds

    // birds shed contaminated material in environment
    double pShed = compModel.getBirdSheddingProb();
    if ( pShed == 0. )
        return;
    
    pBirdVecType::iterator birdIt = setting.begin();
    pBirdVecType::iterator birdItEnd = setting.endInf();
    
    while ( birdIt < birdItEnd ) {
        std::advance( birdIt, randGen.getGeom( pShed ) );

        pBird& bird = *birdIt;
        for (auto& strain: compModel.getStrainList( bird->infState ) ) {
            contEnv.addUnits( bird->id, strain, 1);
        }
        
        std::advance( birdIt, 1 ); // if geom takes values 0,1,2,... then use this as well!
    }
}

void applyDirectInfection(pBird& bird, const StrainIdType& strain, const uint& tCurr, EventPriorityQueue<BirdStatusEvent>& recovEventQueue, const BaseCompartmentalModel& compModel, Random& randGen) {
    
    bool wasInfectious = bird->isInfectious();
    
    addImmunity(*bird, strain);
    addInfection(*bird, strain);
    
    double recProb = compModel.getRecoveryProb( strain );
    if ( recProb > 0. ) {
        uint timeToEvent = randGen.getGeom( recProb );
        recovEventQueue.pushEvent( BirdStatusEvent( bird, strain, tCurr + timeToEvent ) );
    }
    
    // this function must stay here
    if (!wasInfectious)
        bird->owner->swapToInfected(bird->index);      // tell holdsbirds new infectious bird
    
    // ?? track the new infection?
    
}
