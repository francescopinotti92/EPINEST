//
//  compartmentalmodelbuilder.hpp
//  CBM
//
//

#ifndef compartmentalmodelbuilder_hpp
#define compartmentalmodelbuilder_hpp


#include "examplemodel.hpp"
#include "MultiSEIR_model.hpp"
#include "SEIR_model.hpp"
#include "epidemic_dynamics.hpp"
#include "distance_kernels.hpp"
#include "checks.hpp"
#include "clock.hpp"
#include <nlohmann/json.hpp>
#include <fstream>


class SIRModelMaker {
public:
    SIRModelMaker( std::string path_to_params_ );
    ~SIRModelMaker() = default;
    ExampleModel* makeModel();
    ExampleModel* getModel();
protected:
    std::string path_to_params;
    ExampleModel* epiModel;
};

class SingleStrainSEIRMaker {
public:
    SingleStrainSEIRMaker( std::string path_to_params_ );
    ~SingleStrainSEIRMaker() = default;
    SEIR* makeModel();
    SEIR* getModel();
protected:
    std::string path_to_params;
    SEIR* epiModel;
};

class MultiStrainSEIRMaker {
public:
    MultiStrainSEIRMaker( std::string path_to_params_ );
    ~MultiStrainSEIRMaker() = default;
    MultiSEIR* makeModel();
    MultiSEIR* getModel();
protected:
    std::string path_to_params;
    MultiSEIR* epiModel;
};



ExampleModel* MakeExampleModel( std::string path_to_params );


#endif /* compartmentalmodelbuilder_hpp */
