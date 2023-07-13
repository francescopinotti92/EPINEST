//
//  contamination.hpp
//  CBM_ECS
//
//

#ifndef contamination_h
#define contamination_h

#include "base_contamination.hpp"

/*
 ContUnit (Contaminated Unit) represents the fundamental contribution shed by a sick bird
 into the environment.
*/

/*
struct ContUnit {
    BirdIdType birdId;
    StrainIdType strainId;
    ContUnit(BirdIdType birdId_, StrainIdType strainId_);
};
 */

/*
 ContEnv (Contamined Environment) basically wraps a container of Contaminated Units.
*/

/*
class ContEnv {
public:
    ContEnv(const uint& initialCapacity = 0);
    ContUnit& getRandomUnit(Random& randGen);
    
    // Manages natural decay of contaminated material.
    // compartmentalModel::envDecayProb is the probability per unit time that a Contaminated Unit disappears
    
    //void removeFraction(Random& randGen);
    
    void insert(const BirdIdType& birdId, const StrainIdType& strainId);
    void remove(const uint& i);
    void clearUrn();
    uint sizeUrn();
    
private:
    std::vector<ContUnit> urn; // contains units
};
 */

class ContEnv: public BaseContEnv {
public:
    ContEnv( const uint& nStrains );
    ~ContEnv();
    void clear()                                                                                override;
    void addUnits( const BirdID& birdID, const StrainIdType& strain, const uint& amount )       override;
    void removeUnits( Random& randGen, const double& decayRate )                                override;
    uint getTotalUnits()  const                                                                 override;
    uint getTotalUnits( const StrainIdType& strain )  const                                     override;
    BirdID getRandomContaminatedUnitID( const StrainIdType& strain, Random& randGen )           override;
    
private:
    uint totalUnits;
    std::vector<uint> nUnits;
};

class ContEnvDetailed: public BaseContEnv {
public:
    ContEnvDetailed( const uint& nStrains = 1, const uint& initialCapacity = 0 );
    ~ContEnvDetailed();
    void clear()                                                                                override;
    void addUnits( const BirdID& birdID, const StrainIdType& strain, const uint& amount )       override;
    void removeUnits( Random& randGen, const double& decayRate )                                override;
    uint getTotalUnits()  const                                                                 override;
    uint getTotalUnits( const StrainIdType& strain ) const                                      override;
    BirdID getRandomContaminatedUnitID( const StrainIdType& strain, Random& randGen )           override;
    
private:
    uint totalUnits;
    std::vector<std::vector<BirdID>> units;
};


#endif /* contamination_h */
