//
//  SEIR_model.hpp
//  CBM
//
//

#ifndef SEIR_model_hpp
#define SEIR_model_hpp

#include "basecompartmentalmodel.hpp"
#include <functional>

/*
 Simplified Compartmental model to simulate a single-strain model
 */
class SEIR: public BaseCompartmentalModel {
public:
    SEIR(uint nStrains);
    
    void reset() override;
    const double getSusceptibility( const Bird& bird, const StrainIdType& strainId, const uint& t ) override;
    const double getHazard( const Bird& infector, const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight = 1. ) override;
    const double getHazard( const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight = 1. );
    bool isImmuneToStrain( const Bird& bird, const StrainIdType& strain ) override;
    void applyTransitionEvents( const Clock& clock, Random& randGen ) override;
    void pushIncubationEvent( pBird& bird, const StrainIdType& strain, const uint& tEvent ) override;
    void applyDirectInfection( pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen ) override;
    void setTransmissibility( const std::vector<double>& trVec, const uint& dt ) override;
    
    void setSimpleInnateImmunity();
    void setCustomInnateImmunity();
    
    void setConstantInnateImmunity( const BirdType& bt, const double& C0 );
    void setWaningInnateImmunity( const BirdType& bt, const double& C0, const double& r );

    void addSingleSettingEffect( std::function< void ( HoldsBirds&, const Clock&, Random& ) > f );
    void addStateTransition( std::function< void ( const Clock&, Random& ) > f );
    
    EventPriorityQueue<BirdStatusEvent>& getIncubEventQueue() { return incubEventQ; }
    EventPriorityQueue<BirdStatusEvent>& getRecEventQueue() { return recEventQ; }

private:
        
    double beta;
    
    std::function<double (const Bird&, const uint&)> getBirdInnateSusceptibility;
    double get1( const Bird& bird, const uint& t ) { return 1. ; }
    double getSusceptibilityFromMap( const Bird& bird, const uint& t ) { return birdInnateSusceptibility[bird.birdType]( bird, t ); }
    
    std::unordered_map<BirdType, std::function<double (const Bird&, const uint&)>> birdInnateSusceptibility;
    double getConstantSusc( const Bird& bird, const uint& t, const double& C0 );
    double getWaningImmSusc( const Bird& bird, const uint& t, const double& C0, const double& r );
    
    EventPriorityQueue<BirdStatusEvent> incubEventQ;
    EventPriorityQueue<BirdStatusEvent> recEventQ;
    std::vector< std::function< void ( HoldsBirds&, const Clock&, Random& ) > > settingEffectFunctions;
    std::vector< std::function< void ( const Clock&, Random& ) > > transitionFunctions;
};

#endif /* SEIR_model_hpp */
