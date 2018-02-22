//
//  thgem_chargingup_norim.C
//  
//
//  Created by Pedro on 27/08/15.
//
//
#include <iostream>
#include <fstream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <TCanvas.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TTree.h>
#include <time.h>


#include <TCanvas.h>
#include <TGraph.h>
#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TH1F.h>
#include <TH2F.h>
#include <TH3F.h>
#include <TTree.h>
#include <TString.h>
#include <time.h>
#include <TSystem.h>

#include "ViewField.hh"
#include "ViewCell.hh"
#include "ViewDrift.hh"

#include "MediumMagboltz.hh"
#include "GeometrySimple.hh"
#include "ComponentAnsys123.hh"
#include "Sensor.hh"
#include "AvalancheMicroscopic.hh"
#include "AvalancheMC.hh"
#include "Plotting.hh"


#include "Random.hh"
#include "fundamental_constants.hh"
#include "Helpers.h"

#include <sys/stat.h>

#include <omp.h>
#include <math.h>

#include "ChargingUpAnsys_norim.hh"
#include <numeric>
#include <algorithm>

using namespace std;
using namespace Garfield;

double epsilon=1e-4;
bool debug = false;


void kaptonChargesVector(int charge, double kapton, Double_t zend, double *vectorCharges,const int nSlices,double normstep){
    if (TMath::Abs(zend-(-kapton/2))<epsilon){
	if(charge==1) {
	  //	  cout<<"Ion stopped in kapton in slice "<<i<<" with ";
	  *(vectorCharges)+=0;	
	}
	else if(charge==-1) {
	  //cout<<"Electron stopped in kapton in slice "<<i<<" with ";
	  *(vectorCharges)-=0;	
	}
    }
    else if (TMath::Abs(zend-kapton/2)<epsilon) {
	if(charge==1) {
	  //	  cout<<"Ion stopped in kapton in slice "<<i<<" with ";
	  *(vectorCharges+nSlices-1)+=0;
	}
	else if(charge==-1) {
	  //cout<<"Electron stopped in kapton in slice "<<i<<" with ";
	  *(vectorCharges+nSlices-1)-=0;	
	}
    }

    else {
	for (int i=1; i < nSlices-1; i++){
	  //mudar aqui o modo como calcular os kapton positions, uma vez que o nSlices vai passar a ser nSlices+2, mas 
	  //o numero de slices do kapton vai passar entao a ser nSlices-2
	  if ((-kapton/2+(i-1)*(kapton/double(nSlices-2))<zend) && (zend <=-kapton/2+(i)*(kapton/double(nSlices-2)))){
	    if(charge==1) {
		//	  cout<<"Ion stopped in kapton in slice "<<i<<" with ";
		*(vectorCharges+i)+=normstep;
	    }
	    else if(charge==-1) {
		//cout<<"Electron stopped in kapton in slice "<<i<<" with ";
		*(vectorCharges+i)-=normstep;
	}
	    //cout<<"VectorCharges["<<i<<"]="<<*(vectorCharges+i)<<endl;
	    break;
	  }
	}
    }
}

void printKapton(const int nSlices, double kapton){
  for (int i=0; i < nSlices; i++){
    std::cout<<(-kapton/2+i*(kapton/double(nSlices-2))) <<" "<< (-kapton/2+(i+1)*(kapton/double(nSlices-2)))<<endl;
  }
}



