//
//  base_contamination.cpp
//  CBM
//
//

#include "base_contamination.hpp"


// default constructor
BaseContEnv::BaseContEnv(): currentStrains() {};

// shuffle order of strains (adds some noise, avoids artefactual advantages)
void BaseContEnv::shuffleStrains( Random& randGen ) {
    randGen.shuffleVector( currentStrains );
}

// get number of distinct strains currently present in the environment
std::vector<StrainIdType>& BaseContEnv::getCurrentStrains() {
    return currentStrains;
}
