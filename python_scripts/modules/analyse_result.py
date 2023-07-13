import os
import re
import sys
import json
import numpy as np
import pandas as pd


def has_column( df, name ):
    r"""

    Returns True if 'df' contains column 'name' and False otherwise

    """

    if ( name in df.columns ):
        return True
    else:
        return False
        
def get_counts( df, name = None, normalize = True ):

    if isinstance( df, pd.Series ):
        return df.value_counts( normalize = normalize ).sort_index( ascending = True )
    elif isinstance( df, pd.DataFrame ):
        if name is None:
            raise ValueError( "Please specify a column." )
        if not has_column( df, name ):
            raise ValueError( "Dataframe has no column '{}'.".format( name ) )
        return df[name].value_counts( normalize = normalize ).sort_index( ascending = True )
    else:
        raise ValueError( "df is not a pandas DataFrame or a Series." )

def rebin_histo(x, bins):
    '''
    Aggregates a probability distribution over integers into
    a new histogram according to a custom binning
    
    Input(s)
    ------
    - x : Probability distribution over integers
    - bins : list of (left) bin edges to aggregate values from x
             last bin corresponds to all entries in x at positions
             bins[-1] or larger
    
    Output
    ------
    - res : new probability distribution over ranges induced by bins
    
    '''
    N = len(x)
    res = np.zeros( len( bins ) )
    for i in range( len( bins ) - 1 ):
        res[i] = sum( x[bins[i]:bins[i+1]] )
    res[-1] = sum( x[bins[-1]:] )
    return res

def rebin_histo_sparse(x, vals, bins):
    '''
    Aggregates a probability distribution over integers into
    a new histogram according to a custom binning
    
    Input(s)
    ------
    - x : Probability distribution over integers
    - vals : integers over which x is defined
    - bins : list of (left) bin edges to aggregate values from x
             last bin corresponds to all entries in x at positions
             bins[-1] or larger
    
    Output
    ------
    - res : new probability distribution over ranges induced by bins
    
    '''
    N = max( vals ) + 1
    x1 = np.zeros( N )
    x1[ vals ] = x # create 'holes' with 0s
    
   
    res = np.zeros( len( bins ) )
   
    for i in range( len( bins ) - 1 ):
        res[i] = sum( x1[bins[i]:bins[i+1]] )
    res[-1] = sum( x1[bins[-1]:] )
    return res


def read_transactions( filename, name = "transact"):
    r"""

    Reads and parses a list of transactions

    Parameters
    ----------

    filename : str
        Path to file contanining transactions.

    name : str, default = "transact"
        Name assigned to transaction column
    
    Returns
    -------

    df : pandas.DataFrame
        Dataframe with individual transaction properties:
            - name    : original transaction string
            - srcType : source actor type
            - dstType : receiver actor type
            - srcId   : numerical id of source actor
            - dstId   : numerical id of receiver actor
            - nb      : number of birds involved in transaction
            - t       : timestep of transaction
            - day     : day of transaction
    """

    df = pd.read_csv( filename, names = [ name ] )  # read file
    splitSeq = df[name].str.split( '-' )      # parse individual transactions
    df['srcType'] = splitSeq.str[0]
    df['dstType'] = splitSeq.str[2]
    df['srcId'] = splitSeq.str[1].astype( int )
    df['dstId'] = splitSeq.str[3].astype (int )
    df['nb'] = splitSeq.str[-1].astype( int )
    df['t'] = splitSeq.str[-2].astype( int )
    df['day'] = df['t'] // 24

    return df


def get_working_day( df, h = 7 ):
    r"""
    
    Computes working day for each transaction.
    A 'working day' might not coincide with an actual 'day'.

    Parameters
    ----------

    df : pandas.DataFrame
        Dataframe with all transactions. Must have a column 't' with corresponding timestep.

    h : int, default = 7
        Hour of the day marking the beginning of a new working day. Must be between 0 and 23 (included).

    Returns
    -------
    
    df : pandas.DataFrame
        Same initial dataframe, with an additional column 'wday'

    """

    if ( 't' not in df.columns ):
        raise ValueError( "column 't' not found." )

    if ( 'day' not in df.columns ):
        df['day'] = df['t'] // 24
        
    df['wday'] = df['day'] 
    df.loc[df['t'] % 24 < h, 'wday'] -= 1

    return df


