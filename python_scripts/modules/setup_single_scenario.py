import os
import json
import popgenerator as pg

def setup_single_scenario_Bangladesh( dx, scenarioID, path_folder_config, path_folder_output ):
    r"""
    
    Create files required to run a single scenario.
    
    Parameters
    ----------
    
        dx : dict
            Contains all parameters needed to build a synthetic PDN and produce
            required configuration files.
            
        scenarioID : int
            Integer label specific to single scenario.
            
        n_realizations : int
            Number of distinct stochastic realizations for each scenario.
            
        path_folder_config : string
            Path to folder where configuration files will be saved.
            
        path_folder_output : string
            Path to folder where simulation output will be stored.
    
    Return
    ------
    
        Nothing.
    
    """
    
    # create folders if necessary
    
    if not os.path.exists( path_folder_config ):
        os.makedirs( path_folder_config )
        
    if not os.path.exists( path_folder_output ):
        os.makedirs( path_folder_output )
    
    
    # number of stochastic realizations
    
    n_realizations = dx['general_settings']['n_realizations']
    
    if n_realizations < 1:
        raise ValueError('n_realizations must be a positive integer.')
        
    # store results
    
    modelSettings = {}
    
    # general settings
    
    for key, val in dx['general_settings'].items():
        if key in ['Tmax', 'bts', 'mmanBatchSizeScaling', 'mmanTotalSizeScaling', 
                   'vendorTotalSizeScaling', 'mmanMinSize', 'vendorMinSize']:
            modelSettings[key] = val
        
    # output settings
    modelSettings['outputParams'] = dx['outputParams'].copy()
    
    for analysis_type in ['birdTransactions','individualInfectionOutput',
                          'crossSectionalAnalyses','birdExitAnalyses', 'completedCropAnalyses']:
        if analysis_type == 'birdTransactions':
            modelSettings['outputParams'][analysis_type]['transactionOutputFolderPath'] = path_folder_output
        elif analysis_type == 'individualInfectionOutput':
            modelSettings['outputParams'][analysis_type]['pathToFile'] = path_folder_output
        else:
            for analysis in modelSettings['outputParams'][analysis_type].keys():
                modelSettings['outputParams'][analysis_type][analysis]['pathToFile'] = path_folder_output
    
    # middlemen settings
    
    modelSettings['mmen_params_path'] = dx['mmen_settings']['param_path']
    
    # vendor settings
    
    modelSettings['vendor_params_path'] = dx['vendor_settings']['param_path']
    
    # epi model settings
    
    epi_model_path = os.path.join( path_folder_config, 'epi_model_parameters_{}.json' ).format( scenarioID )
    epi_params = dx['epi_params']
    
    with open( epi_model_path, 'w') as writefile:
        json.dump( epi_params, writefile )
    
    modelSettings['epi_model_path'] = epi_model_path
    
    # write results
    
    #==== 0) Obtain fluxes-related parameters
    
    bts = dx['general_settings']['bts']
    n_areas = dx['popgenerator_settings']['flux_params']['n_areas']
    districts = dx['popgenerator_settings']['flux_params']['districts']
    paths_read_raw_flux = dx['popgenerator_settings']['flux_params']['paths_raw_flux']
    
    path_save_flux_params = os.path.join( '.', 'tmp_flux_{}.json'.format( scenarioID ) )
    
    pg.compute_area_market_fluxes_parameters_bangladesh( paths_read_raw_flux, 
                                                         bts, 
                                                         n_areas, 
                                                         districts, 
                                                         path_save_flux_params )
    #==== 1) Geography
    
    path_flux_params = path_save_flux_params
    path_geo  = dx['popgenerator_settings']['geography_params']['path_geo']
    path_geo_info = os.path.join( path_folder_config, 'area_list_path_{}.txt'.format( scenarioID ) )

    areas_included = pg.get_areas_included_from_flux( path_flux_params, bts )  
    gdf = pg.get_geography( path_geo, areas_included )

    pg.save_geography( gdf, path_geo_info )  # Save geography information
    
    modelSettings['area_list_path'] = path_geo_info
    
    for i in range( n_realizations ):
        
        modelSettings['scenarioID'] = '{}_{}'.format( scenarioID, i )
        modelSettings['realizationID'] = i

        #==== 2) Market info
        n_markets         = dx['popgenerator_settings']['market_params']['n_markets']
        n_layers          = dx['popgenerator_settings']['market_params']['n_layers']
        prop_birds_market = dx['popgenerator_settings']['market_params']['prop_birds_market']
        path_market_info  = os.path.join( path_folder_config, 'market_info_{}_{}.json'.format( scenarioID, i ) )
        
        pg.create_market_info( n_markets, 
                               bts, 
                               n_layers, 
                               path_market_info, 
                               prop_birds_market, 
                               None, None )

        modelSettings['market_list_path'] = path_market_info
        
        #==== 3) Inter-market mobility network

        mode                = dx['popgenerator_settings']['market_network_params']['mode'] 
        path_market_network = os.path.join( path_folder_config, 
                                           'market_network_{}_{}.txt'.format( scenarioID, i ) )
        
        if mode == 'generative_mixed_attachment_model':
        
            rho_market      = dx['popgenerator_settings']['market_network_params']['rho_market']
            p_random_market = dx['popgenerator_settings']['market_network_params']['p_random_market']
            self_w_market   = dx['popgenerator_settings']['market_network_params']['self_w_market']
            het_w_market    = dx['popgenerator_settings']['market_network_params']['het_w_market']


            with open( path_market_network, 'w' ) as outfile:
                for bt in bts:
                    inter_market_edges = pg.generate_mixed_attachment_network( n_markets, 
                                                                               rho_market, 
                                                                               p_random_market )
                    inter_market_edges_weight = pg.compute_weights_dirichlet( n_markets, 
                                                                              inter_market_edges, 
                                                                              self_w_market, 
                                                                              het_w_market )

                    for edge in inter_market_edges_weight: # expected format: i,j,w,bt
                        outfile.write( '{} {} {} {}\n'.format( edge[0], edge[1], edge[2], bt ) )
         
        modelSettings['market_network_path'] = path_market_network

        #==== 4) Area-market fluxes
        
        path_area_market_flux = os.path.join( path_folder_config, 
                                             'area_market_flux_{}_{}.txt'.format( scenarioID, i ) )
        
        pg.compute_area_market_flux_dirichlet( gdf, bts, n_markets, path_flux_params, path_area_market_flux )            

        modelSettings['area_market_fluxes_path'] = path_area_market_flux
        
        #==== 5) Create farms

        # create all-in-all-out farms
        
        n_farms                 = dx['popgenerator_settings']['farm_params']['n_farms']
        p_random_farm           = dx['popgenerator_settings']['farm_params']['p_random_farms']
        params_farm_size        = dx['popgenerator_settings']['farm_params']['params_farm_size']
        params_farm_minrollout  = dx['popgenerator_settings']['farm_params']['params_farm_minrollout']
        params_farm_refill_time = dx['popgenerator_settings']['farm_params']['params_farm_refill_time']
        
        farm_list_path = os.path.join( path_folder_config, 'farm_info_{}_{}.json'.format( scenarioID, i ) )

        pg.create_singlebt_farms( gdf, 
                                  n_farms, 
                                  bts, 
                                  path_flux_params, 
                                  p_random_farm, 
                                  params_farm_size, 
                                  params_farm_minrollout,
                                  params_farm_refill_time, 
                                  farm_list_path )
        
        modelSettings['farm_list_path'] = farm_list_path
    
        #==== 6) save model settings
        
        path_conf_file = os.path.join( path_folder_config, 'settings_{}_{}.json'.format( scenarioID, i ) )

        with open( path_conf_file, 'w' ) as writefile:
            json.dump( modelSettings, writefile )
    
    # remove temporary files
    os.remove( path_save_flux_params )