//
//  compartmentalmodelbuilder.cpp
//  CBM
//
//

#include "compartmentalmodelbuilder.hpp"

SIRModelMaker::SIRModelMaker( std::string path_to_params_ ): path_to_params( path_to_params_ ), epiModel( nullptr ) {};

ExampleModel* SIRModelMaker::makeModel() {
    std::ifstream readFile( path_to_params );
    nlohmann::json params = nlohmann::json::parse( readFile );
    
    requireJsonKey( params, "nStrains" );
    requireJsonKey( params, "maxStrains" );
    requireJsonKey( params, "transmissibility" );
    requireJsonKey( params, "expectedIncubationTimes" );
    requireJsonKey( params, "expectedRecoveryTimes" );
    requireJsonKey( params, "transmissibilityEnvironment" );
    requireJsonKey( params, "environmentDecayProb" );
    requireJsonKey( params, "birdEnvironmentSheddingProb" );
    requireJsonKey( params, "timeStepSize" );

    
    uint nStrains   = params["nStrains"];
    uint maxStrains = params["maxStrains"];
    std::vector<double> trVec = params["transmissibility"];
    std::vector<double> expectedIncubTimeVec = params["expectedIncubationTimes"];
    std::vector<double> expectedRecTimeVec = params["expectedRecoveryTimes"];
    double trEnv = params["transmissibilityEnvironment"];
    double decayProbEnv = params["environmentDecayProb"];
    double sheddingProb = params["birdEnvironmentSheddingProb"];
    uint dt = params["timeStepSize"];

    ExampleModel* model = new ExampleModel( nStrains, maxStrains );

    model->setTransmissibility( trVec, dt );
    model->setIncubationProb( expectedIncubTimeVec, dt );
    model->setRecoveryProb( expectedRecTimeVec, dt );
    model->setEnvTransmissibility( trEnv );
    model->setEnvDecayProb( decayProbEnv );
    model->setBirdSheddingProb( sheddingProb );
    
    // read and set chicken-specific properties
    
    if ( params["innateImmunityParameters"].empty() ) {
        for ( const BirdType& bt: constants::allBirdTypes ) {
            model->setConstantInnateImmunity( bt, 1. );
        }
    }
    else {

        requireJsonKey( params, "innateImmunityParameters" );

        for ( auto& innateParams: params["innateImmunityParameters"].items() ) {
            BirdType bt = constants::getBirdTypeFromString( innateParams.key() );
            auto& vals = innateParams.value();
            
            requireJsonKey( vals, "mode" );
            std::string mode = vals["mode"].get<std::string>();
            
            if ( mode == std::string("constant") ) {
                
                requireJsonKey( vals, "C0" );

                double C0 = vals["C0"].get<double>();
                checkStrictlyPositive( C0 );
                model->setConstantInnateImmunity( bt, C0 );
            }
            else if ( mode == std::string("waning") ) {
                
                requireJsonKey( vals, "C0" );
                requireJsonKey( vals, "r" );

                double C0 = vals["C0"].get<double>();
                double r  = vals["r"].get<double>();
                checkStrictlyPositive( C0 );
                checkPositive( r );
                model->setWaningInnateImmunity( bt, C0, r );
            }
            else {
                throw Warning("innate immunity mode not recognized");
            }
        }
    }
    
    // setup farm spatial transmission Kernel
    std::function< double ( const double& )> farmSpatialKernel;
    
    // set Farm spatial kernel
    {
        
        requireJsonKey( params, "FarmSpatialGridLambda" );
        requireJsonKey( params, "FarmSpatialKernelSizeExponentA" );
        requireJsonKey( params, "FarmSpatialKernelSizeExponentB" );
        requireJsonKey( params, "FarmSpatialKernelType" );

        // adaptive grid creation parameter
        double adaptiveGridLambda = params["FarmSpatialGridLambda"];
        model->setAdaptiveGridLambda( adaptiveGridLambda );
        
        // size exponents
        double expon_a = params["FarmSpatialKernelSizeExponentA"].get<double>();
        double expon_b = params["FarmSpatialKernelSizeExponentB"].get<double>();
        model->setFarmTransmissionExponents( expon_a, expon_b );

        double distMaxKernel = std::numeric_limits<double>::max();
        std::string kernelType = params["FarmSpatialKernelType"].get<std::string>();
        
        if ( kernelType == "ShiftedPareto" ) {
            
            requireJsonKey( params, "FarmSpatialKernelExponent" );
            requireJsonKey( params, "FarmSpatialKernelDistMin" );
            requireJsonKey( params, "FarmSpatialKernelCoefficient" );
            
            double exponent = params["FarmSpatialKernelExponent"].get<double>();
            double distMin  = params["FarmSpatialKernelDistMin"].get<double>();
            double coefficient = params["FarmSpatialKernelCoefficient"].get<double>();

            if ( coefficient > 0. )
                farmSpatialKernel = std::bind( shiftedParetoKernel, std::placeholders::_1, distMaxKernel, distMin, exponent, coefficient);
            else
                farmSpatialKernel = nullptr;
        }
        else if ( kernelType == "HillLikeParetoKernel" ) {
            
            requireJsonKey( params, "FarmSpatialKernelExponent" );
            requireJsonKey( params, "FarmSpatialKernelDistMin" );
            requireJsonKey( params, "FarmSpatialKernelCoefficient" );
            
            double exponent = params["FarmSpatialKernelExponent"].get<double>();
            double distMin  = params["FarmSpatialKernelDistMin"].get<double>();
            double coefficient = params["FarmSpatialKernelCoefficient"].get<double>();
            
            if ( coefficient > 0. )
                farmSpatialKernel = std::bind( hillLikeParetoKernel, std::placeholders::_1, distMaxKernel, distMin, exponent, coefficient );
            else
                farmSpatialKernel = nullptr;
        }
        else
            farmSpatialKernel = nullptr;
    
    }
    
    model->setFarmTransmissionKernel( farmSpatialKernel );
    
    epiModel = model;
    
    readFile.close();
    
    return epiModel;
}

