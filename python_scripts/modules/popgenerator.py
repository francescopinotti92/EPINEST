import os
import json
import warnings
import numpy as np
import pandas as pd
import geopandas as gpd
from shapely.geometry import Point
from scipy.optimize import brentq
from scipy.special import polygamma
from parsing_utils import parse_args

#=== Constants ===#

validBirdNames = ['BR', 'DE', 'SU']

#=== Geography ===#

def random_points_in_polygon(n, polygon):
    r"""
    
    Samples a fixed number of points within a polygon.
    
    Parameters
    ----------
    
    n: int
        number of points to sample
    
    polygon: shapely polygon
        polygon of interest
        
    Returns
    -------
    
    points: list of shapely Point objects
        list of random points
    
    """

    points = []
    min_x, min_y, max_x, max_y = polygon.bounds
    i= 0
    while i < n:
        point = Point(np.random.uniform(min_x, max_x), np.random.uniform(min_y, max_y))
        if polygon.contains(point):
            points.append(point)
            i += 1
    return points  # returns list of shapely point

def get_areas_included_from_flux( path_read_flux, bts ):
    r"""
    
    Gets regions to be included in the analysis based on regions with a non-zero
    outgoing bird flux.
    
    Parameters
    ----------
    
    path_read_flux : string
        Path to file with information.
        
    bts : list of strings
        Names of bird species considered.
    
    Returns
    -------
    
    areas_included : list of ints
        IDs of administrative units considered in this analysis.
    
    """
    
    #== Check parameters ==#
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    else:
        raise ValueError('bts is empty.')

    with open( path_read_flux, 'r' ) as readfile:
        farm_flows_info = json.load( readfile )

    # get ids of areas considered in this study
    areas_included = set( {} )
    for bt, params in farm_flows_info.items():
        if bt in bts:
            for areaId in params.keys():
                flux_out = params[areaId]['weight'] # check total flux from this region (might be zero)
                if ( flux_out > 0. ):
                    areas_included.add( int( areaId ) )
                
    areas_included = sorted( list( areas_included ) )
    
    if not areas_included:
        warnings.warn('No areas included')
    
    return areas_included

def get_geography( path_geo, areas_included, geometry = 'geometry', epsg=3106 ):
    r"""
    
    Gets regions to be included in the analysis based on regions with a non-zero
    outgoing bird flux.
    
    Parameters
    ----------
    
    path_geo : string
        Path to (geo)dataframe with all regions.
        
    areas_included : list of ints
        IDs of administrative units considered in this analysis.
        
    geometry : string
        (geo)Dataframe column holding geometric info.

    epsg : ???
        Spatial reference system. It should be a cartesian one (i.e. it should use
        cartesian coordinates and should be measured in m)
    
    Returns
    -------
    
    gdf : GeoDataFrame
        GeoDataFrame with all regions. Has the following columns:
        - id : region's ID
        - x : region's centroid x coordinate (measured in m)
        - y : region's centroid y coordinate (measured in m)
        - isIncluded : True if region is included in analysis, else set to False
        - 'geometry' : region's geometry 
    
    areas_included : list of ints
        IDs of administrative units considered in this analysis
        
    See also
    --------
    
    get_areas_included_from_flux :
        retrieves IDs of administrative units considered based on fluxes.
    
    
    """
    
    gdf = gpd.read_file( path_geo, geometry = geometry )
    
    if ( gdf.crs.srs != 'epsg:{}'.format( epsg ) ):
        gdf = gdf.to_crs( epsg = epsg ) # convert to desired system

    gdf['id'] = range( len( gdf ) )
    gdf['coords'] = gdf.centroid
    gdf['x'] = gdf['coords'].apply( lambda pt: pt.x )
    gdf['y'] = gdf['coords'].apply( lambda pt: pt.y )
    gdf['isIncluded'] = False
    gdf.loc[ gdf['id'].isin( areas_included ), 'isIncluded' ] = True
    
    return gdf[['id', 'isIncluded', 'x', 'y', geometry]].reset_index()
 
def save_geography( gdf, path_save ):
    r"""
    
    Save info about included regions to file .
    
    Parameters
    ----------
    
    gdf : GeoDataFrame/DataFrame
        Dataframe with info about regions. Must have the following columns:
        - 'id' : int
        - 'isIncluded' : bool
        - 'Lat' : float
        - 'Lon' : float
        
        
    path_save : string
        Path to output file 
        
    Returns
    -------
    
        Nothing.
    
    """
    
    for col in ['id', 'isIncluded', 'x', 'y']:
        if col not in gdf.columns:
            raise ValueError( "'{}' column not found in dataframe".format( col ) )
    
    folder = os.path.dirname( path_save )
    if not os.path.exists( folder ):
        os.makedirs( folder )
    
    # determine numerical ids of included areas
    areasID2index = { int(areaID) : int(i) for i, areaID in enumerate( gdf.loc[ gdf['isIncluded'] == True, 'id' ].values ) }
    
    with open( path_save, 'w') as outfile:
        for index, area in gdf.loc[ gdf['isIncluded'] == True ].iterrows():
            outfile.write( '{} {} {}\n'.format( areasID2index[ area['id'] ], area['x'], area['y'] ) )
    
    path_save_dict = '/'.join( path_save.split('/')[:-1] ) + '/areaID2index.json'
    with open( path_save_dict, 'w' ) as outfile:
        json.dump( areasID2index, outfile )