int main(int argc, char * argv[]) {
	
if(argc!=11){
    cout << "Error: not enough arguments provided, exit"<<endl;
    return 0;
  }
  
  //funcion must be called as
  //./gem_chargingup nSlices nprimary_elec vgem gas_name IterationStart numberOfIterations Pressure Temperature step PATH_to_cutansys_results
  //ex  ./thgem_chargingup_norim 22 40 400 Ar 0 10 760 293 100000 cutansys_results
  const int nSlices=atoi(argv[1]);
  int npe=atoi(argv[2]);
  int vgem=atoi(argv[3]);
  char gasstr[5]="";
  std::strcpy(gasstr,argv[4]);
  int iterStart=atoi(argv[5]);
  int numberOfIterations=atoi(argv[6]);
  int p=atoi(argv[7]);
  int temp=atoi(argv[8]);
  int step=atoi(argv[9]);
  char fieldMapDir[200]="";
  std::strcpy(fieldMapDir,argv[10]);
  std::cout<<"Field maps will be available at "<<fieldMapDir<<endl;
  
  //get simulation parameters:
  char npeStr[5]="";
  sprintf(npeStr,"%d",npe);
  char vStr[5]="";
  sprintf(vStr,"%d",vgem);
  char pStr[5]="";
  sprintf(pStr,"%d",p);
  char tempStr[5]="";
  sprintf(tempStr,"%d",temp);
  char nSlicesStr[5]="";
  sprintf(nSlicesStr,"%d",nSlices);
  
  

  char elist_file[500];
  char nlist_file[500];
  char mplist_file[500];
  char prnsol_file[500];
    
    
    char mapfilesdir[500]="";
    std::strcat(mapfilesdir,fieldMapDir);    
    std::cout<<"mapFilesDir="<<mapfilesdir<<endl;
    
    double ChargesVector[ nSlices ];
    
    ChargingUpAnsys file(mapfilesdir, nSlices, ChargesVector, gasstr, vgem, npe);
    //this function has to be called before any TFile or TTree, otherwise the software will crash... 
    ////file.checkFileName();

    
    if (!file.checkSlicesFieldMaps()){
    	std::cout<<"Error 1, files don't exist"<<endl;
    	exit(0);
    }
    file.loadSlicesFieldMaps();
    
    //bool files_exist=file.loadSlicesFieldMaps();
    
    
    
    double simulatedCharges[ nSlices ];
    
    static const int simulatedCharges_size = sizeof(simulatedCharges) / sizeof(simulatedCharges[0]);
    cout<<"Number of slices in simulatedCharges="<<simulatedCharges_size<<endl;
    for (int i = 0; i < simulatedCharges_size; i++){
    
        simulatedCharges[i]=0.;
    }

      
    const int nThreads=omp_get_max_threads();
	cout<<"Number of available threads is "<<nThreads<<endl;
	
	// set gas composision
	TString gas_mixture(argv[4]);
	int n_gases=gas_mixture.Tokenize("-")->GetEntries();
	TString gas1=gas_mixture.Tokenize("-")->First()->GetName();
	TString gas2=gas_mixture.Tokenize("-")->Last()->GetName();
	float frac_mix=100.0;
	if(n_gases==2) {
	  frac_mix=atoi(&gas1.Data()[2]);
	  gas1.ReplaceAll(TString(&gas1.Data()[2]),"");
	  gas2=gas2.Tokenize("_")->First()->GetName();
	}
	if(n_gases!=2 && n_gases!=1) {cout <<"Error: Wrong gas composission - " << argv[4] << endl; return 0;}
	MediumMagboltz* gas = new MediumMagboltz();
    if(1==n_gases) gas->SetComposition(gas1.Data(), frac_mix);
	else gas->SetComposition(gas1.Data(), frac_mix, gas2.Data(), 100-frac_mix);
	gas->SetTemperature(temp);
	gas->SetPressure(p);
	gas->SetMaxElectronEnergy(300); // in eV for maximal electron energy for high fields increase to 200
	  
	//Penning properties
	const double lambda = 0.0;
	const double r=GetPenning(gas_mixture);
	gas->EnablePenningTransfer(r, lambda);
	if(gas1.Contains("ne")) gas->LoadIonMobility("/mnt/Lustre/detect/mpitt/garfield/Data/IonMobility_Ne+_Ne.txt");
	if(gas1.Contains("ar")) gas->LoadIonMobility("/mnt/Lustre/detect/mpitt/garfield/Data/IonMobility_Ar+_Ar.txt");   
	gas->EnableDebugging();
	gas->Initialise(true);
	gas->DisableDebugging();
        
    // Make a component from ansys file
    const double pitch = 1.0e-1;
	TString current_folder(TString(gSystem->pwd()).Tokenize("/")->Last()->GetName());
    const double kapton = TString(TString(current_folder.Tokenize("t")->At(1)->GetName()).Tokenize("_")->First()->GetName()).Atof()*0.1; // electrode thickness in cm
	const double rim = (TString(TString(current_folder.Tokenize("t")->At(1)->GetName()).Tokenize("_")->At(2)->GetName()).Atof()-0.5)*0.1*0.5;	// electrode rim in cm
	cout << "folder = " << current_folder.Data() << " , thickness=" <<kapton << ", rim = " << rim << endl;

	
    
    // Make a microscopic tracking class for electron transport
    //aval->EnableAvalancheSizeLimit(100);
    
    // const double cut = 10.0; //10eV, check the best value for each gas
    //aval->SetElectronTransportCut(cut);
    
    
    
    //int nbins=30;
    //float zlow=-0.1;
//    float zhigh=0.1;
    
    
    TGraph *graf = new TGraph(numberOfIterations); 
    
    std::cout<<"Before RootFile"<<endl;
    std::cout<<"After RootFile"<<endl;

	const double smear = pitch / 2.;
        
	
	if (iterStart!=0 && step){ // in case of step=0, will load uncharged slices (for gain simulation)
	  
	  file.UpdateFieldMap(simulatedCharges, iterStart);
	  
	}

	
	cout<<"Number of available threads is "<<omp_get_max_threads()<<endl;
 
	// Define usefull variables and instances
	AvalancheMicroscopic *aval[nThreads]; 
	Sensor *sensor[nThreads];
	AvalancheMC *drift[nThreads];
	Sensor* sensor_ion[nThreads];

	float gain[npe];

	Int_t iter_branch;
	Int_t aval_number;
	Double_t TimeI_elec;
	Double_t TimeF_elec;
	Double_t TimeF_ion;
	Double_t XI_elec;
	Double_t YI_elec;
	Double_t ZI_elec;
	Double_t XF_elec;
	Double_t YF_elec;
	Double_t ZF_elec;
	Double_t XF_ion;
	Double_t YF_ion;
	Double_t ZF_ion;
	Int_t ElecStatus;
	Int_t IonStatus;	
	
    for (int iter=iterStart; iter < numberOfIterations; iter++) {
	char iterStr[50]="";
	sprintf(iterStr,"%d",iter);
	char TreeIterName[50]="";
	std::strcpy(TreeIterName,"Tree");
	std::strcat(TreeIterName,iterStr);

	//create the file and the tree to store info:
	char rootfile[500]="ChargingUpTHGEM_";
	std::strcat(rootfile,vStr);
	std::strcat(rootfile,"V_");
	std::strcat(rootfile,gasstr);
	std::strcat(rootfile,"_");
	std::strcat(rootfile,pStr);
	std::strcat(rootfile,"torr_");
	std::strcat(rootfile,tempStr);
	std::strcat(rootfile,"K_");
	std::strcat(rootfile,npeStr);
	std::strcat(rootfile,"npe_");
	std::strcat(rootfile,iterStr);
	std::strcat(rootfile,"iter_PrimaryElec.root");
	   
	if(debug) cout << "opend file " <<  rootfile << endl;
	TFile f(rootfile,"recreate");	

    if(debug) cout << "Start iteration number = " << iter << endl;
	TH3D * TriDRimElecHist = new TH3D("ElecRim","Electrons in Rim", 100, -smear, +smear, 100, -smear, +smear, 100, -kapton, +kapton);
	TH3D * TriDRimIonHist = new TH3D("IonRim","Ions in Rim", 100, -smear, +smear, 100, -smear, +smear, 100, -kapton, +kapton);
	TH1F * SliceElecHist = new TH1F("SliceElecHist","Slices; slice number ; n electrons", nSlices, -0.5, nSlices+0.5);
	TH1F * SliceIonHist = new TH1F("SliceIonHist","Slices; slice number ; n ions", nSlices, -0.5, nSlices+0.5);
	
	
	if(debug) cout << "Define Tree " << endl;
	TTree * Tree1 = new TTree(TreeIterName,"A simple tree, each entry is an avalanche");
	
	//float AvalSize;
	//    const int nSlices=20;
	
	//maybe create another tree, with simplified information like the avalanche size for each avalanche
	//  std::vector<float> SliceNumberOfCharges(nSlices);
	Tree1->Branch("iter_branch",&iter_branch,"iter_branch/I");
	Tree1->Branch("aval_number",&aval_number,"aval_number/I");
	Tree1->Branch("TimeI_elec",&TimeI_elec,"TimeI_elec/D");
	Tree1->Branch("TimeF_elec",&TimeF_elec,"TimeF_elec/D");
	Tree1->Branch("TimeF_ion",&TimeF_ion,"TimeF_ion/D");
	Tree1->Branch("XI_elec",&XI_elec,"XI_elec/D");
	Tree1->Branch("YI_elec",&YI_elec,"YI_elec/D");
	Tree1->Branch("ZI_elec",&ZI_elec,"ZI_elec/D");
	Tree1->Branch("XF_elec",&XF_elec,"XF_elec/D");
	Tree1->Branch("YF_elec",&YF_elec,"YF_elec/D");
	Tree1->Branch("ZF_elec",&ZF_elec,"ZF_elec/D");
	Tree1->Branch("XF_ion",&XF_ion,"XF_ion/D");
	Tree1->Branch("YF_ion",&YF_ion,"YF_ion/D");
	Tree1->Branch("ZF_ion",&ZF_ion,"ZF_ion/D");
	Tree1->Branch("ElecStatus",&ElecStatus,"ElecStatus/I");
	Tree1->Branch("IonStatus",&IonStatus,"IonStatus/I");

    cout << "Define ComponentAnsys123() " << endl;
    ComponentAnsys123 *fm = new ComponentAnsys123();
	   
	if (iter==0 || step==0) {
	  
        std::strcpy(elist_file,mapfilesdir);
        std::strcat(elist_file,"/ELIST.lis");
        std::strcpy(nlist_file,mapfilesdir);
        std::strcat(nlist_file,"/NLIST.lis");
        std::strcpy(mplist_file,mapfilesdir);
        std::strcat(mplist_file,"/MPLIST.lis");
        std::strcpy(prnsol_file,mapfilesdir);
        std::strcat(prnsol_file,"/PRNSOL_");
        std::strcat(prnsol_file,vStr);
        std::strcat(prnsol_file,"V.lis");
            
        fm->Initialise(elist_file,nlist_file,mplist_file,prnsol_file,"mm");
        fm->EnableMirrorPeriodicityX();
        fm->EnableMirrorPeriodicityY();
        if(debug) fm->PrintRange();
        const int nMaterials = fm->GetNumberOfMaterials();
        for (int i = 0; i < nMaterials; ++i) {
            const double eps = fm->GetPermittivity(i);
            //cout << "permitivity: " << eps << "\n";
            if (eps == 1.) fm->SetMedium(i, gas);
        }
            
        if(debug) fm->PrintMaterials();
		
    }
    else{   
	    std::strcpy(elist_file,mapfilesdir);
	    std::strcat(elist_file,"/ELIST.lis");
	    std::strcpy(nlist_file,mapfilesdir);
	    std::strcat(nlist_file,"/NLIST.lis");
	    std::strcpy(mplist_file,mapfilesdir);
	    std::strcat(mplist_file,"/MPLIST.lis");
	    std::strcpy(prnsol_file,mapfilesdir);
	    std::strcat(prnsol_file,"/PRNSOL_");
	    std::strcat(prnsol_file,vStr);
	    std::strcat(prnsol_file,"V_");
	    std::strcat(prnsol_file,npeStr); 
	    std::strcat(prnsol_file,"npe_");
	    std::strcat(prnsol_file,gasstr);
	    std::strcat(prnsol_file,"_Iterable.lis");

	    fm->Initialise(elist_file,nlist_file,mplist_file,prnsol_file,"mm");
	    fm->EnableMirrorPeriodicityX();
	    fm->EnableMirrorPeriodicityY();
	    if(debug) fm->PrintRange();
	    const int nMaterials = fm->GetNumberOfMaterials();
	    for (int i = 0; i < nMaterials; ++i) {
		const double eps = fm->GetPermittivity(i);
		//cout << "permitivity: " << eps << "\n";
		if (eps == 1.) fm->SetMedium(i, gas);
	    }
	    
	    if(debug) fm->PrintMaterials();
	    
    }
	cout << "Finish to set up ComponentAnsys123()" << endl;
      

	/*
	  char PlotFilename[20]="";
	  ViewField *fieldView = new ViewField();
	  fieldView->SetComponent(fm);
	  fieldView->SetArea(-0.07, -0.1, 0.07, 0.1);
	  //	  fieldView->SetVoltageRange(-200., 200.);
	  fieldView->SetElectricFieldRange(0e3,300e3);
	  fieldView->SetPlane(0., -1., 0., 0.0002, 0., 0.);
	  //fieldView->Rotate(90);
	  std::strcpy(PlotFilename,"");
	  sprintf(PlotFilename,"%d",iter);
	  std::strcat(PlotFilename,".root");
	  TFile *f2 = new TFile(PlotFilename,"recreate");
	  f2->cd();
	  TCanvas* cF = new TCanvas();
	  fieldView->SetCanvas(cF);
	  fieldView->PlotContour("e");
	  cF->Write();
	  std::cout << "Writes: " << f2->GetName() << std::endl;
	  f2->Close();
	  //fieldView->PlotSurface();
	
	 */

	
	  iter_branch=iter;  
	  cout<<"You are running iteration "<<iterStr<<endl;
        //TH1F *histELEC = new TH1F(histELEC_name,"endZEelc",1000,zlow,zhigh);
        //TH1F *histION = new TH1F(histION_name,"endZIon",1000,zlow,zhigh);
		
	// Set up avalanche and drift for each thread:
	for(int i_fm=0;i_fm<nThreads;i_fm++){
	    sensor[i_fm] = new Sensor();
	    sensor[i_fm]->AddComponent(fm);
	    sensor[i_fm]->SetArea(-10 * pitch, -10 * pitch, -kapton ,10 * pitch      ,  10 * pitch,  3 * kapton);
	    sensor_ion[i_fm] = new Sensor();
	    sensor_ion[i_fm]->AddComponent(fm);
	    sensor_ion[i_fm]->SetArea(-10 * pitch, -10 * pitch, -kapton ,10 * pitch      ,  10 * pitch,  3 * kapton);
	    
		// Create an avalanche object
	    aval[i_fm] = new AvalancheMicroscopic();
	    aval[i_fm]->SetSensor(sensor[i_fm]);
		aval[i_fm]->SetCollisionSteps(100);
		
		// Necessary for the ions
	    drift[i_fm] = new AvalancheMC();
	    drift[i_fm]->SetDistanceSteps(10.e-4);	
	    drift[i_fm]->SetSensor(sensor_ion[i_fm]);
	}
	cout << "end Set up avalanche and drift for each thread" << endl;
	
	cout<<"------------------------------------------------- You are running iteration "<<iterStr<<endl;

	#pragma omp parallel for shared(gain)
    for (int i = 0; i<npe;i++) {
	    int thread_num=omp_get_thread_num();
          
            Double_t x0 =  (RndmUniform() * smear - smear/2);
            Double_t y0 =  (RndmUniform() * smear * sqrt(3)-smear * sqrt(3)/2);
    		//cout<<"Electron starting at x="<<x0<<" and y="<<y0<<endl;
            Double_t z0 = kapton;
            Double_t t0 = 0.0; 
            Double_t e0 = 0.0;
            if(debug) cout <<"avalanche = " << i << "thread = " << thread_num << ", input number of the variables: x0,y0,z0,t0=" << x0<<","<<y0<<","<<z0<<","<<t0 <<endl;
			if(debug) cout << "This thread is started"<<endl;			
			
			aval[thread_num]->AvalancheElectron(x0, y0, z0, t0, e0, 0., 0., 0.);
            int ne=aval[thread_num]->GetNumberOfElectronEndpoints();
			if(debug)  cout<<"The thread "<<omp_get_thread_num()<<" had a gain of "<<ne<<endl;
			gain[i]=ne;
			
			const int Aval_SIZE=ne;
			Double_t v_TimeI_elec[Aval_SIZE];
            Double_t v_TimeF_elec[Aval_SIZE];
            Double_t v_TimeF_ion[Aval_SIZE];
            Double_t v_XI_elec[Aval_SIZE];
            Double_t v_YI_elec[Aval_SIZE];
            Double_t v_ZI_elec[Aval_SIZE];
            Double_t v_XF_elec[Aval_SIZE];
            Double_t v_YF_elec[Aval_SIZE];
            Double_t v_ZF_elec[Aval_SIZE];
            Double_t v_XF_ion[Aval_SIZE];
            Double_t v_YF_ion[Aval_SIZE];
            Double_t v_ZF_ion[Aval_SIZE];
            int v_ElecStatus[Aval_SIZE];
            int v_IonStatus[Aval_SIZE];

		
		// Loop over all electrons in the avalanche
        for (int ie = 0; ie<ne;ie++) {
	      Double_t x0_ie, y0_ie, z0_ie, t0_ie, e0_ie;
	      Double_t x1_ie, y1_ie, z1_ie, t1_ie, e1_ie;
	      Double_t xi2_ie, yi2_ie, zi2_ie, ti2_ie;
	      Int_t status_ie, statusi_ie;
	      
		  aval[thread_num]->GetElectronEndpoint(ie,x0_ie,y0_ie,z0_ie,t0_ie,e0_ie,x1_ie,y1_ie,z1_ie,t1_ie,e1_ie,status_ie);
		  drift[thread_num]->DriftIon(x0_ie, y0_ie, z0_ie, t0_ie);
		  drift[thread_num]->GetIonEndpoint(0, x0_ie, y0_ie, z0_ie, t0_ie,xi2_ie, yi2_ie, zi2_ie, ti2_ie, statusi_ie);
		  if(debug) {
		  cout << "el ["<<ie<<"]: ("<<x0_ie<<","<<y0_ie<<","<<z0_ie<<") -> ("<<x1_ie<<","<<y1_ie<<","<<z1_ie<<")"<<endl;
		  cout << "ion ["<<ie<<"]: ("<<x0_ie<<","<<y0_ie<<","<<z0_ie<<") -> ("<<xi2_ie<<","<<yi2_ie<<","<<zi2_ie<<")"<<endl;}
        
        if(	status_ie!=-7 && ChargeIsAttached(x1_ie, y1_ie, z1_ie, kapton, rim)){        
		kaptonChargesVector(-1,kapton,z1_ie,&simulatedCharges[0], nSlices, double(step)/double(npe));
		SliceElecHist->Fill(GetSliceNumber(z1_ie,nSlices,kapton));
		if (TMath::Abs(z1_ie-(-kapton/2))<epsilon){
		  TriDRimElecHist->Fill(x1_ie,y1_ie,z1_ie);
		}
		else if (TMath::Abs(z1_ie-kapton/2)<epsilon) {
		  TriDRimElecHist->Fill(x1_ie,y1_ie,z1_ie);
		}
		}
		if(	statusi_ie!=-7 && ChargeIsAttached(xi2_ie, yi2_ie, zi2_ie, kapton, rim)){
		kaptonChargesVector(1,kapton,zi2_ie,&simulatedCharges[0], nSlices, double(step)/double(npe));
		SliceIonHist->Fill(GetSliceNumber(z1_ie,nSlices,kapton));
		if (TMath::Abs(zi2_ie-(-kapton/2))<epsilon){
		  TriDRimIonHist->Fill(xi2_ie,yi2_ie,zi2_ie);
		}
		else if (TMath::Abs(zi2_ie-kapton/2)<epsilon) {
		  TriDRimIonHist->Fill(xi2_ie,yi2_ie,zi2_ie);
		}
		}
		
				v_TimeI_elec[ie]=(t0_ie);
                v_TimeF_elec[ie]=(t1_ie);
                v_TimeF_ion[ie]=(ti2_ie);
                v_XI_elec[ie]=(x0_ie);
                v_YI_elec[ie]=(y0_ie);
                v_ZI_elec[ie]=(z0_ie);
                v_XF_elec[ie]=(x1_ie);
                v_YF_elec[ie]=(y1_ie);
                v_ZF_elec[ie]=(z1_ie);
                v_XF_ion[ie]=(xi2_ie);
                v_YF_ion[ie]=(yi2_ie);
                v_ZF_ion[ie]=(zi2_ie);
                v_ElecStatus[ie]=(status_ie);
                v_IonStatus[ie]=(statusi_ie);
		    
		    
        } // end loop over all electrons in the avalanche
		  
			
		// store information of all electrons in the avalanche in the tree:
		// Code syncronization, assign values to shared branches and write tree by one thread at a time
		#pragma omp critical
		{
		  for (int iie = 0; iie<ne;iie++) {
            aval_number=i;
            TimeI_elec=v_TimeI_elec[iie];
            TimeF_elec=v_TimeF_elec[iie];
            TimeF_ion=v_TimeF_ion[iie];
            XI_elec=v_XI_elec[iie];
            YI_elec=v_YI_elec[iie];
            ZI_elec=v_ZI_elec[iie];
            XF_elec=v_XF_elec[iie];
            YF_elec=v_YF_elec[iie];
            ZF_elec=v_ZF_elec[iie];
            XF_ion=v_XF_ion[iie];
            YF_ion=v_YF_ion[iie];
            ZF_ion=v_ZF_ion[iie];
            ElecStatus=v_ElecStatus[iie];
            IonStatus=v_IonStatus[iie];
            Tree1->Fill();			
		  }
		}  
		    

	    //delete aval[thread_num];
	    //delete sensor[thread_num];
	    //delete drift[thread_num];
	    //delete sensor_ion[thread_num];
            
	    // cout<<"Avalanche number "<<i<<" has ended"<<endl;
        } // end loop over npe
		
	//can calculate the gain for each primary avalanche
	  
        
	double sum=0;
	for (int i = 0; i < npe ; i++) sum=sum+gain[i];
	double m =  sum / float(npe);

	graf->SetPoint(iter,float(iter),m);
	  
	double m_error=m*1/sqrt(float(npe));
	std::cout<<"Mean "<<m<<" and with error "<<m_error<<endl;
	if(step) file.SaveGainFile(iter, m, m_error);
	  
	// gain.clear();
	for(int i=0;i<npe;++i) gain[i]=0;;
	  
	  
	  //        histELEC->Write();
        //histION->Write();
	  
	  
        
	  //histELEC->Delete();
        //histION->Delete();	
	  //Tree1->Show();
	  
	// finish with the step, down-charching the total accomulated charge
    //file.DownCharge(DownChargeLambda);
		
	if(step){  
	  file.UpdateFieldMap(simulatedCharges);
	  file.SaveKaptonChargesFile(iter);
	  file.printCurrentCharges();
	
	
	
	  for (int i = 0; i < simulatedCharges_size; i++){
	    
	    cout<<"Simulated charges vector "<<simulatedCharges[i]<<endl;
	    simulatedCharges[i]=0.;
	    
	  }
	}
	  //for (auto &charge : simulatedCharges){
	  //        cout<<"Simulated charges vector "<<charge<<endl;
	  //        charge=0.;
	  
	  //    }
	  f.cd();
	  Tree1->Write();
	  TriDRimElecHist->Write();
	  TriDRimIonHist->Write();
	  SliceElecHist->Write();
	  SliceIonHist->Write();
	  graf->Write();
	  f.Close();
	  //delete fm;
	  
    }	// end over all iterations

       
    return 1;
}
