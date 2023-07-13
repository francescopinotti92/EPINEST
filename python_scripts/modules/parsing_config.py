import itertools
import warnings

def expand_dict( dx, base_key = '', stop_str = '_', sep ='/' ):
    r"""
        Flattens dict with multi-scenario parameters.
        A few notes on using this function:
        
        1) If an entry is meant to be a dict itself, further parsing can
        be stopped by prepending a special character (stop_str) before
        the corresponding key.
        For example:
        
            { 'key' : { ... } }   no stop_str, does not prevent further parsing.
             
            { '_key' : { ... } }  stop_str, prevents further parsing.
               |
               |__ stop_str is '_'
    
        2) If an entry is meant to be a list of parameters (e.g. infectious periods for each strain),
        enclose them in another list.
        For example:
        
            { 'multi_valued_arg': [ [ value_1, value_2, ..., value_n ] ] }
            
        3) If intending to explore multiple values of a single argument, then enclose them in a list.
        For example, if a single scalar parameter would normally be specified as:
        
            { 'single_valued_key': value }
            
        a multi-parameter exploration is then specified through:
        
            { 'single_valued_key': [ value_1, value_2, ..., value_n ] }
            
        For multi-valued arguments, simply use a list of lists:
    
            { 'multi_valued_key': [ [ value_1_1, value_1_2 ], 
                                    ..., 
                                    [ value_n_1, value_n_2 ] }
                                    
        For dict-valued arguments, enclose dicts in a list and do not forget the stop_str:

            { '_dict_valued_key': [ { ... }, ... , { ... } ] }
            
        Parameters
        ----------
        
        dx : dict
            Contains all parameters necessary to run the analysis
            
        base_key : string
            Keys in the resulting dict will have 'base_key' prepended.
            Default argument ('') should be used when function is called externally;
            recursive calls will make use of this argument.
            
        stop_str : string
            A special character indicating keys with dict values that should not be
            parsed further.
            
        sep : string
            Will be used to concatenate nested keys.
        
        Returns
        -------
        
            res : dict
                A flattened version of the input dict. Keys are organized as in a filesystem,
                with 'sep' as a separator.

    
    """
    res = {}
    for key, val in dx.items():
        if not isinstance( key, str ):
            raise ValueError('key {} is not a string'.format() )

        key_strip = key.strip( stop_str )
        if base_key:
            key_final = sep.join( [ base_key, key_strip ] )
        else:
            key_final = key_strip
            
        if isinstance( val, list ):
            res[key_final] = val
        elif isinstance( val, dict ):
            if ( key[0] == stop_str ): # stop parsing
                res[key_final] = [val]
            else:
                inner_dict = expand_dict( val, base_key = key_final, stop_str = stop_str, sep = sep )
                res.update( inner_dict )
        else:
            res[key_final] = [val]
                
    return res