#=== Flux routines ===#

def Dirichlet_MLE(x, w):
    r"""
        
    Implicit MLE equation for estimating a Dirichlet distribution alpha parameter.
    This is meant as an input to a root-finding routine.
    
    Parameters
    ----------
    
    x : float
        free argument (it is a placeholder for alpha)
        
    w : numpy array
        Array with positive floats adding up to 1.
        
    
    Returns
    -------
    
    Nothing. 
    
    """
    n = len(w)
    S = np.mean( np.log(w) )
    res = S - polygamma( 0, x ) + polygamma( 0, x * n )
    return res

def read_fluxes( path_read, districts, n_areas = -1, n_markets = -1 ):
    r"""
    
    Reads flux data and selects a subset of upazilas and markets
    
    Parameters
    ----------
    
    path_read : str
        Path to file with fluxes

    districts : list
        Contains names of macro-areas to include in study (only DHAKA or CTG are valid entries).

    n_areas : int
        Number of upazilas to include. This function will select the upazilas with
        the largest outgoing flux. Non-positive values retain all areas.
    
    n_markets : int
        Number of markets to include. This function will select the markets with
        the largest ingoing flux. Non-positive values retain all markets.
    
    Returns
    -------
    
        pandas dataframe with fluxes
    
    """
    # get absolute fluxes

    df = pd.read_csv( path_read )


    # select relevant markets

    markets = list( df.columns )
    markets_ok = [ market for market in markets if ( market.split('_')[-1] in districts ) ]
    df = df[ markets_ok ]

    # select areas trading anything
    df = df.loc[ df.values.sum( axis = 1 ) > 0 ]

    # determine number of areas and markets to keep

    if ( n_areas <= 0 ):
        n_areas = len( df )
    else:
        n_areas = min( len( df ), n_areas )        

    flux_out = df.sum( axis = 1 )
    flux_in  = df.sum( axis = 0 )

    # select markets according to largest incoming flux
    markets_top = flux_in.sort_values( ascending = False ).index[:n_markets]
    df = df.loc[:,markets_top]

    # select areas according to largest outgoing flux
    areas_top = flux_out.sort_values( ascending = False ).index[:n_areas]
    df = df.loc[areas_top, :]

    # select areas with the largest fluxes
    #n_max_areas = min( n_areas, len( df ) )
    #flux_tot = df.values.sum( axis = 1 )
    #idxs = np.argsort( flux_tot )[::-1] # get area indexes in ascending order
    #upazilaIDs = sorted( df.index.values[idxs][:n_max_areas] )

    return df 

def compute_area_market_fluxes_parameters_raw_bangladesh( paths_read, bts, n_areas, n_markets, districts, path_save = None ):
    
    r"""
    
    Just reads raw fluxes.
    
    Parameters
    ----------
    
    paths_read : dict
        Paths to files with information about fluxes by bird type.
        Has structure:
        
        dict = { bt : path }
        
    bts : list of strings
        Names of bird species considered.
        
    n_areas : int
        Number of upazilas to include. This function will select the upazilas with
        the largest outgoing flux. Non-positive values retain all areas.
    
    n_markets : int
        Number of markets to include. This function will select the markets with
        the largest ingoing flux. Non-positive values retain all markets.
        
    districts : list
        Contains names of macro-areas to include in study (only DHAKA or CTG are valid entries).
        
    path_save:
        Path to file where results will be written to.
    
    Returns
    -------
    
        Nothing.
    
    """
    
    #== Check parameters ==#
    
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    
    if ( not isinstance( n_areas, int ) ):
        if ( isinstance( n_areas, float ) ):
            if ( n_areas.is_integer() ):
                n_areas = (int)(n_areas)
            else:
                raise ValueError('n_areas must be an integer scalar')
        else:
            raise ValueError('n_areas must be an integer scalar')  
    
    if districts:
        for el in districts:
            if el not in ['DHAKA','CTG']:
                raise ValueError('{} is not a valid district name. Please use DHAKA or CTG'.format(el))
    else:
        raise ValueError('districts is empty. Please use DHAKA or CTG')
        
    for bt in bts:
        if not paths_read[bt]:
            raise ValueError( 'No path found for {}'.format( bt ) )
        
    
    #========#
    
    flux_params = {}
    
    for bt in bts:
        
        flux_params[bt] = {}

        # get absolute fluxes


        # read raw flux
        df = read_fluxes( paths_read[bt], districts, n_areas, n_markets )
        flux_tot = df.values.sum( axis = 1 )
        idxs = np.argsort( flux_tot )[::-1] # get area indexes in ascending order
        upazilaIDs = sorted( df.index.values[idxs] )
        markets = df.columns

        # compute params for each upazila
        for upazilaID in upazilaIDs:

            row = df.loc[upazilaID].values.tolist()

            flux_params[bt][int(upazilaID)] = { 'values': row, 
                                                'weight': sum( row ) }
    
    # get ids of areas considered in this study
    areas_included = set( {} )
    for bt, params in flux_params.items():
        if bt in bts:
            for areaId in params.keys():
                flux_out = params[areaId]['weight'] # check total flux from this region (might be zero)
                if ( flux_out > 0. ):
                    areas_included.add( int( areaId ) )
                
    areas_included = sorted( list( areas_included ) )

    if path_save is None:
        return flux_params, areas_included
    else:   
        folder = os.path.dirname( path_save )
        if not os.path.exists( folder ):
            os.makedirs( folder )

        with open( path_save, 'w' ) as outfile:
            json.dump( flux_params, outfile )
        
        return flux_params, areas_included

