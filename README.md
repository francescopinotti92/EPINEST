# EPINEST

An agent-based model for epidemic simulation in poultry production and distribution networks.

This repository contains code to install and run **EPINEST** and to analyse simulation outputs.

# Installation

**EPINEST** is written in C++ and must be compiled before running simulations. To compile **EPINEST**, open the terminal and move to the **CBM** folder. Once there, type **make** to compile the code. Compilation may print lots of stuff in your terminal and should complete in a few minutes. Importantly, it may be necessary to edit the **makefile** before running **make** by specifying the location of the Boost library in your computer (follow instructions at https://www.boost.org/ to download and install Boost). This can be done by opening **makefile** with a text editor and editing the value of BOOSTPATH. The present **makefile** has been tested on macOS (with clang++ compiler) and Linux (with g++ compiler) machines, but not on Windows.

# Creating configuration files

**EPINEST** requires several files and settings to be specified in advance. These define the layout of the production and distribution network, the epidemic model and many other aspects of the simulator. 

Notebooks contained in **python_scripts** contain code to modify simulation settings and create configuration files. Of note, files named **settings_X_Y.json** (X,Y are numbers) contain all the relevant info required by EPINEST. Example files are contained in **baseline_scenario_files/Config**, **market_scenario_files/Config** and **epidemic_scenario_files/Config**.

# Running EPINEST

After compilation, an executable file **CBMsimulator** will appear in **CBM/bin**. To run a single simulation, open the terminal and move to this folder. Then type:

`./CBMsimulator path_to_settings 0 1 0`

Where **path_to_settings** is the absolute path to a settings .json file (as those described above). The second argument should be always set to 0 and is irrelevant for now. The third argument is the number of (independent) simulations. The last argument is the random generator seed and can take any integer values.

For the examples provided in this repository, a single simulation may take up to 1 minute.

# Analysing results

Notebooks in **python_scripts** provide also code to analyse and plot results from simulations. **EPINEST** saves results in **Output** folders located alongside the corresponding **Config** files. The number and types of output files depend on specifications contained in configuration files: **EPINEST** writes only the information demanded by the user.

Available notebooks are:

- **poultry_movements_baseline.ipynb**: Simulates poultry movements without pathogen transmission. Analyses individual chicken transactions and other outputs relative to poultry movements.
- **poultry_movements_intermarket_mobility.ipynb**: Simulates poultry movements without pathogen transmission. Shows how to modify inter-market mobility and analyses poultry mixing in markets.
- **epidemic_simulation.ipynb**: Simulates poultry movements with pathogen transmission. Analyses transmission trees and other epidemiological outputs.
