//
//  base_contamination.hpp
//  CBM
//
//

#ifndef base_contamination_hpp
#define base_contamination_hpp

#include "birdID.hpp"
#include "types.hpp"
#include "random.hpp"

class BaseContEnv {
public:
    BaseContEnv();
    
    // virtual methods
    virtual ~BaseContEnv() = default;
    virtual void clear() = 0;
    virtual void addUnits( const BirdID& birdID, const StrainIdType& strain, const uint& amount ) = 0;
    virtual void removeUnits( Random& randGen, const double& decayRate ) = 0;
    virtual uint getTotalUnits() const = 0;
    virtual uint getTotalUnits( const StrainIdType& strain ) const = 0;
    virtual BirdID getRandomContaminatedUnitID( const StrainIdType& strain, Random& randGen ) = 0;
    
    // non-virtual methods
    void shuffleStrains( Random& randGen );
    std::vector<StrainIdType>& getCurrentStrains();

protected:
    std::vector<StrainIdType> currentStrains;
};


#endif /* base_contamination_hpp */