def read_rollout_duration( filename, sampling_hour ):

    df = pd.read_csv( filename, names = ['t', 'id', 'dt'] ) # read file with rollout duration

    df['srcId'] = df['id'].str[1:].astype(int) # ??? improve
    df['day'] = ( df['t'] - sampling_hour ) // 24

    df.drop( columns = ['id'], axis = 1, inplace = True ) 

    return df

'''
def get_batch_info_farms( df, roll_info, day_min ):

    df1 = df[df['srcType'] == 'PF']

    # group all transactions made by any pair of actors over a single day ( i.e. define an aggregated transaction )
    btc = df1.groupby(['srcId', 'dstId', 'wday'], as_index = False)['nb'].sum().sort_values( by = ['srcId', 'wday'])
    btc['batchId'] = -1

    # for each farm, assign a unique BATCH identifier to each aggregated transaction
    # n.b. multiple aggregated transactions might share the same BATCH identifier

    identifier = 0
    for farmId in roll_info['srcId'].unique():
        single_farm_rolls = roll_info.loc[ roll_info['srcId'] == farmId, [ 'day', 'dt' ] ].values
        for i, infos in enumerate( single_farm_rolls ):
            day_end, dt = infos
            day_start = day_end - dt
            if ( day_start > day_min ):
                btc.loc[ ( btc['srcId'] == farmId ) & ( btc['wday'] <= day_end ) & ( btc['wday'] >= day_start ), 'batchId' ] = identifier
                btc.loc[ ( btc['srcId'] == farmId ) & ( btc['wday'] <= day_end ) & ( btc['wday'] >= day_start ), 'day_start' ] = day_start
                identifier += 1

    return btc
'''

'''
def get_proportion_unsold_birds_by_day( btc, farm_info ):

    if isinstance( farm_info, str ):
        path = farm_info

        if not os.path.exists( path ):
            raise ValueError( "'farm_info' does not represent an existing file." )

        with open( path, 'r' ) as readr:
            farm_info = json.load( readr )

    elif isinstance( farm_info, dict ):
        pass
    else:
        raise ValueError( "'farm_info' is neither a 'dict' nor a 'str'." ) 


    farmSizes = { farmId: farm_info[str( farmId )]['Size'] for farmId in btc['srcId'].unique() }
    batch2farm = btc.loc[btc['batchId'] != -1].groupby('batchId')['srcId'].first().to_dict()
    batch2size = { batch: farmSizes[farm] for batch, farm in batch2farm.items() }

    btc1 = btc[ btc['batchId'] != -1 ].copy()
    btc1['dt'] = ( btc1['wday'] - btc1['day_start'] ).astype(int) + 1 # time since beginning of rollout
    btc1 = btc1.groupby( ['batchId', 'dt'] )['nb'].sum().groupby( level = 0 ).cumsum().reset_index() # sum birds by day, take cumulative sum
    btc1['batchSize'] = btc1['batchId'].map( batch2size )
    btc1['rb'] = btc1['batchSize'] - btc1['nb'] # compute remaining birds

    dt_max = btc1['dt'].max()
    ICDF = np.zeros( dt_max ) # probability being unsold after d days since rollout
    for batch in btc1['batchId'].unique():
        infos = btc1[ btc1['batchId'] == batch ].copy()
        infos = infos[['dt','rb']]
        infos.set_index('dt', inplace = True)
        infos = infos.reindex( range( dt_max ) )['rb'].fillna( method = 'ffill' ).fillna( batch2size[batch] )
        ICDF += infos.values

    return ICDF
'''

def get_batch_info_farms( df, roll_info, tlims, dt_raise ):
    r"""

    Merges info about individual transactions and batches (crops)
    Assigns first day to each transaction.

    Parameters
    ----------
    
    Returns
    -------
    
    """
    
    # process rollout information from crops
    roll_info['srcId'] = roll_info['farmStrID'].str[1:].astype(int) # farm index
    
    btc = df[df['srcType'] == 'PF'].copy()

    # group all transactions made by any pair of actors over a single day ( i.e. define an aggregated transaction )
    btc['batchId'] = -1

    # for each farm, assign a unique BATCH identifier to each aggregated transaction
    # n.b. multiple aggregated transactions might share the same BATCH identifier

    identifier = 0
    for farmId in roll_info['srcId'].unique():
        
        # get start and stop for every 
        single_farm_rolls = roll_info.loc[ roll_info['srcId'] == farmId, [ 't0', 't1', 'size' ] ].values
        
        for i, infos in enumerate( single_farm_rolls ):
            t0, t1, size = infos # t0: t at which batch was created, t1: time at which batch is emptied
            t0 += dt_raise  # add raising time
            if ( ( t0 >= tlims[0] ) and ( t0 < tlims[1] ) ):
                cond = ( btc['srcId'] == farmId ) & ( btc['t'] <= t1 ) & ( btc['t'] >= t0 )
                btc.loc[ cond, 'batchId' ] = identifier
                btc.loc[ cond, 't_start' ] = t0
                btc.loc[ cond, 'size_btc'] = size
                identifier += 1
                
    #btc['t_start'] = btc['t_start'].astype( int )

    return btc

