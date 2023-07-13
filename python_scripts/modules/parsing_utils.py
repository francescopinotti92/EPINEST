import numpy as np
import pandas as pd
from scipy.stats import nbinom

def isscalar( x ):
    if ( isinstance( x, int ) or isinstance( x, float ) ):
        return True
    else:
        return False

def isdict( x ):
    if ( isinstance( x, dict ) ):
        return True
    else:
        return False

def parse_args( args, nrep ):
    r"""

    Generates arrays of parameters for each parameter field specified in 'args'
    

    Parameters
    ----------

    args : dict
        Dict with all infos about each parameter field. It has the following fields:

            'mode' : string
                Specifies how the model will behave wrt this property. For example, if 'mode' = 'Constant',
                the model will draw a constant scalar, while if 'mode' = 'Geometric', the model will draw a
                value from a geometric distribution (but then you must specify a probability argument)
            '...' : int, floor or dict
                '...' stands for any parameter name as required by 'mode'. If value is int or floor, the
                same value will be assigned to each replicate. If dict, you must specify how you want to
                sample parameters ('mode') and any relevant arguments.
        
    nrep : int
        Number of replicates.

    Returns
    -------

    res : dict
        Contains a list of values for each parameter.

    """
    mode = args['mode']
    params = [ key for key in args.keys() if key != 'mode' ]
    
    res = { 'mode' : mode }

    for paramName in params:
        paramGenerationArgs = args[ paramName ]
        if isscalar( paramGenerationArgs ): # just a constant
            value = paramGenerationArgs
            res[ paramName ] = [ value ] * nrep            
        elif isdict( paramGenerationArgs ):
            modeGeneration = paramGenerationArgs['mode']
            if modeGeneration == 'Constant': # just a constant, but specified from dict
                value = paramGenerationArgs['value']
                res[ paramName ] = [ value ] * nrep
            elif modeGeneration == 'Custom': # random, uses a pdf
                values = paramGenerationArgs['values']
                pmf = paramGenerationArgs['pmf']
                res[ paramName ] = np.random.choice( a = values, p = pmf, size = nrep, replace = True )
            elif modeGeneration == 'List': # uses a list of values
                tmp = paramGenerationArgs['values']

                if isinstance( tmp, list ):
                    values = tmp
                elif isinstance( tmp, np.array ):
                    values = list( tmp.flatten() )
                elif isinstance( tmp, pd.Series ):
                    values = list( tmp.values )
                elif isinstance( tmp, pd.DataFrame ):
                    values = list( tmp[paramName].values )
                else:
                    raise( ValueError( "Param generation argument for {} is not supported".format( paramName ) ) )

                if ( len( values ) != nrep ):
                    raise ValueError("Number of values ({}) provided for {} does not match nrep".format( len( values ), paramName ) )

                res[ paramName ] = values
            elif modeGeneration == 'NegativeBinomial': # random, uses a negative binomial
                p = paramGenerationArgs['p']
                n = paramGenerationArgs['n']

                if 'bounds' in paramGenerationArgs: # sample between [bounds[0], bounds[1]]
                    bounds = paramGenerationArgs['bounds']
                    valmin, valmax = bounds
                    values = np.arange( valmin, valmax + 1 )
                    pmf = np.array( [ nbinom.pmf( k = x, p = p, n = n ) for x in values ] )
                    pmf = pmf / pmf.sum()
                    res[ paramName ] = np.random.choice( a = values, p = pmf, size = nrep, replace = True )
                else: # unrestricted sampling
                    res[ paramName ] = nbinom.rvs( p = p, n = n, size = nrep )
            else:
                raise( ValueError( "Param generation argument for {} is not supported".format( paramName ) ) )
        else:
            raise( ValueError( "Param generation argument for {} is not supported".format( paramName ) ) )

    return res




