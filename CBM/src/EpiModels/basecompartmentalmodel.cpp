//
//  basecompartmentalmodel.cpp
//  CBM
//
//

#include <stdio.h>
#include "basecompartmentalmodel.hpp"

//====== BaseCompartmentalModel ======//

BaseCompartmentalModel::BaseCompartmentalModel( uint nStrains_, uint maxStrains_ ): nStrains( nStrains_ ),
    maxStrains( maxStrains_ ),
    transmissibility( nStrains_, 0. ),
    incubationProb( nStrains_, 0. ),
    recoveryProb(nStrains, 0.),
    envTransmissibility(0.),
    birdSheddingProb(0.),
    envDecayProb(0.)
{
    // check max number of strains is allowed
    assert( nStrains <= std::numeric_limits<StateType>::digits ) ;
    
    // Enumerate combinations of strains for each infectious state
    
    viableStates = {} ;
    strainList   = {} ;
    
    // add fully naive state, indexed by 0
    viableStates.push_back( 0 ) ;
    strainList[0] = {} ;
    
    for ( int k = 1; k <= maxStrains; ++k ) {

        std::string inf_state( k, 1 ); // k leading 1's
        inf_state.resize( nStrains, 0 ); // N-K trailing 0's
         
        // print integers and permute bitmask
        do {
            
            StateType r = 0 ;
            std::vector<StrainIdType> strainsTmp = {} ;
            strainsTmp.reserve( maxStrains ) ;

            for ( int i = 0; i < nStrains; ++i ) {
                if ( inf_state[i] ) {
                    r += ( constants::ONE64 << i ) ;
                    strainsTmp.push_back( i ) ;
                }
            }
            
            viableStates.push_back( r ) ;
            strainList[r] = strainsTmp ;
            
        } while ( std::prev_permutation( inf_state.begin(), inf_state.end()) ) ;
    
    }

                                    
    setFarmTransmissionKernel( nullptr );
    adaptiveGridLambda = 1.;
    expon_a_farm = 0.;
    expon_b_farm = 0.;
                                    
    assignSettingWeight = WeightManager();
    
    lngTrackerMngr = nullptr ;
    this->recordRecovery = std::bind( &BaseCompartmentalModel::recordRecoveryDoNothing, this,
                                      std::placeholders::_1, std::placeholders::_2 ) ;
                                    
}



void BaseCompartmentalModel::applySuccessfulExposure(pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen) {
    
    // modify infectee
    bird->hazard = randGen.getExpo() ;
    bird->pendingInfections += 1  ;
    addImmunity( *bird, strain ) ;
    
    // spawn incubation event
    double incProb = getIncubationProb( strain ) ;
    if ( incProb > 0. ) { // ?? raise error if not
        
        uint timeToEvent = randGen.getGeom( incProb ) ;
        pushIncubationEvent( bird, strain, tCurr + timeToEvent ) ;
        
    }
}


void BaseCompartmentalModel::shuffleStrainList( Random& randGen ) {
    
    for ( auto& item : strainList ) {
        
        randGen.shuffleVector( item.second ) ;
        
    }
    
}


void BaseCompartmentalModel::setTransmissibility(const std::vector<double>& trVec, const uint& dt) {
    assert( trVec.size() == nStrains );
    for (uint i = 0; i < nStrains; i++) {
        transmissibility[i] = trVec[i] * dt;
    }
}
    
void BaseCompartmentalModel::setIncubationProb(const std::vector<double>& incubationPeriodVec, const uint& dt) {
    assert( incubationPeriodVec.size() == nStrains );
    for (uint i = 0; i < nStrains; i++) {
        if (incubationPeriodVec[i] == 0)
            incubationProb[i] = 1.;
        else
            incubationProb[i] = 1. - exp( ( - (double)dt ) / ( incubationPeriodVec[i] + 1. ) );
        // N.B. adding +1 because getGeom mean is T - 1, not T
    }
}
    
void BaseCompartmentalModel::setRecoveryProb(const std::vector<double>& recoveryPeriodVec, const uint& dt) {
    assert( recoveryPeriodVec.size() == nStrains );
    for (uint i = 0; i < nStrains; i++) {
        if (recoveryPeriodVec[i] == 0)
            incubationProb[i] = 1.;
        else
            recoveryProb[i] = 1. - exp( - ( (double)dt ) / ( recoveryPeriodVec[i] + 1 ) );
        // N.B. adding +1 because getGeom mean is T - 1, not T
    }
}

void BaseCompartmentalModel::setEnvTransmissibility(const double &envTransmissibility_) {
    envTransmissibility = envTransmissibility_;
}

void BaseCompartmentalModel::setEnvDecayProb(const double& envDecayProb_) {
    envDecayProb = envDecayProb_;
}

void BaseCompartmentalModel::setBirdSheddingProb(const double &birdSheddingProb_) {
    birdSheddingProb = birdSheddingProb_;
}

void BaseCompartmentalModel::setFarmTransmissionKernel( std::function<double (const double&)> kernel ) {
    
    farmTransmissionKernel = kernel;
    
}