def get_proportion_unsold_birds_by_dt( btc ):
    
    r"""

    Computes survival distribution in farms, name

    Parameters
    ----------
    
    Returns
    -------
    
    """
    
    btc1 = btc[ btc['batchId'] != -1 ].copy() # eliminate transactions without batchId 
    btc1['size_btc'] = btc1['size_btc'].astype( int ) # make sure batch size is int
    btc1['dt'] = ( btc1['t'] - btc1['t_start'] ).astype( int ) # time since beginning of rollout
    dt_max = btc1['dt'].max()
    
    # sum birds by dt, then sum birds sold up to dt in current batch
    btc1 = btc1.groupby( ['batchId', 'dt'] ).agg( { 'nb': np.sum, 
                                              'size_btc': lambda x: x.iloc[0] } ) 
    btc1['nb'] = btc1.groupby( level = 0 )['nb'].cumsum()
    
    # compute remaining birds after dt in current batch
    btc1['rb'] = btc1['size_btc'] - btc1['nb']
    btc1 = btc1.reset_index()

    ICDF = np.zeros( dt_max ) # probability being unsold after d days since rollout
    for batch in btc1['batchId'].unique():
        infos = btc1.loc[ btc1['batchId'] == batch ].copy()
        size_btc = infos['size_btc'].values[0]
        infos = infos[['dt','rb']]
        infos.set_index('dt', inplace = True)
        infos = infos.reindex( range( dt_max ) )['rb'].fillna( method = 'ffill' ).fillna( size_btc )
        ICDF += infos.values

    return np.arange( 1, dt_max + 1 ), ICDF


def eval_mman_marketdeg_distr( values, mman_info, breed ):

    # read mman info

    if isinstance( mman_info, str ):
        path = mman_info

        if not os.path.exists( path ):
            raise ValueError( "'mman_info' does not represent an existing file." )

        with open( path, 'r' ) as readr:
            mman_info = json.load( readr )

    elif isinstance( mman_info, dict ):
        pass
    else:
        raise ValueError( "'mman_info' is neither a 'dict' nor a 'str'." )

    distr_type = mman_info[breed]['outDegreeDistr']
    if ( distr_type == 'Geometric' ):
        p = mman_info[breed]['p']
        return [ np.power( 1 - p, val - 1 ) * p for val in values ]
     

def get_unique_farm_mmen_visits_daily( df ):
    r"""

    Measures the number of distinct middlemen visiting a farm daily and
    the number of distinct farms visited by a middleman daily

    """

    if not has_column( df, 'wday' ):
        raise ValueError( "Dataframe has no column 'wday'." )

    tmp = df[(df['srcType'] == 'PF') & (df['dstType'] == 'MM')].copy()

    nmmen_visited = tmp.groupby(['srcId', 'wday'])['dstId'].nunique().value_counts(normalize = True).sort_index()
    nfarms_visited = tmp.groupby(['dstId', 'wday'])['srcId'].nunique().value_counts(normalize = True).sort_index()

    return nmmen_visited, nfarms_visited

def get_mmen_vendor_transaction( df, name = 'transact' ):

    if not has_column( df, name ):
        raise ValueError( "Dataframe has no column '{}'.".format( name ) )

    if not has_column( df, 'wday' ):
        raise ValueError( "Dataframe has no column 'wday'." )

    tmp = df[(df['srcType'] == 'MM') & (df['dstType'] == 'V')].copy() # select MM->V transactions
    tmp['Market'] = tmp['transact'].str.split('-').str[5] # parse market information

    return tmp