ExampleModel* SIRModelMaker::getModel() {
    return epiModel;
}


SingleStrainSEIRMaker::SingleStrainSEIRMaker( std::string path_to_params_ ): path_to_params( path_to_params_ ), epiModel( nullptr ) {};

SEIR* SingleStrainSEIRMaker::makeModel() {
    std::ifstream readFile( path_to_params );
    nlohmann::json params = nlohmann::json::parse( readFile );
    
    requireJsonKey( params, "nStrains" );
    requireJsonKey( params, "transmissibility" );
    requireJsonKey( params, "expectedIncubationTimes" );
    requireJsonKey( params, "expectedRecoveryTimes" );
    requireJsonKey( params, "transmissibilityEnvironment" );
    requireJsonKey( params, "environmentDecayProb" );
    requireJsonKey( params, "birdEnvironmentSheddingProb" );
    requireJsonKey( params, "timeStepSize" );
    
    uint nStrains = params["nStrains"];
    std::vector<double> trVec = params["transmissibility"];
    std::vector<double> expectedIncubTimeVec = params["expectedIncubationTimes"];
    std::vector<double> expectedRecTimeVec = params["expectedRecoveryTimes"];
    double trEnv = params["transmissibilityEnvironment"];
    double decayProbEnv = params["environmentDecayProb"];
    double sheddingProb = params["birdEnvironmentSheddingProb"];
    uint dt = params["timeStepSize"];

    assert( nStrains == 1 );
    assert( trVec.size() == 1 );
    assert( expectedIncubTimeVec.size() == 1 );
    assert( expectedRecTimeVec.size() == 1 );
    //checkEquality( expectedRecTimeVec.size(), 1 );

    SEIR* model = new SEIR( nStrains );

    model->setTransmissibility( trVec, dt );
    model->setIncubationProb( expectedIncubTimeVec, dt );
    model->setRecoveryProb( expectedRecTimeVec, dt );
    model->setEnvTransmissibility( trEnv );
    model->setEnvDecayProb( decayProbEnv );
    model->setBirdSheddingProb( sheddingProb );
    
    // read and set chicken-specific properties
    
    if ( params["innateImmunityParameters"].empty() ) {
        /*
        for ( const BirdType& bt: constants::allBirdTypes ) {
            model->setConstantInnateImmunity( bt, 1. );
        }*/
        model->setSimpleInnateImmunity();
    }
    else {
        
        model->setCustomInnateImmunity();
        requireJsonKey( params, "innateImmunityParameters" );

        for ( auto& innateParams: params["innateImmunityParameters"].items() ) {
            
            BirdType bt = constants::getBirdTypeFromString( innateParams.key() );
            auto& args = innateParams.value();
            
            requireJsonKey( args, "mode" );
            std::string mode = args["mode"].get<std::string>();
            
            if ( mode == std::string("constant") ) {
                
                requireJsonKey( args, "C0" );

                double C0 = args["C0"].get<double>();
                checkStrictlyPositive( C0 );
                model->setConstantInnateImmunity( bt, C0 );
            }
            else if ( mode == std::string("waning") ) {
                
                requireJsonKey( args, "C0" );
                requireJsonKey( args, "r" );

                double C0 = args["C0"].get<double>();
                double r  = args["r"].get<double>();
                
                checkStrictlyPositive( C0 );
                checkPositive( r );
                
                model->setWaningInnateImmunity( bt, C0, r );
            }
            else {
                throw Warning("innate immunity mode not recognized");
            }
        }
    }
    
    // setup setting-specific weights
    if ( !params["settingSpecificWeights"].empty() ) {
        
        for ( auto& weightParams: params["settingSpecificWeights"].items() ) {
            
            NodeType nt = constants::getNodeTypeFromString( weightParams.key() );
            auto& args = weightParams.value();
            std::string mode = args["mode"].get<std::string>();

            if ( mode == "constant" ) {
                double weight = args["weight"].get<double>();
                (model->getWeightManager()).setConstantReturn( nt, weight );
            }
            else {
                throw Warning("weight assignment mode not recognized");
            }

            
        }

        
    }
    
    // setup farm spatial transmission Kernel
    std::function< double ( const double& )> farmSpatialKernel;
    
    // set Farm spatial kernel
    {
        
        requireJsonKey( params, "FarmSpatialGridLambda" );
        requireJsonKey( params, "FarmSpatialKernelSizeExponentA" );
        requireJsonKey( params, "FarmSpatialKernelSizeExponentB" );
        requireJsonKey( params, "FarmSpatialKernelType" );
        
        // adaptive grid creation parameter
        double adaptiveGridLambda = params["FarmSpatialGridLambda"];
        model->setAdaptiveGridLambda( adaptiveGridLambda );
        
        // size exponents
        double expon_a = params["FarmSpatialKernelSizeExponentA"].get<double>();
        double expon_b = params["FarmSpatialKernelSizeExponentB"].get<double>();
        model->setFarmTransmissionExponents( expon_a, expon_b );

        double distMaxKernel = std::numeric_limits<double>::max();
        std::string kernelType = params["FarmSpatialKernelType"].get<std::string>();
        
        if ( kernelType == "ShiftedPareto" ) {
            
            requireJsonKey( params, "FarmSpatialKernelExponent" );
            requireJsonKey( params, "FarmSpatialKernelDistMin" );
            requireJsonKey( params, "FarmSpatialKernelCoefficient" );
            
            double exponent = params["FarmSpatialKernelExponent"].get<double>();
            double distMin  = params["FarmSpatialKernelDistMin"].get<double>();
            double coefficient = params["FarmSpatialKernelCoefficient"].get<double>();

            if ( coefficient > 0. )
                farmSpatialKernel = std::bind( shiftedParetoKernel, std::placeholders::_1, distMaxKernel, distMin, exponent, coefficient);
            else
                farmSpatialKernel = nullptr;
        }
        else if ( kernelType == "HillLikeParetoKernel" ) {
            
            requireJsonKey( params, "FarmSpatialKernelExponent" );
            requireJsonKey( params, "FarmSpatialKernelDistMin" );
            requireJsonKey( params, "FarmSpatialKernelCoefficient" );
            
            double exponent = params["FarmSpatialKernelExponent"].get<double>();
            double distMin  = params["FarmSpatialKernelDistMin"].get<double>();
            double coefficient = params["FarmSpatialKernelCoefficient"].get<double>();
            
            if ( coefficient > 0. )
                farmSpatialKernel = std::bind( hillLikeParetoKernel, std::placeholders::_1, distMaxKernel, distMin, exponent, coefficient );
            else
                farmSpatialKernel = nullptr;
        }
        else
            farmSpatialKernel = nullptr;
    
    }
    
    model->setFarmTransmissionKernel( farmSpatialKernel );
    
    
    epiModel = model;
    
    readFile.close();
    
    return epiModel;
}

