//
//  distance_kernels.hpp
//  CBM
//
//

#ifndef distance_kernels_h
#define distance_kernels_h

#include <math.h>

double shiftedParetoKernel( const double& dist, const double& distMax, const double& distMin, const double& expo, const double& coeff );

double hillLikeParetoKernel( const double& dist, const double& distMax, const double& distMin, const double& expo, const double& coeff );



#endif /* distance_kernels_h */