def get_mmen_nareas_visited_daily( df, farm_info ):

    # check dataframe

    if not has_column( df, 'wday' ):
        raise ValueError( "Dataframe has no column 'wday'." )

    # read farm info

    if isinstance( farm_info, str ):
        path = farm_info

        if not os.path.exists( path ):
            raise ValueError( "'farm_info' does not represent an existing file." )

        with open( path, 'r' ) as readr:
            farm_info = json.load( readr )

    elif isinstance( farm_info, dict ):
        pass
    else:
        raise ValueError( "'farm_info' is neither a 'dict' nor a 'str'." ) 

    # get area-farm info 
    farmId2Area = {}
    for key, data in farm_info.items():
        farmId2Area[ data['id'] ] = data['catchment_area']

     

    tmp = df[(df['srcType'] == 'PF') & (df['dstType'] == 'MM')].copy()
    tmp['area'] = tmp['srcId'].map(farmId2Area)

    nareas = tmp.groupby(['dstId', 'wday'], as_index = False)['area'].nunique()
    nareas_histo = get_counts( nareas, 'area' )

    return nareas_histo

def compute_area_market_flux( df, farm_info, nAreas, name = 'transact' ):

    # read farm info
    if isinstance( farm_info, str ):
        path = farm_info

        if not os.path.exists( path ):
            raise ValueError( "'farm_info' does not represent an existing file." )

        with open( path, 'r' ) as readr:
            farm_info = json.load( readr )

    elif isinstance( farm_info, dict ):
        pass
    else:
        raise ValueError( "'farm_info' is neither a 'dict' nor a 'str'." ) 
        
    # get area-farm info 
    farmId2Area = {}
    for key, data in farm_info.items():
        farmId2Area[ data['id'] ] = data['catchment_area']
        
    tmp0 = df[ ( df['srcType'] == 'PF' ) & ( df['dstType'] == 'MM' ) ].copy()
    tmp0['Area'] = tmp0['srcId'].map( farmId2Area )
    tmp0['Market'] = tmp0[name].str.split('-').str[5].astype( int )

    # get flows from areas to mmen by day

    tmp1 = tmp0.groupby( ['Area', 'Market'], as_index = False )['nb'].sum()
    afl = tmp1.pivot_table(index = 'Area', columns = 'Market', values = 'nb', fill_value=0).reindex( range( nAreas ) ).values # absolute flux
    rfl = afl / np.sum( afl, axis = 1 )[:,None] # relative flux
    rfl[np.isnan(rfl)] = 0. # some rows could be just zeros 

    return rfl

def read_vendor_demography( path ):

    vd = pd.read_csv( path, names = ['id', 'type', 'mb', 'ms', 'l']).sort_values( by = 'id' )

    vct = vd.groupby(['l','type'], as_index = False).size() 
    vct['size'] = vct['size']/ vct.groupby('l')['size'].transform('sum')
    vct_tot = vd.groupby('type', as_index = False).size()
    vct_tot['size'] = vct_tot['size'] / len(vd)
    vct_tot['l'] = 'total'
    vct = pd.concat([vct, vct_tot]).set_index(['l','type'])

    return vct

def read_vendor_property_mappings( path ):

    vd = pd.read_csv( path, names = ['id', 'type', 'mb', 'ms', 'l']).sort_values( by = 'id' )

    # maps vendor to corresponding type (W or R)
    v2type = pd.Series( vd['type'].values, index=vd['id'] ).to_dict()

    # maps vendor to corresponding layer
    v2layer = pd.Series( vd['l'].values, index=vd['id'] ).to_dict()

    # maps vendor to corresponding market (buying phase)
    v2mb = pd.Series( vd['mb'].values, index=vd['id'] ).to_dict()

    # maps vendor to corresponding market (selling phase)
    v2ms = pd.Series( vd['ms'].values, index=vd['id'] ).to_dict()

    return v2type, v2layer, v2mb, v2ms

