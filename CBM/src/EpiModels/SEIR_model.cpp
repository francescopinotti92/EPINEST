//
//  SEIR_model.cpp
//  CBM
//
//

#include "SEIR_model.hpp"


SEIR::SEIR( uint nStrains ): BaseCompartmentalModel( nStrains, nStrains ) {
    assert( nStrains == 1 );
    settingEffectFunctions = {};
    transitionFunctions = {};
}

void SEIR::reset() {
    incubEventQ.resetQueue();
    recEventQ.resetQueue();
}

void SEIR::setTransmissibility( const std::vector<double>& trVec, const uint& dt ) {
    assert( trVec.size() == 1 );
    transmissibility[0] = trVec[0] * dt;
    beta = trVec[0] * dt; // store transmissibility aside for faster access
}

void SEIR::setSimpleInnateImmunity() {
    
    getBirdInnateSusceptibility = std::bind( &SEIR::get1, this, std::placeholders::_1, std::placeholders::_2 );
    
}

void SEIR::setCustomInnateImmunity() {
    
    getBirdInnateSusceptibility = std::bind( &SEIR::getSusceptibilityFromMap, this, std::placeholders::_1, std::placeholders::_2 );
    
}

void SEIR::setConstantInnateImmunity( const BirdType& bt, const double& C0 ) {
    birdInnateSusceptibility[bt] = std::bind( &SEIR::getConstantSusc, this, std::placeholders::_1, std::placeholders::_2, C0 );
}

void SEIR::setWaningInnateImmunity( const BirdType& bt, const double& C0, const double& r ) {
    birdInnateSusceptibility[bt] = std::bind( &SEIR::getWaningImmSusc, this, std::placeholders::_1, std::placeholders::_2, C0, r );
}

const double SEIR::getSusceptibility( const Bird& bird, const StrainIdType& strainId, const uint& t ) {
        
    if ( isImmuneToStrain( bird, 0 ) )
        return 0.;
    else
        return getBirdInnateSusceptibility( bird, t );
    
}

const double SEIR::getHazard( const Bird& infector, const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight ) {
    double susc = getSusceptibility( infectee, strain, t );
    return beta * susc * weight;
    
    /*
    if ( susc == 0. ) {
        return 0.;
    }
    else {
        //return hazardify_prob( beta * susc * weight );
        return beta * susc * weight;
    }*/
}

// overload of previous method, takes only the infectee as parameter and not the infector
const double SEIR::getHazard( const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight ) {
    double susc = getSusceptibility( infectee, strain, t );
    return beta * susc * weight;
    /*
    if ( susc == 0. ) {
        return 0.;
    }
    else {
        return hazardify_prob( beta * susc * weight );
    }*/
}

double SEIR::getConstantSusc(const Bird& bird, const uint& t, const double& C0 ) {
    return C0;
}

double SEIR::getWaningImmSusc(const Bird& bird, const uint& t, const double& C0, const double& r ) {
    return C0 * ( 1. - exp( - r * ( t - bird.t_birth ) ) );
}


void SEIR::addSingleSettingEffect( std::function< void ( HoldsBirds&, const Clock&, Random& ) > f ) {
    
    settingEffectFunctions.push_back( f ) ;
    
}

void SEIR::addStateTransition( std::function< void ( const Clock&, Random& ) > f ) {
    
    transitionFunctions.push_back( f ) ;
    
}

// checks if 'bird' is immune to 'strain'
// in this model immunity occurs after successful exposure
bool SEIR::isImmuneToStrain( const Bird& bird, const StrainIdType& strain ) {
    
    return ( bird.immState > 0 ) ? true : false ;
    
}

void SEIR::pushIncubationEvent( pBird& bird, const StrainIdType& strain, const uint& tEvent) {
    incubEventQ.pushEvent( BirdStatusEvent( bird, strain, tEvent ) );
}

void SEIR::applyDirectInfection( pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen ) {
    
    bool wasInfectious = bird->isInfectious();
    
    addImmunity(*bird, strain);
    addInfection(*bird, strain);
    
    double recProb = this->getRecoveryProb( strain );
    if ( recProb > 0. ) {
        
        uint timeToEvent = randGen.getGeom( recProb ) ;
        recEventQ.pushEvent( BirdStatusEvent( bird, strain, tCurr + timeToEvent ) ) ;
        
    }
    
    // this function must stay here
    if ( !wasInfectious )
        bird->owner->swapToInfected( bird->index ) ;      // tell holdsbirds about new infectious bird
}

// Applies events involving spontaneous state transition
// in this model, these include transitions from exposed (E) to infectious (I)
// and from I to recovered (R)
void SEIR::applyTransitionEvents( const Clock& clock, Random& randGen ) {
    
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
