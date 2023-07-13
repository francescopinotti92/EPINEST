//
//  basecompartmentalmodel.hpp
//  CBM
//
//

#ifndef basecompartmentalmodel_h
#define basecompartmentalmodel_h

#define INITIAL_QUEUE_CAPACITY 10000

#include "WeightManager.hpp"
#include "types.hpp"
#include "bird.hpp"
#include "holdsbirds.hpp"
#include "event_system.hpp"
#include "random.hpp"
#include "lineage_tracker_manager.hpp"
#include <map>
#include <math.h>
#include <bitset>


/*
 This class exemplifies how we can create a compartmental model.
 In this example, susceptibility is 0 if a strain has already been encountered (complete immunity), and 1 otherwise (no immunity).
 */

class BaseCompartmentalModel {
public:
    BaseCompartmentalModel( uint nStrains, uint maxStrains );
    virtual ~BaseCompartmentalModel() = default;
    virtual bool isImmuneToStrain( const Bird& bird, const StrainIdType& strain ) = 0;
    virtual void pushIncubationEvent( pBird& bird, const StrainIdType& strain, const uint& tEvent) = 0;
    virtual void applyDirectInfection( pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen ) = 0;
    virtual const double getHazard( const Bird& infector, const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight = 1. ) = 0;
    const double getHazard( const Bird& infectee, const StrainIdType& strain, const uint& t, const double& weight = 1.);
    virtual const double getSusceptibility( const Bird& bird, const StrainIdType& strain, const uint& t ) = 0;
    virtual void reset() = 0;
    
    virtual void applyTransitionEvents( const Clock& clock, Random& randGen ) = 0;

    void applySuccessfulExposure( pBird& bird, const StrainIdType& strain, const uint& tCurr, Random& randGen );
    
    //inline double hazardify_prob(const double & p) const { return -std::log(1. - p); }
    
    inline bool isStrainValid( const StrainIdType& strain ) const {
        return ( strain < nStrains ) ?  true : false;
    };
    
    void setLineageTrackerManager( std::shared_ptr<LineageTrackerManager> lngTrackerMngr ) ;
    
    
    void shuffleStrainList( Random& randGen );

    virtual void setTransmissibility(const std::vector<double>& trVec, const uint& dt);
    void setIncubationProb(const std::vector<double>& incubationPeriodVec, const uint& dt);
    void setRecoveryProb(const std::vector<double>& recoveryPeriodVec, const uint& dt);
    void setEnvTransmissibility(const double& envTransmissibility);
    void setBirdSheddingProb(const double& birdSheddingProb);
    void setEnvDecayProb(const double& envDecayProb);
    void setAdaptiveGridLambda( const double& lamda );
    void setFarmTransmissionKernel(std::function<double (const double&)> kernel);
    void setFarmTransmissionExponents(const double& expon_a, const double& expon_b);
    
    const double getTransmissibility(const StrainIdType& strainId) const;
    const double getIncubationProb(const StrainIdType& strainId) const;
    const double getRecoveryProb(const StrainIdType& strainId) const;
    double getEnvTransmissibility() const;
    double getEnvDecayProb() const;
    double getBirdSheddingProb() const;
    std::function<double (const double&)> getFarmTransmissionKernel() const;
    double getAdaptiveGridLambda() const;
    double getFarmExponentA() const;
    double getFarmExponentB() const;
    double getNodeWeight( const NodeType& nt ) { return assignSettingWeight.get_weight( nt ); }
    WeightManager& getWeightManager() { return assignSettingWeight; }
    
    uint getNstrains() const;
    
    bool applyIncubationEvent( const BirdStatusEvent& ev ) ;
    bool applyRecoveryEvent( const BirdStatusEvent& ev ) ;
    
    std::vector<StrainIdType>::const_iterator beginStrainList(const StateType& s);
    std::vector<StrainIdType>::const_iterator endStrainList(const StateType& s);
    const std::vector<StrainIdType>& getStrainList(const StateType& s);

protected:
    uint nStrains;
    uint maxStrains;
    std::vector<double> transmissibility; // bare transmissibility (rate?) strains
    std::vector<double> incubationProb;  // bare E->I progression probability (per time step)
    std::vector<double> recoveryProb;  // bare recovery probability (per time step)
    double envTransmissibility;
    double birdSheddingProb;
    double envDecayProb; // daily probability of a contaminated unit decaying
    std::function<double (const double&)> farmTransmissionKernel;
    double adaptiveGridLambda;
    double expon_a_farm;
    double expon_b_farm;
    
    
    std::vector<StateType> viableStates;
    std::unordered_map<StateType, std::vector<StrainIdType>> strainList;
    WeightManager assignSettingWeight;
    
    
    std::shared_ptr<LineageTrackerManager> lngTrackerMngr ;
    std::function<void ( const Bird&, const StrainIdType& )> recordRecovery;
    void recordRecoveryDoNothing( const Bird& bird, const StrainIdType& strain);
    void recordRecoveryFunc( const Bird& bird, const StrainIdType& strain );
    

};

void addImmunity(Bird& bird, const StrainIdType& strainId);
void addInfection(Bird& bird, const StrainIdType& strainId);
void removeInfection(Bird& bird, const StrainIdType& strainId);


    
#endif /* basecompartmentalmodel_h */