def get_vendor_transactions_detailed( df, path_info ):

    # read demography
    v2type, v2layer, v2mb, v2ms = read_vendor_property_mappings( path_info )

    dd = df[df['dstType'].isin(['V','M'])].copy()
    dd['typeSrc'] = 'unknown'
    dd['typeDst'] = 'unknown'
    dd['layerDst'] = -1

    # assign layer, vendor type to vendors involved
    # but must distinguish between transaction types as well

    # 1) MM -> V
    cond1 = dd['srcType'] == 'MM'
    dd.loc[cond1, 'typeSrc'] = 'MM'
    dd.loc[cond1, 'typeDst'] = dd.loc[cond1, 'dstId'].map( v2type )
    dd.loc[cond1, 'layerDst'] = 0

    # 2) V -> V
    cond2 = (dd['srcType'] == 'V') & (dd['dstType'] == 'V')
    dd.loc[cond2, 'typeSrc'] = dd.loc[cond2, 'srcId'].map( v2type )
    dd.loc[cond2, 'typeDst'] = dd.loc[cond2, 'dstId'].map( v2type )
    dd.loc[cond2, 'layerDst'] = dd.loc[cond2, 'dstId'].map( v2layer )

    # 3) V -> M (end-consumers)
    cond3 = (dd['srcType'] == 'V') & (dd['dstType'] == 'M')
    dd.loc[cond3, 'typeSrc'] = dd.loc[cond3, 'srcId'].map( v2type )
    dd.loc[cond3, 'typeDst'] = 'C'
    dd.loc[cond3, 'layerDst'] = dd.loc[cond3, 'srcId'].map( lambda x: v2layer[x] + 1 )

    return dd

def compute_inmarket_fluxes_bylayer( df ):

    r"""
    
    Computes the proportions of chickens sold to W and R by W (or MM), by market layer.

    Parameters
    ----------

    Returns
    -------

    """

    # -> layer 0 needs special treatment
    tmp1 = df[df['layerDst'] == 0].groupby(['layerDst', 'typeDst'], as_index = False)['nb'].sum()
    tmp1['nb'] = tmp1['nb'] / tmp1.groupby('layerDst')['nb'].transform('sum')
    tmp1

    # -> layers > 0 
    tmp2 = df[df['typeSrc'] == 'W'].groupby(['layerDst', 'typeDst'], as_index = False)['nb'].sum()
    tmp2['nb'] = tmp2['nb'] / tmp2.groupby('layerDst')['nb'].transform('sum')
    tmp2

    flux = pd.concat([tmp1, tmp2])
    flux = flux[flux['typeDst'].isin(['W','R'])].copy()
    
    return flux

def get_vendor_batch_size_distr( df, v2type, type = None ):

    r"""
    
    Computes the number of chickens bought daily by vendors

    Parameters
    ----------

    Returns
    -------

    """

    if isinstance( v2type, dict ):
        pass
    elif isinstance( v2type, str ):
        v2type = read_vendor_property_mappings( v2type )[0]
    else:
        raise ValueError( "v2type must be a dict or a str." )

    # get only transactions where vendor is sink
    dd = df[df['dstType'] == 'V'].copy()

    if type is None:

        # compute aggregated distribution

        histo = get_counts( dd.groupby(['dstId', 'day'])['nb'].sum() ) # !!! is day ok 4ever?
        return histo
    else:
        if isinstance( type, str ):

            # check that mapping is provided
            if not isinstance( v2type, dict ):
                raise ValueError( "Please specify vendor to type mapping." )

            if ( type == 'W' ): # W's distribution

                # filter transactions (select only W as sinks)
                wsIds = [ key for key, val in v2type.items() if val == 'W' ] 
                dd = dd[ dd['dstId'].isin( wsIds ) ]

                histo = get_counts( dd.groupby(['dstId', 'day'])['nb'].sum() ) # !!! is day ok 4ever?
                return histo

            elif ( type == 'R' ): # R's distribution
               
                # filter transactions (select only R as sinks)
                wsIds = [ key for key, val in v2type.items() if val == 'W' ] 
                dd = dd[ ~dd['dstId'].isin( wsIds ) ]

                histo = get_counts( dd.groupby(['dstId', 'day'])['nb'].sum() ) # !!! is day ok 4ever?
                return histo
            else:
                raise ValueError( "Please specify either W or R in 'type'." )
        else:
            raise ValueError( "'type' is either None or a str." ) 


