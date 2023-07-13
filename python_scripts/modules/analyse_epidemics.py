import os
import re
import sys
import json
import numpy as np
import pandas as pd
from analyse_result import has_column, get_counts, get_working_day

def get_loctype( df, col = 'loc', newcol = 'loctype' ):
    
    if col not in df.columns:
        raise ValueError( "No column '{}' in dataframe.\n".format( col ) )
        
    df['loctype'] = df['loc'].apply( lambda x: re.search( '(\D+)(\d+)', x ).group( 1 ) )
    
    return df
    

def read_infection_tree( path, locType = True, day = True ):
    '''
    
    Reads a transmission tree as a Pandas DataFrame
    
    
    '''
    
    try:
        df = pd.read_csv( path, names = [ 'i', 'j', 'type', 'strain', 't', 'loc' ] )
    except:
        raise ValueError( 'Could not open tree at {}'.format( path ) )
        
    if ( locType ):
        df = get_loctype( df )
        
    if ( day ):
        df['day'] = df['t'] // 24
        
    return df


def get_interfarm_infections( tree ):
    
    if 'loctype' not in tree.columns:
        df = get_loctype( df )

    # select events where recipient was in a farm 
    tmp = tree.loc[ tree['loctype'] == 'F' ] 
    
    # select events of type 0 (direct transmission)
    tmp = tmp.loc[ tmp['type'] == 0 ] 

    
    tmp['farm_i'] = tmp['i'].str.split('-').str[0]
    tmp['farm_j'] = tmp['j'].str.split('-').str[0]
    
    tmp = tmp.loc[ tmp['farm_i'] != tmp['farm_j'] ]
    
    return tmp
    
def get_offspring_size( tree, settings = None ):
    
    # get offspring size
    
    if settings is None: # use all settings (default)
        offspring = tree.groupby('i').size()
    elif isinstance( settings, list ): # select only incident events in some settings
        if 'loctype' not in tree.columns:
            tree = get_loctype( tree )
        offspring = tree.loc[ tree['loctype'].isin( settings ) ].groupby('i').size()
    
    # get chickens that did not generate new infections
    tmp = [ j for j in tree['j'].values if j not in offspring.index ]    
    offspring0 = pd.Series( [0] * len( tmp ), tmp ).rename( 'i' )
    
    # merge counts
    offspring = pd.concat( [offspring, offspring0] )
    
    return offspring