def compute_area_market_fluxes_parameters_dirichlet_bangladesh( paths_read, bts, n_areas, districts, path_save = None ):
    
    r"""
    
    Computes upazila-specific parameters about upazila-market specific parameters by Fitting a Dirichlet distribution.
    
    Parameters
    ----------
    
    paths_read : dict
        Paths to files with information about fluxes by bird type.
        Has structure:
        
        dict = { bt : path }
        
    bts : list of strings
        Names of bird species considered.
        
    n_areas : int
        Number of upazilas to include. This function will select the upazilas with
        the largest outgoing flux.
        
    districts : list
        Contains names of macro-areas to include in study (only DHAKA or CTG are valid entries).
        
    path_save:
        Path to file where results will be written to.
    
    Returns
    -------
    
        Nothing.
    
    """
    
    #== Check parameters ==#
    
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    
    if ( n_areas < 0 ):
        raise ValueError('n_areas must be positive')
    if ( not isinstance( n_areas, int ) ):
        if ( isinstance( n_areas, float ) ):
            if ( n_areas.is_integer() ):
                n_areas = (int)(n_areas)
            else:
                raise ValueError('n_areas must be an integer scalar')
        else:
            raise ValueError('n_areas must be an integer scalar')  
    
    if districts:
        for el in districts:
            if el not in ['DHAKA','CTG']:
                raise ValueError('{} is not a valid district name. Please use DHAKA or CTG'.format(el))
    else:
        raise ValueError('districts is empty. Please use DHAKA or CTG')
        
    for bt in bts:
        if not paths_read[bt]:
            raise ValueError( 'No path found for {}'.format( bt ) )
        
    
    #========#
    
    flux_params = {}
    
    for bt in bts:
        
        flux_params[bt] = {}

        # get absolute fluxes

        '''
        df = pd.read_csv( paths_read[bt] )

        # select relevant markets

        markets = list( df.columns )
        markets_ok = [ market for market in markets if ( market.split('_')[-1] in districts ) ]
        df = df[ markets_ok ]

        # filter out regions with no connections to markets

        df = df.loc[ df.values.sum( axis = 1 ) > 0 ]

        # select areas with the largest fluxes
        
        n_max_areas = min( n_areas, len( df ) )

        flux_tot = df.values.sum( axis = 1 )
        idxs = np.argsort( flux_tot )[::-1] # get area indexes in ascending order
        
        upazilaIDs = sorted( df.index.values[idxs][:n_max_areas] )
        '''

        # read raw flux
        df = read_fluxes( paths_read[bt], districts, n_areas, -1 )
        flux_tot = df.values.sum( axis = 1 )
        idxs = np.argsort( flux_tot )[::-1] # get area indexes in ascending order
        upazilaIDs = sorted( df.index.values[idxs] )
        markets = df.columns

        # compute params for each upazila
        for upazilaID in upazilaIDs:

            row = df.loc[upazilaID].values

            deg = sum( row > 0 )
            fls = row
            fls = fls[fls > 0]

            fltot = fls.sum()
            rfls = fls / fltot # normalized rates

            alpha = brentq( Dirichlet_MLE, 0.001, 1000, args = rfls ) # get Dirichlet parameter

            flux_params[bt][int(upazilaID)] = { 'p_connectivity': deg / len( markets ), 
                                                   'weight': fltot, 
                                                   'alpha': alpha }
    
    # get ids of areas considered in this study
    areas_included = set( {} )
    for bt, params in flux_params.items():
        if bt in bts:
            for areaId in params.keys():
                flux_out = params[areaId]['weight'] # check total flux from this region (might be zero)
                if ( flux_out > 0. ):
                    areas_included.add( int( areaId ) )
                
    areas_included = sorted( list( areas_included ) )

    if path_save is None:
        return flux_params, areas_included
    else:   
        folder = os.path.dirname( path_save )
        if not os.path.exists( folder ):
            os.makedirs( folder )

        with open( path_save, 'w' ) as outfile:
            json.dump( flux_params, outfile )
        
        return flux_params, areas_included