SEIR* SingleStrainSEIRMaker::getModel() {
    return epiModel;
}


//===== MultiStrainSEIRMaker =====//

MultiStrainSEIRMaker::MultiStrainSEIRMaker( std::string path_to_params_ ): path_to_params( path_to_params_ ), epiModel( nullptr ) {};

MultiSEIR* MultiStrainSEIRMaker::makeModel() {
    
    std::ifstream readFile( path_to_params );
    nlohmann::json params = nlohmann::json::parse( readFile );
    
    requireJsonKey( params, "nStrains" );
    requireJsonKey( params, "maxStrains" );

    requireJsonKey( params, "transmissibility" );
    requireJsonKey( params, "expectedIncubationTimes" );
    requireJsonKey( params, "expectedRecoveryTimes" );
    requireJsonKey( params, "transmissibilityEnvironment" );
    requireJsonKey( params, "environmentDecayProb" );
    requireJsonKey( params, "birdEnvironmentSheddingProb" );
    requireJsonKey( params, "timeStepSize" );
    
    uint nStrains   = params["nStrains"];
    uint maxStrains = params["maxStrains"];
    std::vector<double> trVec = params["transmissibility"];
    std::vector<double> expectedIncubTimeVec = params["expectedIncubationTimes"];
    std::vector<double> expectedRecTimeVec = params["expectedRecoveryTimes"];
    double trEnv = params["transmissibilityEnvironment"];
    double decayProbEnv = params["environmentDecayProb"];
    double sheddingProb = params["birdEnvironmentSheddingProb"];
    uint dt = params["timeStepSize"];
    
    assert( maxStrains >= 1 );
    assert( maxStrains <= nStrains );
    assert( trVec.size() == nStrains );
    assert( expectedIncubTimeVec.size() == nStrains );
    assert( expectedRecTimeVec.size() == nStrains );

    MultiSEIR* model = new MultiSEIR( nStrains, maxStrains );

    model->setTransmissibility( trVec, dt );
    model->setIncubationProb( expectedIncubTimeVec, dt );
    model->setRecoveryProb( expectedRecTimeVec, dt );
    model->setEnvTransmissibility( trEnv );
    model->setEnvDecayProb( decayProbEnv );
    model->setBirdSheddingProb( sheddingProb );
    
    // read and set strain interactions
    if ( !params["strainInteractionsParameters"].empty() ) {
        
        auto& paramsImm = params["strainInteractionsParameters"] ;
        requireJsonKey( paramsImm, "mode" ) ;
        
        std::string mode = paramsImm["mode"].get<std::string>() ;

        
        if ( mode == "simpleCrossImmunityFixed" ) {
            /*
             Any prior exposure affects later exposures by fixed modifier sigma.
             sigma = 1      --> independent strains
             0 <= sigma < 1 --> competition
             sigma > 1      --> cooperation
             */
            
            requireJsonKey( paramsImm, "sigma" ) ;
            double sigma = paramsImm["sigma"].get<double>() ;
            model->setSimpleCrossReactiveSusceptibility( sigma );
            

        }
        else if ( mode == "simpleCrossImmunityMultiplicative" ) {
            
            /*
             Prior exposures affect later exposures multiplicatively,
             each by the same amount.
             */
            
            requireJsonKey( paramsImm, "sigma" ) ;
            double sigma = paramsImm["sigma"].get<double>() ;
            model->setMultiplicativeCrossReactiveSusceptibility( sigma );
            
        }
        else {
            
            throw Warning("cross reactivity mode not recognized") ;
        
        }
        
        
    }
    
    
    // read and set chicken-specific properties
    
    if ( params["innateImmunityParameters"].empty() ) {
        /*
        for ( const BirdType& bt: constants::allBirdTypes ) {
            model->setConstantInnateImmunity( bt, 1. );
        }*/
        model->setSimpleInnateImmunity();
    }
    else {
        
        model->setCustomInnateImmunity();
        requireJsonKey( params, "innateImmunityParameters" );

        for ( auto& innateParams: params["innateImmunityParameters"].items() ) {
            
            BirdType bt = constants::getBirdTypeFromString( innateParams.key() );
            auto& args = innateParams.value();
            
            requireJsonKey( args, "mode" );
            std::string mode = args["mode"].get<std::string>();
            
            if ( mode == std::string("constant") ) {
                
                requireJsonKey( args, "C0" );

                double C0 = args["C0"].get<double>();
                checkStrictlyPositive( C0 );
                model->setConstantInnateImmunity( bt, C0 );
            }
            else if ( mode == std::string("waning") ) {
                
                requireJsonKey( args, "C0" );
                requireJsonKey( args, "r" );

                double C0 = args["C0"].get<double>();
                double r  = args["r"].get<double>();
                
                checkStrictlyPositive( C0 );
                checkPositive( r );
                
                model->setWaningInnateImmunity( bt, C0, r );
            }
            else {
                throw Warning("innate immunity mode not recognized");
            }
        }
    }
    
    // setup setting-specific weights
    if ( !params["settingSpecificWeights"].empty() ) {
        
        for ( auto& weightParams: params["settingSpecificWeights"].items() ) {
            
            NodeType nt = constants::getNodeTypeFromString( weightParams.key() );
            auto& args = weightParams.value();
            std::string mode = args["mode"].get<std::string>();

            if ( mode == "constant" ) {
                double weight = args["weight"].get<double>();
                (model->getWeightManager()).setConstantReturn( nt, weight );
            }
            else {
                throw Warning("weight assignment mode not recognized");
            }

            
        }

        
    }
    
    // setup farm spatial transmission Kernel
    std::function< double ( const double& )> farmSpatialKernel;
    
    // set Farm spatial kernel
    {
        
        requireJsonKey( params, "FarmSpatialGridLambda" );
        requireJsonKey( params, "FarmSpatialKernelSizeExponentA" );
        requireJsonKey( params, "FarmSpatialKernelSizeExponentB" );
        requireJsonKey( params, "FarmSpatialKernelType" );
        
        // adaptive grid creation parameter
        double adaptiveGridLambda = params["FarmSpatialGridLambda"];
        model->setAdaptiveGridLambda( adaptiveGridLambda );
        
        // size exponents
        double expon_a = params["FarmSpatialKernelSizeExponentA"].get<double>();
        double expon_b = params["FarmSpatialKernelSizeExponentB"].get<double>();
        model->setFarmTransmissionExponents( expon_a, expon_b );

        double distMaxKernel = std::numeric_limits<double>::max();
        std::string kernelType = params["FarmSpatialKernelType"].get<std::string>();
        
        if ( kernelType == "ShiftedPareto" ) {
            
            requireJsonKey( params, "FarmSpatialKernelExponent" );
            requireJsonKey( params, "FarmSpatialKernelDistMin" );
            requireJsonKey( params, "FarmSpatialKernelCoefficient" );
            
            double exponent = params["FarmSpatialKernelExponent"].get<double>();
            double distMin  = params["FarmSpatialKernelDistMin"].get<double>();
            double coefficient = params["FarmSpatialKernelCoefficient"].get<double>();

            if ( coefficient > 0. )
                farmSpatialKernel = std::bind( shiftedParetoKernel, std::placeholders::_1, distMaxKernel, distMin, exponent, coefficient);
            else
                farmSpatialKernel = nullptr;
        }
        else if ( kernelType == "HillLikeParetoKernel" ) {
            
            requireJsonKey( params, "FarmSpatialKernelExponent" );
            requireJsonKey( params, "FarmSpatialKernelDistMin" );
            requireJsonKey( params, "FarmSpatialKernelCoefficient" );
            
            double exponent = params["FarmSpatialKernelExponent"].get<double>();
            double distMin  = params["FarmSpatialKernelDistMin"].get<double>();
            double coefficient = params["FarmSpatialKernelCoefficient"].get<double>();
            
            if ( coefficient > 0. )
                farmSpatialKernel = std::bind( hillLikeParetoKernel, std::placeholders::_1, distMaxKernel, distMin, exponent, coefficient );
            else
                farmSpatialKernel = nullptr;
        }
        else
            farmSpatialKernel = nullptr;
    
    }
    
    model->setFarmTransmissionKernel( farmSpatialKernel );
    
    
    epiModel = model;
    
    readFile.close();
    
    return epiModel;
}

