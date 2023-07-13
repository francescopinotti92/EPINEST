//
//  holdsbirds.hpp
//  CBM
//
//

#ifndef holdsbirds_h
#define holdsbirds_h

#include "bird.hpp"
#include "base_contamination.hpp"
#include "infected_set.hpp"
#include "event_system.hpp"
#include "ChickenPool.hpp"
#include "random.hpp"
#include <string>

// Base class for entities that own bird pointers.

class HoldsBirds {
public:
    HoldsBirds();
    HoldsBirds( const std::vector<BirdType>& birdTypes_, const uint& initialCapacityBirds_ );
    virtual ~HoldsBirds() = default;
    virtual Bird& insertBird(pBird&& bird) = 0;
    virtual pBird removeBird(const uint& index) = 0;
    virtual pBird removeDeadBird(const uint& index) = 0;
    virtual void notifySuccessfulExposure( Bird& bird ) = 0;
    virtual std::string getStringID() = 0;
    virtual NodeType getSettingType() = 0;

    // Non-virtual functions
    void swapToInfected(const uint& index);
    void swapToSusceptible(const uint& index);

    bool trades(const BirdType& bt);

    inline uint getSizeBirds()                                          { return birds.size(); }
    inline uint getSizeBirdsInf()                                       { return birds.size_infected(); }
    inline uint getCapacity()                                           { return birds.capacity(); }
    inline uint getAvailableSpace()                                     { return birds.available_space();}
    inline uint getTotalContaminatedUnits()                             { return contEnv->getTotalUnits(); }
    inline double getBaseWeight()                                       { return baseWeight; }
    pBird& getBirdDense( const uint& denseIndex )                       { return birds.getBirdDense(denseIndex); }
    pBird& getBirdSparse( const uint& sparseIndex )                     { return birds.getBirdSparse( sparseIndex ) ; }
    pBird& getRandomBird( Random& randGen );
    BaseContEnv& getContEnv()                                           { return *contEnv; }
    // from great power comes great responsibility
    infected_set& getBirds()                                            { return birds; }
    void setContEnv( BaseContEnv* newContEnv )                          { contEnv = newContEnv; }
    void setBaseWeight( const double& weight )                          { baseWeight = weight; }
    
    const std::vector<BirdType>& getTradedBirdTypes() const             { return birdTypes; }
    
    pBirdVecType::iterator begin() { return birds.begin(); }
    pBirdVecType::iterator end() { return birds.end(); }
    pBirdVecType::iterator endInf() { return birds.endInf(); }
    //pBirdVecType::const_iterator cbegin() const;
    //pBirdVecType::const_iterator cend() const;
    //pBirdVecType::const_iterator cendInf() const;
    
protected:
    std::vector<BirdType> birdTypes;
    infected_set birds;
    BaseContEnv* contEnv;
    double baseWeight;
};


#endif /* holdsbirds_h */