def compute_vendor_outdegree_distr( df, v2type, dstType = ['W','R']  ):

    r"""

    Computes the number of distinct vendors met a single W sells to on a daily basis.

    Parameters
    ----------

    Returns
    -------

    """
    
    # check v2type

    if isinstance( v2type, str ):
        v2type = read_vendor_property_mappings( v2type )[0]
    elif isinstance( v2type, dict ):
        pass
    else:
        raise ValueError( "v2type must be a dict or a str." )

    # check dstTypes

    if isinstance( dstType, str ):
        if dstType in [ 'W', 'R' ]:
            dstType = [ dstType ]
        else:
            raise ValueError( "dstType must be 'W', 'R' or a list." )
    elif isinstance( dstType, list ):
        for el in dstType:
            if el not in [ 'W', 'R' ]:
                raise ValueError( "Unknown value encountered in dstType" ) 


    dd = df[(df['srcType'] == 'V') & (df['dstType'] == 'V')].copy()
    dd['srcVendorType'] = dd['srcId'].map( v2type )
    dd['dstVendorType'] = dd['dstId'].map( v2type )

    histo = get_counts( dd.loc[ ( dd['srcVendorType'] == 'W') & ( dd['dstVendorType'].isin( dstType ) )  ].groupby( ['day', 
                    'srcId' ] )[ 'dstId' ].nunique() )

    return histo

def compute_vendor_indegree_distr( df, v2type, dstType = ['W','R'] ):

    r"""

    Computes the number of distinct vendors met a single vendor buys from on a daily basis.

    Parameters
    ----------

    Returns
    -------

    """
    
    # check v2type

    if isinstance( v2type, str ):
        v2type = read_vendor_property_mappings( v2type )[0]
    elif isinstance( v2type, dict ):
        pass
    else:
        raise ValueError( "v2type must be a dict or a str." )

    # check dstTypes

    if isinstance( dstType, str ):
        if dstType in [ 'W', 'R' ]:
            dstType = [ dstType ]
        else:
            raise ValueError( "dstType must be 'W', 'R' or a list." )
    elif isinstance( dstType, list ):
        for el in dstType:
            if el not in [ 'W', 'R' ]:
                raise ValueError( "Unknown value encountered in dstType" ) 


    dd = df[(df['srcType'] == 'V') & (df['dstType'] == 'V')].copy()
    dd['srcVendorType'] = dd['srcId'].map( v2type )
    dd['dstVendorType'] = dd['dstId'].map( v2type )

    histo = get_counts( dd.loc[ ( dd['srcVendorType'] == 'W') & ( dd['dstVendorType'].isin( dstType ) )  ].groupby( ['day', 
                    'dstId' ] )[ 'srcId' ].nunique() )

    return histo

def compute_inmarket_SF( path, delimiter = ',' ):

    r"""
    
    Computes the survival distribution P(X > x) of individual marketing times.

    Parameters
    ----------

    Returns
    -------

    """

    dt_sell = np.loadtxt( path, delimiter = delimiter ) # read histogram
    pmf = dt_sell / dt_sell.sum()
    pmf = np.concatenate( [ np.array( [ 0. ] ), pmf] )
 
    sf = 1. - np.cumsum( pmf )

    return sf

def get_pianka_from_df( df, col_sp, col_res, col_val = 'nb' ):
    
    '''
    Computes Pianka's overlap index
    '''
    
    # get count matrix
    tmp = df.pivot_table( index = col_sp, columns = col_res, values = col_val, 
                         fill_value=0, aggfunc = 'sum' )
    
    p = tmp.values
    
    
    # normalize (each row -species- adds to 1)
    p = p / p.sum( axis = 1 )[:,None]
    
    # compute pianka's index as (p * p')_ij / (A_i * A_j)^0.5
    A = np.square( p ).sum( axis = 1 )
    num = np.dot( p, p.T )
    den = np.sqrt( np.outer( A, A ) )

    res = num / den
    
    return res

def get_presence_vec_from_df( df, col_i, col_j = 'market', col_val = 'nb' ):
    
    # get count matrix
    
    m = df.pivot_table( index = col_i, columns = col_j, values = col_val, 
                       fill_value=0, aggfunc='sum' )
    m = m.sort_index()
    m = m.values
    m = ( m > 0 ).astype( int )
    
    return m
    
def get_copresence_from_df( df, col_i, col_j = 'market', col_val = 'nb' ):
    
    '''
    Computes copresence matrix
    '''
    
    # get count matrix
    m = df.pivot_table( index = col_i, columns = col_j, 
                        values = col_val, fill_value=0, aggfunc = 'sum' )
    m = m.sort_index()
    m = m.values
    m = ( m > 0 ).astype( int )
    
    res = np.dot( m, m.T )
    
    return res