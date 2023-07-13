/*

 
 ███████╗██████╗ ██╗███╗   ██╗███████╗███████╗████████╗
 ██╔════╝██╔══██╗██║████╗  ██║██╔════╝██╔════╝╚══██╔══╝
 █████╗  ██████╔╝██║██╔██╗ ██║█████╗  ███████╗   ██║
 ██╔══╝  ██╔═══╝ ██║██║╚██╗██║██╔══╝  ╚════██║   ██║
 ███████╗██║     ██║██║ ╚████║███████╗███████║   ██║
 ╚══════╝╚═╝     ╚═╝╚═╝  ╚═══╝╚══════╝╚══════╝   ╚═╝
                                                       
 Author
 ------
  
 Francesco Pinotti :: email: francesco.pinotti92@gmail.com
 
 Description
 -----------
 
    @Brief :
        
        EPINEST is an individual-based model for epidemic transmission simulation in poultry
        production and distribution networks (PDN). It simulates poultry production, distribution
        and marketing in a synthetic supply chain system involving key actors of poultry trade.
        In addition, EPINEST allows to simulate the spread of one or more, possibly interacting,
        infectious agents.
 
    @Longer :
 
        The actual model is stochastic, which means that individual simulations may differ
        even when the same external parameters are set. Setting the same (externally provided)
        seed is the only way to obtain the same output.
 
        The present code allows to run multiple simulations over the SAME synthetic PDN;
        in other words, this code instantiates a single synthetic PDN whose structure that is
        adopted in all simulations.
        The line:

        mySimulator->endSimulation( args );
     
        ensures that the simulator resets the PDN before running the next simulation. This
        will not affect in any way the structural properties of the underlying PDN.
     
        The simulator runs in discrete time, with individual 1 hour-long time steps.
        Each time step involves at least two updates: a first one resolving poultry movements
        only, and a second one resolving epidemic dynamics.
 
 Parameters
 ----------
 
    1) PATH-TO-CONFIGURATION-FILE : specifies the location of a .json file containing all
       informations necessary to instantiate a PDN and run simulations.
 
    2) N_SIM : integer representing how many simulations need to be run.
 
    3) ID_SIM_START : integer representing the numerical ID of the first simulation to be
       run in this batch.
 
    4) SEED : integer to be used as a seed for pseudo-random-number generation.
 
 Output
 ------
 
    A single simulation may write multiple files depending on the requested analyses,
    as specified in the configuration file. To request a specific analysis, please modify the
    section 'outputParams' in the configuration file. According to the type of analysis,
    please modify the corresponding sub-section.

*/


#include <iostream>
#include <fstream>
#include <chrono>
#include "clock.hpp"
#include "random.hpp"
#include "types.hpp"
#include "compartmentalmodelbuilder.hpp"
#include "WorldSimulator.hpp"
#include "worldBuilder.hpp"
#include "epidemic_dynamics.hpp"
#include <stdlib.h>
#include <nlohmann/json.hpp>


// accepts a map to seed the desired strain in a given area
int main( int argc, const char* argv[] ) {
    
    // parse external arguments
    
    
    std::string model_settings_path =   std::string( argv[1] );
    uint simIDstart                 =   static_cast<uint>( strtoul( argv[2], nullptr, 10 ) );
    uint Nsim                       =   static_cast<uint>( strtoul( argv[3], nullptr, 10 ) );
    uint seed                       =   static_cast<uint>( strtoul( argv[4], nullptr, 10 ) );    

    // parse file with simulation settings
    
    std::ifstream readFileModelParams( model_settings_path ); // contains all model parameters
    nlohmann::json model_settings = nlohmann::json::parse( readFileModelParams );
    readFileModelParams.close();
    

    // parse file about epi-related stuff
    
    std::ifstream readEpiParams( model_settings["epi_model_path"].get<std::string>() );
    nlohmann::json epi_settings = nlohmann::json::parse( readEpiParams );
    readEpiParams.close();
    
    // get general simulator parameters
    
    std::string scenarioID  = model_settings["scenarioID"].get<std::string>(); // scenario identifier (e.g. "0" or "0_0")
    uint nDays              = model_settings["Tmax"].get<uint>();              // duration single simulation (in days)
    uint Tintro             = epi_settings["Tintro"].get<uint>();              // pathogen seeding time (in days)
    double sparkRate        = epi_settings["sparkRate"].get<double>();         // rate at which infection is seeded (after Tintro)
        
    std::map<uint,StrainIdType> area2strain = {}; // map assigning strain to area
    
    for ( auto& el : epi_settings["area2strain"].items() ) {
        
        uint area = el.value()["area"];
        StrainIdType strainId = el.value()["strain"];
        
        area2strain[area] = strainId;
        
    }
         
    // set up epidemic model
    
    std::string epi_params_path = model_settings["epi_model_path"].get<std::string>() ;
    MultiStrainSEIRMaker epiModelMaker( epi_params_path );
    MultiSEIR* epiModel = epiModelMaker.makeModel();
    
    // set up random number generator with seed
    
    Random randGen( seed );
    
    // use generator to instantiate a synthetic population
    
    WorldBuilder worldGenerator( model_settings_path, randGen ); // WorldBuilder is a helper class to build a synthetic population
    
    // set up simulator
    
    WorldSimulator* mySimulator = new WorldSimulator( worldGenerator, model_settings_path, epi_params_path, randGen, scenarioID, simIDstart );
    
        
    mySimulator->setEpiModel( epiModel ); // bind epidemic model
    
    // loop over simulations
    
    std::chrono::time_point<std::chrono::system_clock> start, end;
    
    for ( uint sim = 0; sim < Nsim; ++sim ) {
        
        // loop over days until simulation ends
        
        start = std::chrono::system_clock::now();
        
        for ( uint day = 0; day < nDays; day++ ) {
        
            // allow external infection starting from Tintro
            // seed specific strain in desired areas
            
            if ( day >= Tintro ) {
                //mySimulator->checkViralMixingFarms( area2strain ) ;
                mySimulator->sparkInfectionFarmsAreaSpecific( area2strain, sparkRate, randGen );
                //mySimulator->checkViralMixingFarms( area2strain ) ;
            }
           

            if ( day >= Tintro )
                epiModel->shuffleStrainList( randGen );
            
            // loop over hours
            for ( uint hour = 0; hour < HOURS_DAY; hour++ ) {

                mySimulator->doTimeStep( randGen, true ); // boolean flag is for epi dynamics
                
            }
        }
        
        // reset simulator at the end of a simulation
        bool resetWorld = ( sim == Nsim - 1 ) ? false : true ; // no need to reset everything after LAST simulation in this batch
        mySimulator->endSimulation( resetWorld );
        
        end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = end - start;
          
        std::cout << "Elapsed time: " << elapsed_seconds.count() << "s\n";

        
    }
    
    return 0;
    
}