MultiSEIR* MultiStrainSEIRMaker::getModel() {
    
    return epiModel ;

}


//===== MakeExampleModel =====//

ExampleModel* MakeExampleModel( std::string path_to_params ) {
    
    std::ifstream readFile( path_to_params );
    nlohmann::json params = nlohmann::json::parse( readFile );
    
    uint nStrains   = params["nStrains"];
    uint maxStrains = params["maxStrains"];

    std::vector<double> trVec = params["transmissibility"];
    std::vector<double> expectedIncubTimeVec = params["expectedIncubationTimes"];
    std::vector<double> expectedRecTimeVec = params["expectedRecoveryTimes"];
    double trEnv = params["transmissibilityEnvironment"];
    double decayProbEnv = params["environmentDecayProb"];
    double sheddingProb = params["birdEnvironmentSheddingProb"];
    uint dt = params["timeStepSize"];

    ExampleModel* model = new ExampleModel( nStrains, maxStrains );

    model->setTransmissibility( trVec, dt );
    model->setIncubationProb( expectedIncubTimeVec, dt );
    model->setRecoveryProb( expectedRecTimeVec, dt );
    model->setEnvTransmissibility( trEnv );
    model->setEnvDecayProb( decayProbEnv );
    model->setBirdSheddingProb( sheddingProb );
    
    readFile.close();
                
    return model;
};