def compute_area_market_flux_raw( gdf, bts, path_read_flux, path_save ):
    r"""
    
    Saves results to file.
    
    Parameters
    ----------
    
    gdf : GeoDataFrame/DataFrame
        Dataframe with info about regions. Must have the following columns:
        - 'id' : int
        - 'isIncluded' : bool
        - 'x' : float
        - 'y' : float
    
    bts : list of strings
        Names of bird species considered.
        
    path_read_flux : 
        Path to json file with info about individual regions.
        
        Resulting dictionary must have the following structure:
        
        dict = { bt : {
                areaID : {
                    'values' : list of floats,
                    'weight' : float,
                }
            }
        }
        
        where:
        - bt : a valid bird species
        - areaID : string containing a region's numerical ID; must be consistent with gdf's 'id' column
        - 'values' : a list of all weights
        - 'weight' : total out-weight, not used here. Must be > 0.
        - 'alpha' : heterogeneity in fluxes. Must be > 0.
        
    path_save : string
        Path to output file.
        
    Returns
    -------
    
        Nothing.
    
    """
    
    #== Check parameters ==#
    
    for col in ['id', 'isIncluded', 'x', 'y']:
        if col not in gdf.columns:
            raise ValueError( "'{}' column not found in dataframe".format( col ) )
            
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    else:
        raise ValueError('bts is empty.')
    
    #========#
    
    with open( path_read_flux, 'r' ) as readfile:
        farm_flows_info = json.load( readfile )
            
    gf = gdf.copy()       
    
    areas_included = gf.loc[gf['isIncluded'], 'id'].values
    
    areaID2Index = { ID: i for i,ID in enumerate( areas_included ) }
    
    areas_market_network = []
    for bt in bts:

        params = farm_flows_info[bt]
        
        for areaID in areas_included:
            
            areaIDstr = str(areaID)
            areaIndex = areaID2Index[areaID]
            
            if areaIDstr not in params:
                continue
                
            # get params to feed into Dirichlet distribution
            areaOutFlows = np.array( params[areaIDstr]['values'] ) / sum( params[areaIDstr]['values'] )
            
            areaOutFlowTot = params[areaIDstr]['weight']
          
            if ( areaOutFlowTot == 0. ):
                continue
          
            for market, value in enumerate( areaOutFlows ):
                if value > 0.:
                    areas_market_network.append( {'areaId': areaIndex, 
                                                  'marketId': market, 
                                                  'weight': float( value ), 
                                                  'bt': bt } )
    
    if not areas_market_network:
        warnings.warn('No area-market flux generated')
        
    # save results
    
    folder = os.path.dirname( path_save )
    if not os.path.exists( folder ):
        os.makedirs( folder )
    
    with open( os.path.join( path_save ), 'w' ) as outfile:
        for edge in areas_market_network:
            outfile.write( '{} {} {} {}\n'.format( edge['areaId'], edge['marketId'], edge['weight'], edge['bt'] ) )

def compute_area_market_flux_dirichlet( gdf, bts, n_markets, path_read_flux, path_save ):
    r"""
    
    Sample expected bird fluxes from each area to markets from a Dirichlet distribution.
    Saves results to file.
    
    Parameters
    ----------
    
    gdf : GeoDataFrame/DataFrame
        Dataframe with info about regions. Must have the following columns:
        - 'id' : int
        - 'isIncluded' : bool
        - 'x' : float
        - 'y' : float
    
    bts : list of strings
        Names of bird species considered.
        
    n_markets : int
        Number of markets.
        
    path_read_flux : 
        Path to json file with info about individual regions.
        
        Resulting dictionary must have the following structure:
        
        dict = { bt : {
                areaID : {
                    'p_connectivity' : float,
                    'weight' : float,
                    'alpha' : float
                }
            }
        }
        
        where:
        - bt : a valid bird species
        - areaID : string containing a region's numerical ID; must be consistent with gdf's 'id' column
        - 'p_connectivity' : probability of any connection between this area and markets. Must be in (0,1]
        - 'weight' : total out-weight, not used here. Must be > 0.
        - 'alpha' : heterogeneity in fluxes. Must be > 0.
        
    path_save : string
        Path to output file.
        
    Returns
    -------
    
        Nothing.
    
    """
    
    #== Check parameters ==#
    
    for col in ['id', 'isIncluded', 'x', 'y']:
        if col not in gdf.columns:
            raise ValueError( "'{}' column not found in dataframe".format( col ) )
            
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    else:
        raise ValueError('bts is empty.')

    if ( n_markets < 0 ):
        raise ValueError('n_markets must be positive')
    if ( not isinstance( n_markets, int ) ):
        if ( isinstance( n_markets, float ) ):
            if ( n_markets.is_integer() ):
                n_markets = (int)(n_markets)
            else:
                raise ValueError('n_markets must be an integer scalar')
        else:
            raise ValueError('n_markets must be an integer scalar')   
    
    #========#
    
    with open( path_read_flux, 'r' ) as readfile:
        farm_flows_info = json.load( readfile )
            
    gf = gdf.copy()       
    
    areas_included = gf.loc[gf['isIncluded'], 'id'].values
    
    areaID2Index = { ID: i for i,ID in enumerate( areas_included ) }
    
    areas_market_network = []
    for bt in bts:

        params = farm_flows_info[bt]
        
        for areaID in areas_included:
            
            areaIDstr = str(areaID)
            areaIndex = areaID2Index[areaID]
            
            if areaIDstr not in params:
                continue
                
            # get params to feed into Dirichlet distribution
            areaOutConnectP = params[areaIDstr]['p_connectivity']
            areaOutFlowTot = params[areaIDstr]['weight']
            areaOutFlowHet = params[areaIDstr]['alpha']    
            
            if ( areaOutConnectP <= 0 or areaOutConnectP > 1 ):
                raise ValueError('connectivity must be in (0,1]')
                
            if ( areaOutFlowTot == 0. ):
                continue
                    
            # sample number of markets (out-degree)         
            outDeg = 0
            while ( outDeg == 0 ):
                outDeg = np.random.binomial( n = n_markets, p = areaOutConnectP )            
            
            # sample markets
            nbrs = sorted( np.random.choice( a = range( n_markets ), size = outDeg, replace = False ) )

            # draw weights with given level of heterogeneity
            if ( areaOutFlowHet == np.inf ):
                w_tmp = np.array( [1./outDeg] * outDeg )
            else:
                w_tmp = np.random.dirichlet( [areaOutFlowHet] * outDeg )

            # add weighted edges
            for i, market in enumerate( nbrs ):
                areas_market_network.append( {'areaId': areaIndex, 
                                              'marketId': market, 
                                              'weight': w_tmp[i], 
                                              'bt': bt } )
    
    if not areas_market_network:
        warnings.warn('No area-market flux generated')
        
    # save results
    
    folder = os.path.dirname( path_save )
    if not os.path.exists( folder ):
        os.makedirs( folder )
    
    with open( os.path.join( path_save ), 'w' ) as outfile:
        for edge in areas_market_network:
            outfile.write( '{} {} {} {}\n'.format( edge['areaId'], edge['marketId'], edge['weight'], edge['bt'] ) )

