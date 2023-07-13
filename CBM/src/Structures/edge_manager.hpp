//
//  edge_manager.hpp
//  CBM
//
//

#ifndef edge_manager_hpp
#define edge_manager_hpp

#include "checks.hpp"
#include "random.hpp"
#include "types.hpp"
#include <vector>
#include <algorithm>

template <typename T>
class edge_manager {
public:
    edge_manager();
    T* getRandomNbr(Random& randGen);
    void addNbr(T* newNbr, double weight);
private:
    uint computeAliasTable(Random& randGen);
    uint sampleAliasTable(Random& randGen);
    uint (edge_manager<T>::* sampler)(Random& randGen);
    std::vector<std::pair<T*, double>> adjList;
    AliasTable wtable;
};



#endif /* edge_manager_hpp */
