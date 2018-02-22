#!/usr/bin/env python
#convert output from simulation to root easy-readable files
import os
import sys
import ROOT
import numpy as n

filename=sys.argv[1]
for par in filename.split('/')[-1].split('_'):
  if 'iter' in par: iter=par.replace('iter','')
tree=ROOT.TFile.Open(filename).Get('Tree'+str(iter))
if not tree: print 'Tree not found, exit'; quit()

#create a new file with ttree for the analysis
filepath="/".join(filename.split('/')[0:len(filename.split('/'))-1])
oldfilename=filename.split('/')[-1]
newfilename='new_'+oldfilename
newfile=ROOT.TFile.Open(filepath+'/output_files/'+newfilename,'recreate')
newtree = ROOT.TTree('tree','new tree of avalanches')

# create 1 dimensional float arrays (python's float datatype corresponds to c++ doubles)
el_n = n.zeros(1, dtype=float)
ion_n = n.zeros(1, dtype=float)
el_x = ROOT.std.vector(float)()
el_y = ROOT.std.vector(float)()
el_z = ROOT.std.vector(float)()
el_x_i = ROOT.std.vector(float)()
el_y_i = ROOT.std.vector(float)()
el_z_i = ROOT.std.vector(float)()
el_status = ROOT.std.vector(int)()
ion_x = ROOT.std.vector(float)()
ion_y = ROOT.std.vector(float)()
ion_z = ROOT.std.vector(float)()
ion_status = ROOT.std.vector(int)()
aval_number = n.zeros(1, dtype=float)

# create the branches and assign the fill-variables to them
newtree.Branch('el_n', el_n, 'el_n/D')
newtree.Branch('ion_n', ion_n, 'ion_n/D')
newtree.Branch('aval_number', aval_number, 'aval_number/D')
newtree.Branch( 'el_x_i', 'vector<float>', el_x_i )
newtree.Branch( 'el_y_i', 'vector<float>', el_y_i )
newtree.Branch( 'el_z_i', 'vector<float>', el_z_i )
newtree.Branch( 'el_x', 'vector<float>', el_x )
newtree.Branch( 'el_y', 'vector<float>', el_y )
newtree.Branch( 'el_z', 'vector<float>', el_z )
newtree.Branch( 'el_status', 'vector<int>', el_status )
newtree.Branch( 'ion_x', 'vector<float>', ion_x )
newtree.Branch( 'ion_y', 'vector<float>', ion_y )
newtree.Branch( 'ion_z', 'vector<float>', ion_z )
newtree.Branch( 'ion_status', 'vector<int>', ion_status )

isNewAvalanche=False
tree.GetEntry(0); 
old_aval_number=tree.aval_number;
#reset all variables
el_n[0]=0
ion_n[0]=0
aval_number[0]=0
for counter, ev in enumerate(tree):
  #Find new event, store previous variables in the tree
  if (old_aval_number) != ev.aval_number:
    newtree.Fill()
    old_aval_number=ev.aval_number
    el_n[0]=0
    ion_n[0]=0
    el_x_i.clear()
    el_y_i.clear()
    el_z_i.clear()
    el_x.clear()
    el_y.clear()
    el_z.clear()
    el_status.clear()
    ion_x.clear()
    ion_y.clear()
    ion_z.clear()    
    ion_status.clear()
  el_n[0]=el_n[0]+1
  ion_n[0]=ion_n[0]+1
  aval_number[0]=old_aval_number
  el_x_i.push_back(ev.XI_elec)
  el_y_i.push_back(ev.YI_elec)
  el_z_i.push_back(ev.ZI_elec)
  el_x.push_back(ev.XF_elec)
  el_y.push_back(ev.YF_elec)
  el_z.push_back(ev.ZF_elec)
  el_status.push_back(ev.ElecStatus)
  ion_x.push_back(ev.XF_ion)
  ion_y.push_back(ev.YF_ion)
  ion_z.push_back(ev.ZF_ion)
  ion_status.push_back(ev.IonStatus)  

#fill last avalanche
newtree.Fill()

# write the tree into the output file and close the file
print 'Writes: '+newfile.GetName()
newfile.Write()
newfile.Close()  