def combine_arguments( dx0, shared_dict = {}, linked_keys = [] ):
    r"""
    
    Given a flat dict with all model parameters and their values expressed as a list,
    create a number of individual configuration dicts, one for each scenario.
    
    Parameters
    ----------
    
    dx0 : dict
        A flat dict (see expand_dict documentation).
        
    shared_dict : dict
        These dict will be added to each setting list.
        
    linked_keys : list
        Specifies keys that will be paired. Can be a list of keys or a list
        of lists, each inner list containing keys that must be matched together.
        
        To illustrate the difference between linked and non-linked arguments,
        consider the following situation with two arguments, each having 2 values:
        
            { 'key_a': [ value_a_1, value_a_2 ],
              'key_b': [ value_b_1, value_b_2 ] }
              
        if 'key_a' and 'key_b' are not linked, the function takes the cross-product
        of their associated parameter lists, yielding the following combinations:
        
            [ { 'key_a': value_a_1, 'key_b': value_b_1 },
              { 'key_a': value_a_1, 'key_b': value_b_2 },
              { 'key_a': value_a_2, 'key_b': value_b_1 },
              { 'key_a': value_a_2, 'key_b': value_b_2 } ]
              
        howeverm if 'key_a' and 'key_b' are linked, the function forms combinations
        from ordered pairs:
        
            [ { 'key_a': value_a_1, 'key_b': value_b_1 },
              { 'key_a': value_a_2, 'key_b': value_b_2 } ]
        
        N.B. Linked arguments must have the SAME number of elements.
        
    
    Return
    ------
    
    res : list of dicts
        Each element is a dict containing parameters necessary to run an analysis of
        a single scenario.
    
    """
    
    #== Check parameters ==#
    
    if not isinstance( dx0, dict ):
        raise ValueError( 'dx0 must be a dict' )
        
    if not dx0:
        raise ValueError( 'dx0 is empty' )
    
    if not isinstance( shared_dict, dict ):
        raise ValueError( 'shared_dict must be a dict' )  
    
    if linked_keys:
        if isinstance( linked_keys[0], str ):
            linked_keys = [ linked_keys ] # adapt to later code
            
    
    #=========#
    
    dx = dx0.copy()
        
    # Create combinations for linked keys
    linked_params_bits = []
    for lkeys in linked_keys:
        
        tmp = []
        # 1) find true keys
        true_keys = []
        
        all_keys = list( dx.keys() )
        for lkey in lkeys:

            if lkey[-1] == '*': # check for special character
                true_keys.extend( [ el for el in all_keys if el.startswith( lkey[:-1] ) ] )
            else: # check if key exists
                if lkey in all_keys:
                    true_keys.append( lkey )
                else:
                    warnings.warn( '{} not found in dict'.format( lkey ) )
        
        # 2) check length of arguments (may throw error)
        
        n_args = -1
        for key in true_keys:
            if ( n_args == -1 ):
                n_args = len( dx[key] )
            else:
                if len( dx[key] ) != n_args:
                    raise ValueError('All arguments in [{}] must have the same number of value.'.format(
                    ' '.join( true_keys ) ) )
        if ( n_args == 1 ):
            warnings.warn('Arguments in [{}] have a single parameter value.'.format(
             ' '.join( true_keys ) ) )
        
        # 3) create new_dicts
        for i_arg in range( n_args ):
            tmp.append( { key: dx[key][i_arg] for key in true_keys } )
        
        linked_params_bits.append( tmp )

        # pop linked params
        for arg in true_keys:
            dx.pop( arg, None )

    # Create combinations for non-linked keys
    cross_params_bits = []
    all_params = list( itertools.product( *list( dx.values() ) ) )
    for params in all_params:
        cross_params_bits.append( { key: val for key, val in zip( dx.keys(), params ) } )

    all_bits = []
    all_bits = linked_params_bits
    all_bits.append( cross_params_bits )
        
    # combine linked and non-linked parameters
    combs = list( itertools.product( *all_bits ) )
    
    # merge dicts (combs is a list of lists each containing >= 2 dicts )
    res = []
    for comb_bits in combs:
        tmp = {}
        for bit in comb_bits:
            tmp.update( bit )
        res.append( tmp )
     
    # add shared keys
    if shared_dict:
        for el in res:
            el.update( shared_dict )
    
    return res

def nest_dict( dx, base_key = '', stop_str = '_', sep = '/' ):
    r"""
    
    Does the opposite of expand_dict: given a flat dict with keys
    organized as in a filesystem, it creates a nested dict.
    
    For example:
    
    given an input dict: 
    
        { 'key1/key1.1/key1.1.1' : value }
    
    returns:
    
        { 'key1': { 'key1.1': { 'key1.1.1': value } } }
    
    Parameters
    ----------
    
    dx : dict
        Flat dict.
        
    sep : string
        Character acting as separator in keys.
        
    Return
    ------
    
    res : dict
        A nested version of the input
    
    """
    
    res = {}
    
    for key, value in dx.items():
        bits = key.split( sep )
        tmp = res
        for bit in bits[:-1]:
            if not bit in tmp:
                tmp.setdefault( bit, {} )
            tmp = tmp[bit]
        tmp[bits[-1]] = value
        
    return res

def generate_configuration_files( dx0, stop_str = '_', sep = '/', linked_keys = [] ):
    
    r"""
    
    Serialize generation of configuration files.
    
    Parameters
    ----------
    
    dx0 : dict
        Contains all info about the analysis (see expand_dict for more info)

    stop_str : string
        Special character to specify dict-like values (see expand_dict for more info)

    sep : string
        Special character to join nested keys (see expand_dict for more info)

    linked_keys : list
        List of keys to link together when generating parameter combinations.
        (see combine_arguments for more info)
        
    Return
    ------
    
    res : list
        A list of dicts, each dict containing settings to generate a single scenario
        
    """

    dx = dx0.copy()

    # remove some invariant keys from parsing, add at the end.
    
    # flatten master configuration dict
    flat_dx = expand_dict( dx, stop_str = stop_str, sep = sep ) 
    
    # create parameter combinations
    individual_settings = combine_arguments( flat_dx, linked_keys = linked_keys )
    
    # nest individual scenario configuration dicts as in master configuration dict
    res = [ nest_dict( el, sep = sep ) for el in individual_settings ]
    
    return res