void BaseCompartmentalModel::setAdaptiveGridLambda( const double &lamda ) {
    
    adaptiveGridLambda = lamda;
    
}

void BaseCompartmentalModel::setFarmTransmissionExponents( const double& expon_a, const double& expon_b ) {
    
    expon_a_farm = expon_a;
    expon_b_farm = expon_b;
    
}

const double BaseCompartmentalModel::getTransmissibility(const StrainIdType& strainId) const {
    return transmissibility[strainId];
}

const double BaseCompartmentalModel::getIncubationProb(const StrainIdType& strainId) const {
    return incubationProb[strainId];
}

const double BaseCompartmentalModel::getRecoveryProb(const StrainIdType& strainId) const {
    return recoveryProb[strainId];
}

double BaseCompartmentalModel::getEnvTransmissibility() const {
    return envTransmissibility;
}

double BaseCompartmentalModel::getEnvDecayProb() const {
    return envDecayProb;
}

double BaseCompartmentalModel::getBirdSheddingProb() const {
    return birdSheddingProb;
}

double BaseCompartmentalModel::getAdaptiveGridLambda() const {
    return adaptiveGridLambda;
}

double BaseCompartmentalModel::getFarmExponentA() const {
    return expon_a_farm;
}

double BaseCompartmentalModel::getFarmExponentB() const {
    return expon_b_farm;
}

std::function<double (const double&)> BaseCompartmentalModel::getFarmTransmissionKernel() const {
    
    return farmTransmissionKernel;
    
}

uint BaseCompartmentalModel::getNstrains() const {
    return nStrains;
}

std::vector<StrainIdType>::const_iterator BaseCompartmentalModel::beginStrainList(const StateType& s) {
    
    return strainList[s].begin();
    
}

std::vector<StrainIdType>::const_iterator BaseCompartmentalModel::endStrainList(const StateType& s) {
    
    return strainList[s].cend();

}

const std::vector<StrainIdType>& BaseCompartmentalModel::getStrainList(const StateType& s) {
    
    return strainList[s] ;
    
}


void BaseCompartmentalModel::setLineageTrackerManager( std::shared_ptr<LineageTrackerManager> lngTrackerMngr_ ) {
    
    lngTrackerMngr = lngTrackerMngr_ ;
    
    this->recordRecovery = std::bind( &BaseCompartmentalModel::recordRecoveryFunc, this,
                                      std::placeholders::_1, std::placeholders::_2 ) ;
    
}

// make bird infectious w/ single strain/pathogen
// returns 'true' if event resolves successfully or 'false'
// an event may not trigger if bird does not exist anymore
bool BaseCompartmentalModel::applyIncubationEvent( const BirdStatusEvent& ev ) {
    
    const pBird& bird = ev.targetBird;
    const BirdID& evBirdID = ev.birdID;
    
    if ( bird->isValid() and bird->id == evBirdID ) {  // check bird is in use and is the same
        
        bool wasInfectious = bird->isInfectious();  // check whether bird was infectious bf event
        
        addInfection( *bird, ev.strain );
        
        if ( !wasInfectious )
            bird->owner->swapToInfected( bird->index );      // tell holdsbirds new infectious bird
        
        bird->pendingInfections -= 1 ; // reduce pending infection counter
        
        return true;
        
    }
    
    return false;
    
};

// make bird clear infection w/ single strain pathogen
// returns 'true' if event resolves successfully or 'false'
// an event may not trigger if bird does not exist anymore
bool BaseCompartmentalModel::applyRecoveryEvent( const BirdStatusEvent& ev ) {
    
    const pBird& bird = ev.targetBird;
    const BirdID& evBirdID = ev.birdID;
    
    if ( bird->isValid() and bird->id == evBirdID ) {               // check bird still exists somewhere
        
        bool wasInfectious = bird->isInfectious() ;  // check whether bird was infectious bf event
        
        recordRecovery( *bird, ev.strain ) ; // Record lineage death event
        
        removeInfection(*bird, ev.strain) ;
        
        if ( wasInfectious and ( !bird->isInfectious() ) )
            bird->owner->swapToSusceptible(bird->index) ;   // tell holdsbirds about new recovered bird
        
        return true;
        
    }
    
    return false;
    
};

void BaseCompartmentalModel::recordRecoveryDoNothing( const Bird& bird, const StrainIdType& strain ) {} ;
void BaseCompartmentalModel::recordRecoveryFunc( const Bird& bird, const StrainIdType& strain ) {
 
    lngTrackerMngr->addRecoveryEvent( bird, strain ) ;
    
}




// Other functions

// update bird immune status (new immunity gained)
void addImmunity(Bird& bird, const StrainIdType& strainId) {
    bird.immState += constants::ONE64 << strainId;
}

// update bird infectious status
void addInfection(Bird& bird, const StrainIdType& strainId) {
    bird.infState += constants::ONE64 << strainId;
}

void removeInfection(Bird& bird, const StrainIdType& strainId) {
    bird.infState -= constants::ONE64 << strainId;
}
