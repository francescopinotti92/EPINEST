//
//  examplemodel.cpp
//  CBM
//
//

#include "examplemodel.hpp"


ExampleModel::ExampleModel( uint nStrains, uint maxStrains ): BaseCompartmentalModel( nStrains, maxStrains ) {
    
    // vector with pre-computed hazards
    
    hazards = std::vector<double>( nStrains, 0. );
                                
    // Compute susceptibility towards any strain, for each possible immune status.
    
    birdAcquiredSusceptibility = {} ;
    
    for ( StateType stateID : viableStates ) {
        
        birdAcquiredSusceptibility[stateID] = std::vector<double>( nStrains, 1. ) ;
        for ( auto strain : getStrainList( stateID ) )
            birdAcquiredSusceptibility[stateID][strain] = 0. ;
        
        
    }
    
    settingEffectFunctions = {};
    transitionFunctions = {};
}

void ExampleModel::reset() {
    incubEventQ.resetQueue();
    recEventQ.resetQueue();
}

void ExampleModel::setTransmissibility( const std::vector<double>& trVec, const uint& dt ) {
    assert( trVec.size() == nStrains );
    for (uint i = 0; i < nStrains; i++) {
        transmissibility[i] = trVec[i] * dt;
        hazards[i] = transmissibility[i];
        /*
        if ( transmissibility[i] > 0. ) {
            hazards[i] = hazardify_prob( transmissibility[i] );
        }*/
    }
    
}

void ExampleModel::setConstantInnateImmunity( const BirdType& bt, const double& C0 ) {
    birdInnateSusceptibility[bt] = std::bind( &ExampleModel::getConstantSusc, this, std::placeholders::_1, std::placeholders::_2, C0 );
}

void ExampleModel::setWaningInnateImmunity( const BirdType& bt, const double& C0, const double& r ) {
    birdInnateSusceptibility[bt] = std::bind( &ExampleModel::getWaningImmSusc, this, std::placeholders::_1, std::placeholders::_2, C0, r );
}

void setWaningInnateImmunity( const BirdType& bt, const double& C0, const double& r );

const double ExampleModel::getSusceptibility( const Bird& bird, const StrainIdType& strainId, const uint& t ) {
    
    if ( getStrainList( bird.immState ).size() >= maxStrains )
        return 0. ; // prevent exposure to more than maxStrains
    
    double sig1 = birdAcquiredSusceptibility[bird.immState][strainId];  // acquired immunity (only due to previously seen strains)
    double sig2 = birdInnateSusceptibility[bird.birdType]( bird, t );   // innate immunity term (includes maternal antibodies)
    return sig1 * sig2;
}

const double ExampleModel::getHazard( const Bird& infector, const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight ) {
    double susc = getSusceptibility( infectee, strain, t );
    return  getTransmissibility( strain ) * susc * weight;
    /*
    if ( susc  == 0. ) {
        return 0.;
    }
    else {
        return hazardify_prob( getTransmissibility( strain ) * susc * weight );
        //return hazards[strain];
    }*/
}

double ExampleModel::getConstantSusc(const Bird& bird, const uint& t, const double& C0 ) {
    return C0;
}

double ExampleModel::getWaningImmSusc(const Bird& bird, const uint& t, const double& C0, const double& r ) {
    return C0 * ( 1. - exp( - r * ( t - bird.t_birth ) ) );
}


void ExampleModel::addSingleSettingEffect( std::function< void ( HoldsBirds&, const Clock&, Random& ) > f ) {
    
    settingEffectFunctions.push_back( f ) ;

}

void ExampleModel::addStateTransition( std::function< void ( const Clock&, Random& ) > f ) {
    
    transitionFunctions.push_back( f ) ;
    
}

// checks if 'bird' is immune to 'strain'
// does not check for bird innate immunity
bool ExampleModel::isImmuneToStrain( const Bird& bird, const StrainIdType& strain ) {
    
    if ( birdAcquiredSusceptibility[ bird.immState ][ strain ] == 0. )
        return true ;
    else
        return false ;
    
}

void ExampleModel::pushIncubationEvent( pBird& bird, const StrainIdType& strain, const uint& tEvent) {
    incubEventQ.pushEvent( BirdStatusEvent( bird, strain, tEvent ) );
}

void ExampleModel::applyDirectInfection(pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen) {
    
    bool wasInfectious = bird->isInfectious();
    
    addImmunity(*bird, strain);
    addInfection(*bird, strain);
    
    double recProb = this->getRecoveryProb( strain );
    if ( recProb > 0. ) {
        uint timeToEvent = randGen.getGeom( recProb );
        recEventQ.pushEvent( BirdStatusEvent( bird, strain, tCurr + timeToEvent ) );
    }
    
    // this function must stay here
    if (!wasInfectious)
        bird->owner->swapToInfected( bird->index );      // tell holdsbirds about new infectious bird
}

void ExampleModel::applyTransitionEvents( const Clock& clock, Random& randGen ) {
    
    uint tCurr = clock.getCurrTime();

    // i) apply incubation events
    
    while ( true ) {
        
        if ( incubEventQ.empty() )   // no more events
            break;
        
        const BirdStatusEvent& eventCurr = incubEventQ.top();
    
        if ( eventCurr.t <= tCurr ) {
            if ( applyIncubationEvent( eventCurr ) ) {
                double recProb = getRecoveryProb( eventCurr.strain );
                if ( recProb > 0. ) {
                    uint timeToEvent = randGen.getGeom( recProb );
                            
                    recEventQ.pushEvent( BirdStatusEvent( eventCurr.targetBird, eventCurr.strain, tCurr + timeToEvent ) );
                }
            }
            incubEventQ.pop();
        }
        else
            break;                 // no more events for this time step
    }
    
    // ii) apply recovery events
    
    while ( true ) {
        
        if ( recEventQ.empty() )
            break;
        
        const BirdStatusEvent& currEvent = recEventQ.top();
        
        if ( currEvent.t <= tCurr ) {
            applyRecoveryEvent( currEvent );
            recEventQ.pop();
        }
        else
            break;
    }
    
}