#=== Farm generation routines ===#

def sample_singlebt_farm_locations( gdf, n_farms, bts, path_read_flux, p_random ):
    
    r"""
    
    Sample farm locations. Farm locations are uniform within sub-districts, but the density of farms
    per district is proportional to the outgoing bird flux
    Saves results to file.
    
    Parameters
    ----------
    
    gdf : GeoDataFrame/DataFrame
        Dataframe with info about regions. Must have the following columns:
        - 'id' : int
        - 'isIncluded' : bool
        - 'x' : float
        - 'y' : float
    
    n_farms : int or dict
        Number of farms. If int, use this for all bird types considered.
        If dict, must be of the type { bt : int }.
    
    bts : list of strings
        Names of bird species considered.
                
    path_read_flux : 
        Path to json file with info about individual regions.
        
        Resulting dictionary must have the following structure:
        
        dict = { bt : {
                areaID : {
                    'p_connectivity' : float,
                    'weight' : float,
                    'alpha' : float
                }
            }
        }
        
        where:
        - bt : a valid bird species
        - areaID : string containing a region's numerical ID; must be consistent with gdf's 'id' column
        - 'p_connectivity' : Not used here.
        - 'weight' : total out-weight. Must be > 0.
        - 'alpha' : Not used here.
        
    p_random : float
        Probability that a farm is assigned to a random region, irrespective of its flux.
        
    Returns
    -------
    
    res : dict
        Contains info about farm locations. It has the following structure:

        res = {
            bt : [(areaID, pos)]
        }

        where:
        - bt : a valid bird species name
        - areaID : ID of area where farm is located
        - pos : shapely Point object with farm coordinates

    """
    
    #== Check parameters ==#
    
    for col in ['id', 'isIncluded', 'x', 'y']:
        if col not in gdf.columns:
            raise ValueError( "'{}' column not found in dataframe".format( col ) )
            
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    else:
        raise ValueError('bts is empty.')

    if ( isinstance( n_farms, int ) ):
        get_n_farms = lambda x, n_farms=n_farms: n_farms
    elif ( isinstance( n_farms, float ) ):
        if ( n_farms.is_integer() ):
            n_farms = (int)(n_farms)
            get_n_farms = lambda x, n_farms=n_farms: n_farms
        else:
            raise ValueError('n_farms should be an integer scalar or a dict')
    elif isinstance( n_farms, dict ):
        for bt, val in n_farms:
            if bt not in validBirdNames:
                raise ValueError('{} is not a valid bird name.'.format( bt ))
            if not isinstance(val, int):
                raise ValueError('non-integer number of farms in n_farms ({})'.format(bt))
                
        get_n_farms = lambda x: n_farms[x]
    else:
         ValueError('n_farms should be an integer scalar or a dict')
                        
    if ( p_random < 0. or p_random > 1. ):
        raise ValueError('p_random must be in [0,1]')
    
    #===========#
    
    # read fluxes
    
    with open( path_read_flux, 'r' ) as readfile:
        farm_flows_info = json.load( readfile )

    # allocate farms for each bt
    
    res = {}
    
    for bt in bts:
        
        tmp = farm_flows_info[bt].copy()
        flux_dict = { int(areaID): p['weight'] for areaID, p in tmp.items() if p['weight'] > 0. } 
        areaIDs_bt = sorted( flux_dict.keys() )
        
        gf = gdf[gdf['id'].isin( areaIDs_bt )].copy()
        gf['weight'] = 0.
        gf['weight'] = gf['id'].map( flux_dict )
        gf['weight'] = gf['weight'] / gf['weight'].sum()
        gf['p_choose'] = p_random * ( 1. / len( gf ) ) + ( 1. - p_random ) * gf['weight']
       
        pf_ids = np.random.choice( a = list(gf['id'].values), 
                                   p = gf['p_choose'].values, 
                                   size = get_n_farms(bt), 
                                   replace = True )

        area_cts_pf = np.bincount( pf_ids, minlength = len( gf ) ) # farm counts per patch

        # sample coordinates

        pf_areaID_coords = []
        

        for i, ct in enumerate( area_cts_pf ):
            if ( ct > 0 ):
                points = random_points_in_polygon(ct, gf[gf['id'] == i].geometry.values[0])
                pf_areaID_coords.extend( list( zip( [i]*ct, points ) ) ) 
                
        res[bt] = pf_areaID_coords
        
    return res

