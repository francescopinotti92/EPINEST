//
//  spatial_farm_transmission_manager.hpp
//  CBM
//
//

#ifndef spatial_farm_transmission_manager_hpp
#define spatial_farm_transmission_manager_hpp

#include <stdio.h>

#include "node.hpp"
#include "epidemic_dynamics.hpp"
#include "clock.hpp"
#include "random.hpp"
#include "ElementPool.hpp"
#include "distance_kernels.hpp"
#include "cell.hpp"
#include "types.hpp"
#include <functional>


struct EpiFarmCell {
    
    EpiFarmCell();
    EpiFarmCell( const uint& ID, const Geom::Cell& cell );
    
    void reset();
    void checkFarms();
    void registerFarm( NodeFarm* nf );
    void computeMaxSusc( const double& expon = 1. );
    bool containsFarm( NodeFarm* nf );
    bool hasInfectedFarms() const { return ( infectedFarms.size() > 0 ) ? true : false ; }
    double getMaxSusc() const { return maxSusc; }
    ElementPool<NodeFarm*>& getInfectedFarms()    { return infectedFarms ; }
    ElementPool<NodeFarm*>& getSusceptibleFarms() { return suscFarms ; }

    uint ID;
    Geom::Cell cell;
    double maxSusc;
    std::vector<NodeFarm*> farms;
    ElementPool<NodeFarm*> infectedFarms;
    ElementPool<NodeFarm*> suscFarms;
    
};

class SpatialFarmTransmissionManager {
public:
    //SpatialFarmTransmissionManager();
    SpatialFarmTransmissionManager( const double& lam, std::vector<NodeFarm*>* farmVec, std::function<double(const double&)> kernel, const double& expon_a, const double& expon_b, BaseInfectionTracker* tracker = nullptr );
    void reset();
    void doTransmissionStep( const Clock& clock, Random& randGen, BaseCompartmentalModel* compModel );

private:
    std::function< double ( const double& ) > evalKernel;
    std::function< void ( HoldsBirds&, HoldsBirds&, const Clock&, BaseCompartmentalModel&, Random& ) > applyTransmission2Settings;
    void doPairwiseTransmissionWithinCell( NodeFarm* nf, EpiFarmCell* targetCell, Random& randGen, const Clock& clock, BaseCompartmentalModel* compModel );
    void doConditionalEntryTransmission( NodeFarm* nf, EpiFarmCell* sourceCell, EpiFarmCell* targetCell, Random& randGen, const Clock& clock, BaseCompartmentalModel* compModel );
    double getKcells( EpiFarmCell* c1, EpiFarmCell* c2 );
    
    BaseInfectionTracker* tracker;
    std::vector<EpiFarmCell> epiCells;
    ElementPool<EpiFarmCell*> infectedCells;
    std::vector<NodeFarm*>* farmVec;
    std::vector<double> Kcells;
    uint nCells;
    double expon_a;
    double expon_b;
};


#endif /* spatial_farm_transmission_manager_hpp */
