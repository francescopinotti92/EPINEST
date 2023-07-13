//
//  AreaManager.hpp
//  CBM
//
//

#ifndef AreaManager_hpp
#define AreaManager_hpp

#include "node.hpp"
#include "network_structures.hpp"
#include "random.hpp"
#include <unordered_map>
#include <vector>

class NodeFarm;

class AreaManager {
public:
    AreaManager( std::vector<BirdType> bts, uint Na, uint Nm );
    
    void addAreaNbr( const uint& a1, const uint& a2 );
    void updateFarmProductionDistribution();
    void reset();
    void increaseTotalFarmCount( const uint& a ) { ++nTotalFarmsByArea[a] ; }
    void registerTradingFarm( NodeFarm* nf );
    void unregisterTradingFarm( NodeFarm* nf );
    std::vector<uint> getNeighboringAreas( const uint& areaId, const uint& nAreas, Random& randGen );
    uint getRandomAreaUsingFarmProduction( const BirdType& bt, Random& randGen );
    DiscreteDistribution synthesizeMarketChoiceDistribution( const BirdType& bt, const std::vector<uint>& areasVec );
    DiscreteDistribution synthesize_P_al_Subset( const BirdType& bt, const uint areaId, const std::vector<uint>& marketsVec );
    
    void set_P_a( const BirdType& bt, const std::vector<double>& N_a );
    void set_P_al( const BirdType& bt, const std::vector<std::vector<double>>& P_al );
    uint getTotalFarms( const uint& a ) const { return nTotalFarmsByArea[a]; }
    std::vector<NodeFarm*>& getTradingFarmsArea( const BirdType& bt, const uint& a );
    std::vector<double>& get_P_a( const BirdType& bt );
    std::vector<double>& get_P_al( const BirdType& bt, const uint& a );
    std::vector<double> synthesize_P_ajl( const BirdType& bt, const uint marketId, const std::vector<uint>& areasVec );

    uint getNumberOfAreas() const { return Na; }
    
    //std::vector<double>& get_Q_al( const BirdType& bt, const uint& a, const uint& l );
private:
    uint Na;
    uint Nm;
    std::vector<uint> nTotalFarmsByArea;
    std::unordered_map<BirdType, std::vector<std::vector<NodeFarm*>>> tradingFarms;
    std::unordered_map<BirdType, std::vector<double>> P_a; // relative weight associated to region
    std::unordered_map<BirdType, std::vector<std::vector<double>>> P_al;
    std::unordered_map<BirdType, std::vector<std::vector<double>>> Q_al;
    std::unordered_map<BirdType, DiscreteDistribution> farmProduction; // relative production
    //std::unordered_map<BirdType, std::vector<double>> farmProductionRaw; // total production

    net_utils::uw_adjacency_list neighboringAreasNetwork;
};

class AreaManagerAdaptorFarm {
    AreaManagerAdaptorFarm();
   
};

class AreaManagerAdaptorMman {
    AreaManagerAdaptorMman();
   
};



#endif /* AreaManager_hpp */