def create_singlebt_farms(gdf, 
                          n_farms, 
                          bts, 
                          path_read_flux, 
                          p_random, 
                          params_size,
                          params_rollout_time,
                          params_refill_time,
                          path_save):
    r"""
    
    Create farm infos.
    
    Parameters
    ----------
    
    gdf : GeoDataFrame/DataFrame
        Dataframe with info about regions. Must have the following columns:
        - 'id' : int
        - 'isIncluded' : bool
        - 'x' : float
        - 'y' : float
    
    n_farms : int or dict
        Number of farms. If int, use this for all bird types considered.
        If dict, must be of the type { bt : int }.
    
    bts : list of strings
        Names of bird species considered.
                
    path_read_flux : 
        Path to json file with info about individual regions.
        
        Resulting dictionary must have the following structure:
        
        dict = { bt : {
                areaID : {
                    'p_connectivity' : float,
                    'weight' : float,
                    'alpha' : float
                }
            }
        }
        
        where:
        - bt : a valid bird species
        - areaID : string containing a region's numerical ID; must be consistent with gdf's 'id' column
        - 'p_connectivity' : Not used here.
        - 'weight' : total out-weight. Must be > 0.
        - 'alpha' : Not used here.
        
    p_random : float
        Probability that a farm is assigned to a random region, irrespective of its flux.
        
    params_size : dict
        Parameters to generate farm sizes. Has the following structure:
        
        {
            bt : {
                'mode' : string,
                '...'
            }
        }
        
        where:
        - bt : a valid bird species
        - 'mode' : generator type 
        - '...' : additional generator parameters
        
        currently implemented modes:
        - 'constant' : return a constant value (access with 'value')
        
    params_rollout_time : dict
        Parameters to generate minimum rollout times. Has the following structure:
        
        {
            bt : {
                'mode' : string,
                '...'
            }
        }
        
        where:
        - bt : a valid bird species
        - 'mode' : generator type 
        - '...' : additional generator parameters
        
        currently implemented modes:
        - 'constant' : return a constant value (access with 'value')
        
    params_refill_time : dict
        Parameters to generate refill probability. Has the following structure:
        
        {
            bt : {
                'mode' : string,
                '...'
            }
        }
        
        where:
        - bt : a valid bird species
        - 'mode_refill' : distribution type 
        - '...' : additional distribution parameters
        
        currently implemented modes:
        - 'Geometric' : time to next batch is geometric (access args with 'p_refill')
        - 'NegativeBinomial' : time to next batch is negative binomial (access args with 'p_refill' and 'n_refill')
    
    path_save : string
        Destination where output will be saved.
        
    Returns
    -------
    
        Nothing.
    
    """
    
    ##== Check parameters ==#
    
    for col in ['id', 'isIncluded', 'x', 'y']:
        if col not in gdf.columns:
            raise ValueError( "'{}' column not found in dataframe".format( col ) )
            
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    else:
        raise ValueError('bts is empty.')

    if ( isinstance( n_farms, int ) ):
        get_n_farms = lambda x, n_farms=n_farms: n_farms
    elif ( isinstance( n_farms, float ) ):
        if ( n_farms.is_integer() ):
            n_farms = (int)(n_farms)
            get_n_farms = lambda x, n_farms=n_farms: n_farms
        else:
            raise ValueError('n_farms should be an integer scalar or a dict')
    elif isinstance( n_farms, dict ):
        
        for bt, val in n_farms:
            if bt not in validBirdNames:
                raise ValueError('{} is not a valid bird name.'.format( bt ))
            if not isinstance(val, int):
                raise ValueError('non-integer number of farms in n_farms ({})'.format(bt))
                
        get_n_farms = lambda x: n_farms[x]
    else:
         ValueError('n_farms should be an integer scalar or a dict')
                        
    if ( p_random < 0. or p_random > 1. ):
        raise ValueError('p_random must be in [0,1]')
    
    # size parameters

    get_farm_size = {}

    pf_sizes = {}
    for bt in bts:
        pf_sizes[bt] = parse_args( params_size[bt], nrep = get_n_farms( bt ) )['Size']
    
    # rollout time parameters
   
    get_farm_min_rollout_time = {}
    
    pf_rollout_times = {}
    for bt in bts:
        pf_rollout_times[bt] = parse_args( params_rollout_time[bt], nrep = get_n_farms( bt ) )['rollout_time']
            
    # refill probability parameters
    
    pf_refill_params = {}
    for bt in bts:
        pf_refill_params[bt] = {}
        tmp = parse_args( params_refill_time[bt], nrep = get_n_farms( bt ) )
        for field, value in tmp.items():
            if field == 'mode':
                pf_refill_params[bt]['mode_refill'] = [ value ] * get_n_farms( bt )
            else:
                pf_refill_params[bt][field] = value
      
    # farm locations and areas
    
    # retrieve info about included areas
    areas_included = get_areas_included_from_flux( path_read_flux, bts )
    areaID2index = { ID: i for i, ID in enumerate( areas_included ) }
        
    # compute positions
    # !!! check this function
    pf_coords = sample_singlebt_farm_locations( gdf, n_farms, bts, path_read_flux, p_random )

    # create and write json file with specifications

    farms_info = {}
    farm_ctr = 0
    for bt in bts:
        for i in range( get_n_farms( bt ) ):
            farms_info[farm_ctr] = {'id': farm_ctr,
                             'Type': 'CommercialFarm',
                             'Size': int(pf_sizes[bt][i]),
                             'x': float( pf_coords[bt][i][1].x ),
                             'y': float( pf_coords[bt][i][1].y ),
                             'BirdType': [bt],
                             'rollout_time': int( pf_rollout_times[bt][i] ),
                             'catchment_area': int( areaID2index[ pf_coords[bt][i][0] ] ) }
            
            # set refill parameters for i-th farm
            for field, data in pf_refill_params[bt].items():
                val = data[i]
                if type( val ) == np.int_:
                    val = int( val )
                elif type( val ) == np.float_:
                    val = float( val )
                farms_info[farm_ctr][field] = val
            
            farm_ctr += 1
    
    # save farms
    folder = os.path.dirname( path_save )
    if not os.path.exists( folder ):
        os.makedirs( folder )
        
    with open( os.path.join( path_save ), 'w') as outfile:
        json.dump( farms_info, outfile )

