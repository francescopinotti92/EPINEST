//
//  BaseEpiModelMaker.hpp
//  CBM
//
//  Created by user on 11/06/2021.
//  Copyright © 2021 Francesco. All rights reserved.
//

#ifndef BaseEpiModelMaker_hpp
#define BaseEpiModelMaker_hpp

#include <stdio.h>

#include "basecompartmentalmodel.hpp"
#include<string>

// Factory class for epi models
class BaseEpiModelMaker {
public:
    BaseEpiModelMaker( std::string path_to_params_ ): path_to_params( path_to_params_ ), epiModel( nullptr ) {};
    virtual ~BaseEpiModelMaker() = default;
    virtual BaseCompartmentalModel* makeModel( bool sampleIndividualInfections ) = 0;
protected:
    std::string path_to_params;
    BaseCompartmentalModel* epiModel;
};


#endif /* BaseEpiModelMaker_hpp */
