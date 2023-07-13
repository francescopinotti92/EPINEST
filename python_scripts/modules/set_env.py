#!/usr/bin/env python3                                                                                                                          
# -*- coding: utf-8 -*-                                                                                                                                                           
"""                                                                                                                                                                    
Sets file paths to data and import modules depending on what host machine it is                                                   
Inspired by scripts from Piotr Bentkowski

"""

import sys
import re
from socket import gethostname

# determining on what computer this script is running                                                                             

host = gethostname()
localComps = ('zooadmins-MBP')

if host in localComps:
    ifCluster = False
    if ( host == 'zooadmins-MBP' ):
        folder_output_CBM = '/Users/user/Documents/project_simulations/Modeling_PDN'
        path_py_modules = '/Users/user/Dropbox/backup/modeling_PDN/python scripts/modules'
        if not path_py_modules in sys.path:
            sys.path.append( path_py_modules )
        
        from parsing_config import generate_configuration_files
        from setup_single_scenario import setup_single_scenario_Bangladesh

elif re.search('node', host) or host == 'clusterv':

    # for working on cluster                                                                                            
    #codz = '/home/bentkowski/geoData/codes_comm_to_region.csv'
    #inputDirr = '/home/bentkowski/dataRS'
    #inputScrips = '/home/bentkowski/fluABM/input/inputScripts'
    ifCluster = True
    #sys.path.append('/home/bentkowski/fluABM/analysis/')