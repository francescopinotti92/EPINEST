//
//  WeightManager.cpp
//  CBM
//
//

#include "WeightManager.hpp"

WeightManager::WeightManager() {
    
    // default constructor sets all weights to 1 (no modification)
    for ( const NodeType& nt : allNodeTypes ) {
        
        auto return1 = std::bind( []() { return 1.; } );
        nodeType2Weight[nt] = return1;
        
    }
    
};

// sets return value to constant
void WeightManager::setConstantReturn( NodeType nt, double val ) {
    
    auto returnConst = std::bind( []( const double& retVal ) { return retVal; }, val );
    nodeType2Weight[nt] = returnConst;
    
}


double WeightManager::get_weight( BaseNode *node ) {
    
    return nodeType2Weight[ node->getNodeType() ]();
    
}

double WeightManager::get_weight( const NodeType& nt ) {
    
    return nodeType2Weight[nt]();
    
}
