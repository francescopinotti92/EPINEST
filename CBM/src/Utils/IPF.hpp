//
//  IPF.hpp
//  CBM
//
//

#ifndef IPF_hpp
#define IPF_hpp

#include "network_structures.hpp"
#include "warnings.hpp"
#include <cassert>
#include <math.h>
#include <numeric>
#include <fstream>
#include <iostream>

namespace net_utils {

void IPF(w_adjacency_list& adj,
         const std::vector<double>& psum_r,
         const std::vector<double>& psum_c,
         const uint& maxIter = 100,
         const double& convergenceThreshold = 0.001,
         const double& fillZero = 0.);
}

#endif /* IPF_hpp */