#=== Market info routines ===#

def create_market_info( n, bts, n_layers, path_save, prop_birds_layer, xs = None, ys = None ):
    r"""
    
    Formats info about markets. Saves info automatically to file.
    
    Parameters
    ----------
    
    n : int
        Number of markets.
    
    bts : list of strings
        Names of bird species considered.
    
    n_layers : int
        Number of market layers.
        
    path_save : string
        Path to output file.
        
    prop_birds_layer : dict
        Proportion of birds flowing to either wholesalers or retailers by bird type and layer.
        Access with prop_birds_layer[bt][vendor_type][layer], where: 'bt' is a value from bts,
        'vendor_type' is 'W' (Wholesaler) or 'R' (Retailer) and layer = 0,...,n_layers - 1.
        
    xs: None, float or list of floats
        Market x coordinates (measured in m).
        
    ys: None, float or list of floats
        Market y coordinates (measured in m).
        
    Returns
    -------
    
        Nothing.
    
    """
    
    #== Check parameters ==#
    
    if ( n < 0 ):
        raise ValueError('n must be positive.')
    if ( not isinstance( n, int ) ):
        if ( isinstance( n, float ) ):
            if ( n.is_integer() ):
                n = (int)(n)
            else:
                raise ValueError('n must be an integer scalar.')
        else:
            raise ValueError('n must be an integer scalar.')
    
    if ( bts ):
        for bt in bts:
            if bt not in validBirdNames:
                raise ValueError( '{} is not a valid bird name.'.format( bt ) )
    else:
        raise ValueError('bts is empty.')
        
    if ( n_layers < 0 ):
        raise ValueError('n_layers must be positive.')
    if ( not isinstance( n_layers, int ) ):
        if ( isinstance( n_layers, float ) ):
            if ( n_layers.is_integer() ):
                n_layers = (int)(n_layers)
            else:
                raise ValueError('n_layers must be an integer scalar.')
        else:
            raise ValueError('n_layers must be an integer scalar.')
    
    # check layer-specific fluxes
    for bt in bts:
        if bt in prop_birds_layer:
            tmp = prop_birds_layer[bt]
            
            if 'W' in tmp:
                tmpW = tmp['W']
                if not isinstance( tmpW, list ):
                    raise ValueError('Wholesaler parameters must be stored in a list ({})'.format( bt ))
                if ( len( tmpW ) != n_layers ):
                    raise ValueError('Wholesaler parameters list must contain {} values ({})'.format( n_layers, bt ))
            else:
                raise ValueError('Wholesaler parameters not specified for {}'.format( bt ))
            
            if 'R' in tmp:
                tmpR = tmp['R']
                if not isinstance( tmpR, list ):
                    raise ValueError('Retailer parameters must be stored in a list ({})'.format( bt ))
                if ( len( tmpR ) != n_layers ):
                    raise ValueError('Retailer parameters list must contain {} values ({})'.format( n_layers, bt ))
            else:
                raise ValueError('Retailer parameters not specified for {}'.format( bt ))
            
            if ( len( tmpR ) != len( tmpW ) ):
                raise ValueError('Number of retailer and wholesaler parameters do not match ({})'.format( bt ))
           
            if ( tmpR[0] + tmpW[0] != 1 ):
                raise ValueError('Wholesaler and retailer parameters must add up to 1 in first layer ({})'.format( bt ))

            if ( tmpR[n_layers - 1] != 0 or tmpW[n_layers - 1] != 0 ):
                raise ValueError('Last layer parameters must be equal to 0 ({})'.format( bt ))
                
        else:
            raise ValueError( '{} not in prop_birds_layer'.format( bt ) )
            
    
    if ( ( xs == None ) or ( ys == None ) ):
        xs = [ 0. ] * n
        ys = [ 0. ] * n
    
    elif ( isinstance( xs, float ), isinstance( ys, float ) ):
        xs = [ xs ] * n
        ys = [ ys ] * n
    
    elif ( isinstance( xs, list ), isinstance( ys, list ) ):
        if ( len( xs ) != n ):
            raise ValueError('xs must contain exactly n elements, contains {}'.format( len( xs ) ) )
        if ( len( ys ) != n ):
            raise ValueError('ys must contain exactly n elements, contains {}'.format( len( ys ) ) )
    
    else:
        raise ValueError('Invalid argument for xs and/or ys')
    
    #========#
    
    market_info = {}
    
    folder = os.path.dirname( path_save )
    if not os.path.exists( folder ):
        os.makedirs( folder )
    
    for i in range( n ):
        market_info[i] = { 'id': i,
                           'x': xs[i],  # place nMarkets into the most populated area
                           'y': ys[i],
                           'Type': 'Market',
                           'TradedBirds': { 
                               bt: { 'propBirdsSold2WS': prop_birds_layer[bt]['W'],
                                     'propBirdsSold2R' : prop_birds_layer[bt]['R'] } for bt in bts } }

    with open( path_save, 'w' ) as outfile:
        json.dump( market_info, outfile )

