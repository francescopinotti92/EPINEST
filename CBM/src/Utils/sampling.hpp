//
//  sampling.hpp
//  CBM
//
//

#ifndef sampling_hpp
#define sampling_hpp

#include "holdsbirds.hpp"
#include "random.hpp"
#include "types.hpp"
#include <vector>

std::vector<uint> sampleInfectedChickensFromSetting( HoldsBirds* setting, Random& randGen, const uint& nBirds, bool sampleLatent = false ) ;


#endif /* sampling_hpp */
