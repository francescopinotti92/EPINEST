//
//  examplemodel.hpp
//  CBM
//
//

#ifndef examplemodel_hpp
#define examplemodel_hpp

//#include "epidemic_dynamics.hpp"
#include "basecompartmentalmodel.hpp"
#include <functional>

// example compartmental model
//
class ExampleModel: public BaseCompartmentalModel {
public:
    ExampleModel( uint nStrains, uint maxStrains );
    
    void reset() override;
    const double getSusceptibility( const Bird& bird, const StrainIdType& strainId, const uint& t ) override;
    const double getHazard( const Bird& infector, const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight = 1. ) override;
    bool isImmuneToStrain( const Bird& bird, const StrainIdType& strain ) override;
    void applyTransitionEvents( const Clock& clock, Random& randGen ) override;
    void pushIncubationEvent( pBird& bird, const StrainIdType& strain, const uint& tEvent ) override;
    void applyDirectInfection( pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen ) override;
    void setTransmissibility( const std::vector<double>& trVec, const uint& dt ) override;
    void setConstantInnateImmunity( const BirdType& bt, const double& C0 );
    void setWaningInnateImmunity( const BirdType& bt, const double& C0, const double& r );

       
    void addSingleSettingEffect( std::function< void ( HoldsBirds&, const Clock&, Random& ) > f );
    void addStateTransition( std::function< void ( const Clock&, Random& ) > f );
    
    EventPriorityQueue<BirdStatusEvent>& getIncubEventQueue() { return incubEventQ; }
    EventPriorityQueue<BirdStatusEvent>& getRecEventQueue() { return recEventQ; }

private:
    std::map<StateType, std::vector<double>> birdAcquiredSusceptibility; // n.b.: 1 << n equals 2ˆn
    std::unordered_map<BirdType, std::function<double (const Bird&, const uint&)>> birdInnateSusceptibility;
    std::vector<double> hazards;
    double getConstantSusc( const Bird& bird, const uint& t, const double& C0 );
    double getWaningImmSusc( const Bird& bird, const uint& t, const double& C0, const double& r );

    EventPriorityQueue<BirdStatusEvent> incubEventQ;
    EventPriorityQueue<BirdStatusEvent> recEventQ;
    std::vector< std::function< void ( HoldsBirds&, const Clock&, Random& ) > > settingEffectFunctions;
    std::vector< std::function< void ( const Clock&, Random& ) > > transitionFunctions;
};

#endif /* examplemodel_hpp */
