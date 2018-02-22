#!/usr/bin/env python 
import os
import sys
import math
import numbers



if len(sys.argv)==9:
    numberOfSlices=22
    thickness=sys.argv[1] #0.4,0.8,1.2
    holediameter='0.5'
    rimsize=sys.argv[2]   #0.50, 0.52, 0.70
    VTHGEM=int(sys.argv[3])
    step=int(sys.argv[4])
    iterations=int(sys.argv[5])
    npe=int(sys.argv[6])
    gas=sys.argv[7] #'ne95-ch4'
    ncores=int(sys.argv[8])
    iterStart=0
    
        
else:
    print 'Found ',len(sys.argv),' agruments instead of 9'
    print 'Error: run ./'+sys.argv[0]+' [thickness] [rimsize] [VTHGEM] [step] [iterations] [npe] [gas] [nCores]'
    quit()



temp=293
P=760

#Folder where the Ansys field maps are stored
folder='THGEM_pitch1.0mm_thickness'+thickness+'mm_holediameter'+holediameter+'mm_rim'+rimsize+'mm_slices'+str(numberOfSlices)+'_ed0.5_ei0.5kVcm-1'

#path to a folder where field map directories are stored
InputFolder='/mnt/Lustre/detect/mpitt/ChargingUp/'
#path to a folder where scripts are 
ScriptFolder='/srv01/detect/mpitt/ChargingUp/code_THGEM/'

#script to run
script='thgem_chargingup_openMP'
if(float(holediameter)==float(rimsize)): script='thgem_chargingup_norim'

#create simulation directory (where the simulation results will be found):
simulationDir=InputFolder+'Jobs/ChargingUP_Gas_'+gas+'_t_'+thickness+'_rim_'+rimsize+'_'+str(VTHGEM)+'V_np_'+str(npe)+'_step'+str(step)+'_iter'+str(iterations)
os.system('mkdir '+simulationDir)


#Field maps, fodler
fieldMapDir=InputFolder+folder+'/cutansys_results'
fieldMapDirNEW = simulationDir+'/cutansys_results'

#create submission file, if done with step=0 submit number of files as the iterations
if step:
  submissionFile=simulationDir+'/charging_up_submission.sh'
  file = open(submissionFile,'w')
  file.write('#!/bin/sh\n\
  echo started `date` `pwd` on `hostname` with jobid $PBS_JOBID\n\
  \n\
  source ~/.zshrc\n\
  export GARFIELD_HOME=/mnt/Lustre/detect/mpitt/garfield\n\
  export OMP_NUM_THREADS='+str(ncores)+'\n\
  cd '+simulationDir+'\n\
  cp -r '+fieldMapDir+' .\n\
  mkdir output_files\n\
  '+ScriptFolder+'/'+script+' '+str(numberOfSlices)+' '+str(npe)+' '+str(VTHGEM)+' '+gas+' '+str(iterStart)+' '+str(iterations)+' '+str(P)+' '+str(temp)+' '+str(step)+' '+fieldMapDirNEW+'\n\
  \n\
  localSetupSFT --cmtConfig=x86_64-slc6-gcc48-opt pyanalysis/1.4_python2.7\n\
  '+ScriptFolder+'/ConvertAll.sh .\n\
  \n\
  echo finished at `date`\n\
  \n')
  file.close()
else:
  os.system('cp -r '+fieldMapDir+' '+fieldMapDirNEW)
  os.system('mkdir '+simulationDir+'/output_files')
  for i in range(iterations):
    submissionFile=simulationDir+'/charging_up_submission_iter'+str(i)+'.sh'
    file = open(submissionFile,'w')
    file.write('#!/bin/sh\n\
    echo started `date` `pwd` on `hostname` with jobid $PBS_JOBID\n\
    \n\
    source ~/.zshrc\n\
    export GARFIELD_HOME=/mnt/Lustre/detect/mpitt/garfield\n\
    export OMP_NUM_THREADS='+str(ncores)+'\n\
    cd '+simulationDir+'\n\
    '+ScriptFolder+'/'+script+' '+str(numberOfSlices)+' '+str(npe)+' '+str(VTHGEM)+' '+gas+' '+str(i)+' '+str(i+1)+' '+str(P)+' '+str(temp)+' '+str(step)+' '+fieldMapDirNEW+'\n\
    \n\
    localSetupSFT --cmtConfig=x86_64-slc6-gcc48-opt pyanalysis/1.4_python2.7\n\
    outfile=`/bin/ls ChargingUpTHGEM* | grep '+str(i)+'iter `\n\
    python '+ScriptFolder+'Convert.py  '+simulationDir+'/${outfile}\n\
    echo finished at `date`\n\
    \n')
    file.close()
  
  
  
  
  
  
  
  
  
  
  
  
  
  

