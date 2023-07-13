//
//  edge_manager.cpp
//  CBM
//
//

#include "node.hpp"
#include "edge_manager.hpp"

template<typename T>
edge_manager<T>::edge_manager() {
    this->sampler = &edge_manager::computeAliasTable;
};

template<typename T>
T* edge_manager<T>::getRandomNbr(Random &randGen) {
    uint idx = (this->*sampler)(randGen);
    return adjList[idx].first;
}

template<typename T>
void edge_manager<T>::addNbr(T *newNbr, double weight) {
    adjList.push_back( std::make_pair(newNbr, weight) );
}

template<typename T>
uint edge_manager<T>::computeAliasTable(Random &randGen) {
    // get weights
    double wtot = 0.;
    std::vector<double> weights = {};
    weights.reserve( adjList.size() );
    for ( std::pair<T*, double>& el: adjList ) {
        checkPositive( el.second );
        weights.push_back( el.second );
        wtot += el.second;
    }
    
    checkStrictlyPositive( wtot );
    
    // normalize weights
    std::vector<double> probs;
    probs.reserve( weights.size() );
    for ( double weight: weights )
        probs.push_back( weight / wtot );

    // use weights to compute an alias table
    wtable.setAliasTable( probs );

    // set sampler function
    this->sampler = &edge_manager::sampleAliasTable;
    
    // finally sample
    return (this->*sampler)(randGen);
    
}

template<typename T>
uint edge_manager<T>::sampleAliasTable(Random &randGen) {
    return randGen.getCustomDiscrete(wtable);
}

template class edge_manager<NodeFarm>;
template class edge_manager<NodeMman>;
template class edge_manager<NodeMarket>;

