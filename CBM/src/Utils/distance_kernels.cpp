//
//  distance_kernels.cpp
//  CBM
//
//

#include "distance_kernels.hpp"

double shiftedParetoKernel( const double& dist, const double& distMax, const double& distMin, const double& expo, const double& coeff ) {
    
    if ( dist > distMax )
        return 0.;
    
    return coeff * pow( 1 + dist / distMin, -expo );
}

double hillLikeParetoKernel( const double& dist, const double& distMax, const double& distMin, const double& expo, const double& coeff ) {
    
    if ( dist < distMin ) {
        return coeff;
    }
    else if ( dist > distMax ) {
        return 0.;
    }
    else {
        return coeff * pow( distMin / dist , expo );
    }
    
}