#=== Market network routines ===#

def generate_mixed_attachment_network(N, rho, p_random):
    r"""
    
    Generates a DAG with tunable density and hierarchical structure.
    
    Parameters
    ----------
    N : int
        Number of nodes.
        
    rho : float
        Network edge density.
        
    p_random : 
        probability that an edge is tossed at random.
        
    Returns
    -------
    
    edges : list of 2-tuples (int,int)
        list of network edges.
        
    """
    
    #== check parameters ==#
    if ( N < 0 ):
        raise ValueError('N must be positive')
    if ( not isinstance( N, int ) ):
        if ( isinstance( N, float ) ):
            if ( N.is_integer() ):
                N = (int)(N)
            else:
                raise ValueError('N must be an integer scalar')
        else:
            raise ValueError('N must be an integer scalar')
        
    if ( rho < 0. or rho > 1. ):
        raise ValueError('rho must be in [0,1]')
    if ( p_random < 0. or p_random > 1. ):
        raise ValueError('p_random must be in [0,1]')

    node_urn = [0]
    edges = []
        
    if ( rho == 0. ):
        return edges
    
    for j in range( 1, N ):
        K = np.random.binomial(n = j, p = rho )
        tmp = []
        
        k = 0    
        while( k < K ): 
            
            if ( np.random.uniform() <= p_random ):
                i = np.random.randint( j )
            else:
                i = np.random.choice( a = node_urn )
            
            if ( i not in tmp ):
                edges.append( ( i, j ) )
                node_urn.append( i )
                tmp.append( i )
                k += 1
        
    return edges

def compute_weights_dirichlet( N, edge_list, self_w, het_w = np.inf ):
    r"""
    
    Given a list of edges, assigns a weight to each edge. Also adds self-loops.

    If a node's out-degree is 0, its self-weight is set to 1, else it is set to self_w.
    
    Weights relative to outgoing edges are sampled from a Dirichlet distribution: if node i has K
    out-neighbors {j1, j2, ..., jK}, then w_j = self_w * a_j, where {a_j} ~ Dirichlet[ [het_w]*K ].

    
    Parameters
    ----------
    
    N : int
        Number of network nodes.
        
    edge_list : list of 2-tuples
        List of network edges. These are interpreted as directed edges.
        
    self_w : float
        Strength of self-links.
        
    het_w : float
        Heterogeneity of network edges.
        
    Returns
    -------
    
    w_edge_list : list of 3-tuples (int,int,float) 
        List of weighted edges.
    
    """
    
    #== check parameters ==#
    if ( N < 0 ):
        raise ValueError('N must be positive')
    if ( not isinstance( N, int ) ):
        if ( isinstance( N, float ) ):
            if ( N.is_integer() ):
                N = (int)(N)
            else:
                raise ValueError('N must be an integer scalar')
        else:
            raise ValueError('N must be an integer scalar')
        
    if ( self_w < 0. or self_w > 1. ):
        raise ValueError('self_w must be in [0,1]')
    if ( het_w < 0. ):
        raise ValueError('het_w must be positive')
    
    adj_list = [ [] for i in range( N ) ]
    for edge in edge_list:
        i,j = edge
        adj_list[ i ].append( j )
    
    w_edge_list = []
        
    if het_w == np.inf:
        getw = lambda n: np.array( [ 1./n ] * n ) # homogeneous weights
    else:
        getw = lambda n: np.random.dirichlet( [ het_w ] * n ) # heterogeneous weights
    
    for i in range( N ):
        if adj_list[i]:
            w_edge_list.append( ( i, i, self_w ) ) # add self-loop
            if ( self_w  < 1. ):
                ws = ( 1 - self_w ) * getw( len( adj_list[i] ) )
                for iw, w in enumerate(ws):
                    w_edge_list.append( ( i, adj_list[i][iw], w ) )
        else:
            w_edge_list.append( ( i, i, 1. ) ) # add self-loop (with unit weight)
     
    return w_edge_list