
## Garfpp-chargingup

Algorithm for charging-up calculations in Micro Patterned Gaseous Detectors such GEM and THGEM, developed in Garfield++

This is a short manual for the simulation tool-kit of charging up process in THGEMs.

updated on February 2018


### Authors
* Pedro Correia [pmcorreia@ua.pt]
* Michael Pitt [michael.pitt@cern.ch]

###  References
1. P. M. M. Correia and M. Pitt, "Charging-up algorithm for garfield++” https://github.com/pmcorreia/Garfpp-chargingup.git, 2018.
1. P. M. M. Correia, M. Pitt et al., “Simulation of gain stability of THGEM gas-avalanche particle detectors “, [JINST (2018) 13 P01015](http://iopscience.iop.org/article/10.1088/1748-0221/13/01/P01015/meta)

## Table of contents (_for lxplus_): 
1. Integrate the code with GarfieldPP
1. Run with bsub (working on lxplus)


### Integrate the code with garfieldPP
Checking out the code:

`git clone https://github.com/pmcorreia/Garfpp-chargingup.git`

set up GARFIELD_HOME env

`export GARFIELD_HOME=PATH_TO_GARFIELD/GarfieldP`

set up root5.X

`lsetup "root 5.34.21-x86_64-slc6-gcc48-opt"`

compile the code

`make`

now you are ready to run the code, there are two examples in the Ansys folder, in order to use them, first need to unzip the folders, excecute the simulation in the run directory

`cd run`

`tar -zxvf ../Ansys/THGEM_pitch1.0mm_thickness0.4mm_holediameter0.5mm_rim0.50mm_slices22_ed0.5_ei0.5kVcm-1.tar.gz`

or 

`tar -zxvf ../Ansys/THGEM_pitch1.0mm_thickness0.4mm_holediameter0.5mm_rim0.70mm_slices22_ed0.5_ei0.5kVcm-1.tar.gz`

:exclamation::exclamation::exclamation: Once the simulation will be launched, the files will be modified. :exclamation::exclamation::exclamation:

:exclamation::exclamation::exclamation: don't run several simulations on the same folder, duplicate the folders to /tmp/ :exclamation::exclamation::exclamation:

Modify `ListRun.txt` accorging to what you need, and run:

`./Run.sh ListRun.txt`

the macro, will create an executable file in the /tmp/ directory, and will write the file that need to be executed. execute the suggested command to start the simulation. In the uploaded example, you should run:

`source /tmp/$LOGNAME/ChargingUP_Gas_ar95-co2_t_0.4_rim_0.50_900V_np_50_step1000000_iter5/charging_up_submission.sh`
the output will be in the

`/tmp/$LOGNAME/ChargingUP_Gas_ar95-co2_t_0.4_rim_0.50_900V_np_50_step1000000_iter5` folder, you can combine all file in a single ROOT file using next lines:

`localSetupSFT --cmtConfig=x86_64-slc6-gcc48-opt pyanalysis/1.4_python2.7`

`./ConvertToSingleROOT.sh /tmp/$LOGNAME/ChargingUP_Gas_ar95-co2_t_0.4_rim_0.50_900V_np_50_step1000000_iter5`

now you can analyze the output by modifying fist line in AnalyzeFolder.cc, and running it with root

`root -l AnalyseFolder.cc`

### Run with bsub
...

