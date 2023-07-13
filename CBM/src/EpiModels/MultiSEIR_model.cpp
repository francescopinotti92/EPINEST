//
//  MultiSEIR_model.cpp
//  CBM
//
//

#include "MultiSEIR_model.hpp"


MultiSEIR::MultiSEIR( uint nStrains, uint maxStrains ): BaseCompartmentalModel( nStrains, maxStrains ) {

    settingEffectFunctions = {} ;
    transitionFunctions = {} ;
    
    // set up non-interacting strains
    getCrossReactiveSusceptibility = std::bind( &MultiSEIR::getCrossReactiveSusceptibilitySimple, this, std::placeholders::_1, std::placeholders::_2, 1. ) ;

}

void MultiSEIR::reset() {
    
    incubEventQ.resetQueue() ;
    recEventQ.resetQueue() ;
    
}

void MultiSEIR::setTransmissibility( const std::vector<double>& trVec, const uint& dt ) {
    
    assert( trVec.size() == nStrains ) ;
    
    for ( uint k = 0; k < nStrains; ++k )
        transmissibility[k] = trVec[k] * dt ;

}

void MultiSEIR::setSimpleCrossReactiveSusceptibility( const double& sigma ) {
    
    getCrossReactiveSusceptibility = std::bind( &MultiSEIR::getCrossReactiveSusceptibilitySimple, this, std::placeholders::_1, std::placeholders::_2, sigma ) ;
    
}

void MultiSEIR::setMultiplicativeCrossReactiveSusceptibility( const double& sigma ) {
    
    getCrossReactiveSusceptibility = std::bind( &MultiSEIR::getCrossReactiveSusceptibilityMultiplicative, this, std::placeholders::_1, std::placeholders::_2, sigma ) ;
    
}


void MultiSEIR::setSimpleInnateImmunity() {
    
    getBirdInnateSusceptibility = std::bind( &MultiSEIR::get1, this, std::placeholders::_1, std::placeholders::_2 ) ;
    
}

void MultiSEIR::setCustomInnateImmunity() {
    
    getBirdInnateSusceptibility = std::bind( &MultiSEIR::getSusceptibilityFromMap, this, std::placeholders::_1, std::placeholders::_2 ) ;
    
}

void MultiSEIR::setConstantInnateImmunity( const BirdType& bt, const double& C0 ) {
    
    birdInnateSusceptibility[bt] = std::bind( &MultiSEIR::getConstantSusc, this, std::placeholders::_1, std::placeholders::_2, C0 ) ;
    
}

void MultiSEIR::setWaningInnateImmunity( const BirdType& bt, const double& C0, const double& r ) {
    
    birdInnateSusceptibility[bt] = std::bind( &MultiSEIR::getWaningImmSusc, this, std::placeholders::_1, std::placeholders::_2, C0, r ) ;
    
}

const double MultiSEIR::getSusceptibility( const Bird& bird, const StrainIdType& strainId, const uint& t ) {
    
    if ( getStrainList( bird.immState ).size() >= maxStrains )
        return 0. ; // prevent exposure to more than maxStrains
    
    if ( isImmuneToStrain( bird, strainId ) )
        return 0. ; // prevent re-infection with previously encountered strain
    else
        return getBirdInnateSusceptibility( bird, t ) * getCrossReactiveSusceptibility( bird, strainId ) ;
    
}

const double MultiSEIR::getHazard( const Bird& infector, const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight ) {
    
    double beta = transmissibility[strain] ;
    double susc = getSusceptibility( infectee, strain, t ) ;
    
    return beta * susc * weight ;
    
}

// overload of previous method, takes only the infectee as parameter and not the infector
const double MultiSEIR::getHazard( const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight ) {

    double beta = transmissibility[strain] ;
    double susc = getSusceptibility( infectee, strain, t ) ;
    
    return beta * susc * weight ;
    
}

// returns susceptibility sigma if chicken has been previously exposed to any strain
double MultiSEIR::getCrossReactiveSusceptibilitySimple( const Bird& bird, const StrainIdType& strainId, const double& sigma ) {
    
    if ( bird.immState == 0 )
        return 1.;
    else
        return sigma;
    
}

double MultiSEIR::getCrossReactiveSusceptibilityMultiplicative( const Bird& bird, const StrainIdType& strainId, const double& sigma ) {
    
    if ( bird.immState == 0 )
        return 1.;
    else {
        
        uint nExp = static_cast<uint>( strainList[bird.immState].size() ); // number of previous exposures
        return pow( sigma, nExp );
        
    }
    
}


double MultiSEIR::getConstantSusc( const Bird& bird, const uint& t, const double& C0 ) {
    
    return C0 ;

}

double MultiSEIR::getWaningImmSusc( const Bird& bird, const uint& t, const double& C0, const double& r ) {
    
    return C0 * ( 1. - exp( - r * ( t - bird.t_birth ) ) ) ;

}


void MultiSEIR::addSingleSettingEffect( std::function< void ( HoldsBirds&, const Clock&, Random& ) > f ) {
    
    settingEffectFunctions.push_back( f ) ;

}

void MultiSEIR::addStateTransition( std::function< void ( const Clock&, Random& ) > f ) {
    
    transitionFunctions.push_back( f ) ;

}

// checks if 'bird' is immune to 'strain'
// bird gets immunity to strain right after successful exposure
bool MultiSEIR::isImmuneToStrain( const Bird& bird, const StrainIdType& strain ) {
    
    auto& strainsImm = getStrainList( bird.immState ) ;
    for ( StrainIdType s : strainsImm ) {
        
        if ( s == strain )
            return true ;
            
    }
    return false ;

}

void MultiSEIR::pushIncubationEvent( pBird& bird, const StrainIdType& strain, const uint& tEvent) {
    
    incubEventQ.pushEvent( BirdStatusEvent( bird, strain, tEvent ) ) ;

}

void MultiSEIR::applyDirectInfection( pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen ) {
    
    bool wasInfectious = bird->isInfectious() ;
    
    addImmunity( *bird, strain ) ;
    addInfection( *bird, strain ) ;
    
    double recProb = this->getRecoveryProb( strain ) ;
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
void MultiSEIR::applyTransitionEvents( const Clock& clock, Random& randGen ) {
    
    uint tCurr = clock.getCurrTime() ;

    // i) apply incubation events
    
    while ( true ) {
        
        if ( incubEventQ.empty() )   // no more events
            break;
        
        const BirdStatusEvent& eventCurr = incubEventQ.top() ;
    
        if ( eventCurr.t <= tCurr ) {
            
            
            if ( this->BaseCompartmentalModel::applyIncubationEvent( eventCurr ) ) {
                double recProb = getRecoveryProb( eventCurr.strain ) ;
                if ( recProb > 0. ) {
                    uint timeToEvent = randGen.getGeom( recProb ) ;
                            
                    recEventQ.pushEvent( BirdStatusEvent( eventCurr.targetBird, eventCurr.strain, tCurr + timeToEvent ) ) ;
                }
         
            }
                    
            incubEventQ.pop() ;
            
        }
        else
            break ;                 // no more events for this time step
    }
    
    // ii) apply recovery events
    
    while ( true ) {
        
        if ( recEventQ.empty() )
            break ;
        
        const BirdStatusEvent& currEvent = recEventQ.top() ;
        
        if ( currEvent.t <= tCurr ) {
            this->BaseCompartmentalModel::applyRecoveryEvent( currEvent );
            recEventQ.pop();
        }
        else
            break;
    }
    
}
