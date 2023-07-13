//
//  WeightManager.hpp
//  CBM
//
//

#ifndef WeightManager_hpp
#define WeightManager_hpp


#include <unordered_map>
#include "baseNode.hpp"
#include "types.hpp"


// This class manages assignment of setting-specific weights
class WeightManager {
public:
    WeightManager();
    void setConstantReturn( NodeType nt, double val );
    double get_weight( BaseNode* node );
    double get_weight( const NodeType& nt );
private:
    std::unordered_map<NodeType, std::function<double()>> nodeType2Weight;
};


#endif /* WeightManager_hpp */
