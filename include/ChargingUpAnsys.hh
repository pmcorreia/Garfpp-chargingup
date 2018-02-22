#define CHARGINGUPANSYS_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>


using namespace std;

class ChargingUpAnsys {
public:
    
    //to do list
    /* - method to check if the ansys files for all slices are present - DONE
     - method to open/update the PRNSOLiterable with the PRNSOL correspondent to each slice, taking into consideration the vector of currentCharges
     - etc
     - 25/01/2015 the method updatefieldmap shoud save the current mapfield configuration (number of electrons in each slice) in a file that will allow a future reconstruction of the mapfields
     because saving the mapfield (10 mb each) would cost a lot of memory, while saving a file with 20 or more rows is unexpensive)
     - Create the fieldmaps for each individual slice. Maybe we can also reduce the number of slices to 20 and analyse the results. 
     
     */
    
   
   
    //an array in each entry is the charge correspondent to each slice of the field map.
    
    //20 values for currentCharges are needed if we use 20 slices in the field map.

    int numberOfSlices;
    //static const int numberOfSlices=24;
  
    double *currentCharges;
    //double currentCharges[numberOfSlices];
    bool mapFileExists;
    const char *fieldMap;
    char fieldMapIterableName[500];
    char PRNSOLfile[500];
    char ELISTfile[500];
    char NLISTfile[500];
    char MPLISTfile[500];
    char PRNSOLSlicesFile[500];
    const char *gasstr;
    
    vector <vector <double> > voltages;
    vector<double> voltagesUncharged;
    vector<double> nodes;
    vector<int> slices;
    int numberOfNodes;
    int vgem;
    int nprimaries;
    
  ChargingUpAnsys(const char *Name, int nSlices, double *ChargesVector, char *gasName, int VGEM_value, int npe) {
        numberOfSlices=nSlices;
	  cout<<"Number of slices in the kapton = "<<numberOfSlices<<endl;
	  //	  currentCharges = (double*) malloc (nSlices+1);
	  //free (currentCharges);
        
	  currentCharges=ChargesVector;
	  fieldMap=Name;
	  std::cout<<"Field maps will be located at "<<fieldMap<<endl;
	  gasstr=gasName;
	  vgem=VGEM_value;
	  nprimaries=npe;
	  char npeStr[5]="";
	  sprintf(npeStr,"%d",nprimaries);
	  char vStr[5]="";
	  sprintf(vStr,"%d",vgem);
	  
        std::strcpy(PRNSOLfile,fieldMap);
        std::strcpy(ELISTfile,fieldMap);
        std::strcpy(NLISTfile,fieldMap);
        std::strcpy(MPLISTfile,fieldMap);
        std::strcat(PRNSOLfile,"/PRNSOL_");
        std::strcat(PRNSOLfile,vStr);
        std::strcat(PRNSOLfile,"V.lis");
        std::strcat(ELISTfile,"/ELIST.lis");
        std::strcat(NLISTfile,"/NLIST.lis");
        std::strcat(MPLISTfile,"/MPLIST.lis");
	
	for (int i = 0; i < nSlices; i++){
	  *(currentCharges+i)=0;
	}
	       
        mapFileExists=(std::ifstream(PRNSOLfile).good() and std::ifstream(ELISTfile).good() and std::ifstream(NLISTfile).good() and std::ifstream(MPLISTfile).good());
        std::strcpy(fieldMapIterableName,fieldMap);
        std::strcat(fieldMapIterableName,"/PRNSOL_"); 
        std::strcat(fieldMapIterableName,vStr);
        std::strcat(fieldMapIterableName,"V_");
        std::strcat(fieldMapIterableName,npeStr); 
        std::strcat(fieldMapIterableName,"npe_");
      std::strcat(fieldMapIterableName,gasstr);
        std::strcat(fieldMapIterableName,"_Iterable.lis");
        cout<<"FieldMapIterable will be "<<fieldMapIterableName<<endl;
        
        //need to check if all *.lis files are present in the directory, otherwise it will not work.
        if (mapFileExists) {
            cout<<"Files exists"<<endl;
            // ifstream source(PRNSOLfile, ios::binary);
            
            
           // dest << source.rdbuf();
            
           // source.close();
        }
        
        else{
	    cout<<"Files don't exists, Garfield++ will not be able to continue calculation without field map files!"<<endl;
	    cout<<"Check the files PRNSOL, MPLIST, ELIST and NLIST, that apparently are located at "<<PRNSOLfile<<endl;
        }
        
    }
    //end constructor ChargingUpAnsys
    
    
    bool checkSlicesFieldMaps(){
        
 
        char PRNSOLSlicesFile_check[500];
        char PRNSOL_unchargedFile[500];
        char slice_str[10];
        bool slicesFilesExists=true;
        std::strcpy(PRNSOLSlicesFile,fieldMap);
        std::strcat(PRNSOLSlicesFile,"/PRNSOL_slice");


	  char vStr[5]="";
	  sprintf(vStr,"%d",vgem);
	        
        std::strcpy(PRNSOL_unchargedFile,fieldMap);
        std::strcat(PRNSOL_unchargedFile,"/PRNSOL_");
        std::strcat(PRNSOL_unchargedFile,vStr);
        std::strcat(PRNSOL_unchargedFile,"V.lis");

	  //        std::strcat(PRNSOL_unchargedFile,"/PRNSOL.lis");
        
        if (!std::ifstream(PRNSOL_unchargedFile).good()) {
            slicesFilesExists=false;
            std::cout<<PRNSOL_unchargedFile<<" file is not present in the directory"<<endl;
        }
        
        for (int slice=1;slice<=numberOfSlices;slice++){
	    if (slice == 1){
		std::strcpy(PRNSOLSlicesFile_check,PRNSOLSlicesFile);
		std::strcat(PRNSOLSlicesFile_check,"RimBottom.lis");
	    }
	    else if (slice == numberOfSlices){
		std::strcpy(PRNSOLSlicesFile_check,PRNSOLSlicesFile);
		std::strcat(PRNSOLSlicesFile_check,"RimTop.lis");
	    }
	    else{
		
		sprintf(slice_str, "%d", slice-1);
		std::strcpy(PRNSOLSlicesFile_check,PRNSOLSlicesFile);
		std::strcat(PRNSOLSlicesFile_check,slice_str);
		std::strcat(PRNSOLSlicesFile_check,".lis");
	    }
	    switch (std::ifstream(PRNSOLSlicesFile_check).good()) {
	    case false:
		slicesFilesExists=false;
                    std::cout<<"File "<<PRNSOLSlicesFile_check<<" is not present!"<<endl;
                    break;
	    default:
		break;
	    }
        }
        if (slicesFilesExists) {
            std::cout<<"Slices files are present, simulation can continue."<<endl;
        }
        else {
             std::cout<<"Please copy the slices files to the directory before starting the charging-up simulation"<<endl;
        }
        return slicesFilesExists;
        
        
    }
    
    void loadSlicesFieldMaps(){
        
        //maybe create a dictionary instead of two vectors of floats (node and voltage)
        if (checkSlicesFieldMaps()){
            
            std::ofstream outfile("PRNfiles.txt");
            voltages.resize(numberOfSlices);
            //vector<float> voltages[numberOfSlices];
            //vector<float> voltages;
            
            std::ifstream file(PRNSOLfile);
            std::cout<<"PRNSOLfile "<<PRNSOLfile<<endl;
            std::string line;
            
            float node=0;
            float voltage=0;
            while (std::getline(file, line)){
                
                std::istringstream iss(line);
                
                
                if (iss >> node >> voltage) {
                    
                    iss >> node >> voltage;
                    nodes.push_back(node);
                   // cout<<node<<" "<<voltage<<endl;
                    voltagesUncharged.push_back(voltage);
                   // cout<<"node "<<nodes.back()<<endl;
                   // cout<<"voltage "<<voltagesUncharged.back()<<endl;
                    
                }
            }
            
           
            file.close();
            
            
            for (int slice=1 ; slice <= numberOfSlices ; slice++ ){
            //for (int slice=0;slice<numberOfSlices;slice++){
                
                slices.push_back(slice);
                
                char PRNSOLSlicesFile_check[1000];
                char slice_str[10];
                sprintf(slice_str, "%d", slice);
                //bool slicesFilesExists=true;
                std::strcpy(PRNSOLSlicesFile_check,fieldMap);
                std::strcat(PRNSOLSlicesFile_check,"/PRNSOL_slice");
          	    if (slice == 1){
			std::strcat(PRNSOLSlicesFile_check,"RimBottom.lis");
		    }
		    else if (slice == numberOfSlices){
			std::strcat(PRNSOLSlicesFile_check,"RimTop.lis");
		    }
		    else{
			
			sprintf(slice_str, "%d", slice-1);
			std::strcat(PRNSOLSlicesFile_check,slice_str);
			std::strcat(PRNSOLSlicesFile_check,".lis");
		    }
		    

		    
		    //		    std::strcpy(PRNSOLSlicesFile_check,PRNSOLSlicesFile);
		    
                std::ifstream file(PRNSOLSlicesFile_check);
		    //		    cout<<"Reached "<<PRNSOLSlicesFile_check<<endl;

                std::string line;
                
                //int *myArray;                //Declare pointer to type of array
                //myArray = new int[x];
        
                
                //pointer *voltage_slice esta sempre a apontar para a memoria de voltages, portanto
                //em memoria a matriz voltage_slice vai ser uma matriz de pointers, todos a apontar
                //o mesmo local de memoria
                //para alterar isto, tenho de cirar uma matriz voltages[][20] com 20 colunas
                //e linhas igual ao numero de nodos, que deve ser calculado previamente.
                
                while (std::getline(file, line)){
                    
                    std::istringstream iss(line);
                    
                    
                    if (iss >> node >> voltage) {
                        iss >> node >> voltage;
                        //if (slice==1) {
                        //    nodes.push_back(node);
                        //}
                        voltages[slice-1].push_back(voltage);
                    }
                }
                
                // to debug the first 5 nodes of each field map, in an output file called PRNfiles.txt
		    //    for (int i=0; i<5;i++){
			//                    outfile<<voltages[slice-1][i]<<" ";
                //}
                
                outfile<<endl;
                file.close();
                
            }
            outfile.close();
            
        }
    
        bool debugOption=false;
        
        if (debugOption) {
            
            for (int i=0; i<5;i++){
                std::cout<<voltages[0][i]<<" "<<voltages[1][i]<<endl;
            }
            
        }
        
        numberOfNodes=nodes.size();
        cout<<"Number of nodes is "<<numberOfNodes<<endl;
        
    }
    
    
    void printCurrentCharges(){
        for (int i=0;i<numberOfSlices;i++) {
            cout<<"Slice "<<i<<" is "<<currentCharges[i]<<endl;
        }
    }
    
    
    void checkFileName(){
        cout<<fieldMap<<endl;
        cout<<PRNSOLfile<<endl;
        cout<<MPLISTfile<<endl;
        cout<<NLISTfile<<endl;
        cout<<ELISTfile<<endl;
    }
    void SetFieldMapDir(const char * Name){
        fieldMap = Name;
    }
    
  void UpdateCurrentCharges(double *simulatedCharges){
        
    for (int i=0; i<numberOfSlices;i++){
	cout<<"Current charge in slice number "<<i<<" was "<<currentCharges[i];
	currentCharges[i]+=simulatedCharges[i];
	cout<<" and now is "<<currentCharges[i]<<endl;
    }
  }
  
  void DownCharge(double lambda){

    for (int i=0; i<numberOfSlices;i++){
        cout<<"Current charge in slice number "<<i<<" was "<<currentCharges[i];
        currentCharges[i]=currentCharges[i] - lambda*currentCharges[i];
        cout<<" and after downcharge is "<<currentCharges[i]<<endl;
    }
  }
  
  void UpdateCurrentCharges(double *simulatedCharges, int iter){
    //only is called if one wants to restart a previous simulation that ended in the iteration iter

    for (int j = iter-1; j < iter; j++){
	//you only need the kaptonCharge_iter#.txt of the previous iteration, since it contains
	//all the information that you need to do the next interations
	char kaptonChargesFileChar[500]="kaptonCharges_iter";
	char iter_char[10]="";
	sprintf(iter_char,"%d",j);
	strcat(kaptonChargesFileChar,iter_char);
	strcat(kaptonChargesFileChar,".txt");
	
	std::ifstream file(kaptonChargesFileChar);
	std::string line;
	while(std::getline(file, line))
	  {
	    std::istringstream iss(line);
	    
	    char sliceTxt[10]="";
	    int sliceInt;
	    char has[10]="";
	    double chargeValue;

	    if (iss >> sliceTxt >> sliceInt >> has >> chargeValue){
		//	cout<<"Sucess"<<endl;
		iss >> sliceTxt >> sliceInt >> has >> chargeValue;
		//	cout<<sliceTxt<<" "<<sliceInt<<" "<<has<<" "<<chargeValue<<endl;
		simulatedCharges[sliceInt]+=chargeValue;
	    }
	    else {
		cout<<"Some error occoured to read the previous kapton charges vectors"<<endl;
	    }
	  }
	
    }
    
    
    for (int i=0; i<numberOfSlices;i++){
	currentCharges[i]+=simulatedCharges[i];
	simulatedCharges[i]=0;
	cout<<"Current charge in slice number "<<i<<" for iteration "<<iter<<" is "<<currentCharges[i]<<endl;
    }
    
  }
  
    void SaveKaptonChargesFile(int iter){
        char kaptonChargesFileChar[500]="kaptonCharges_iter";
        char iter_char[10]="";
        sprintf(iter_char,"%d",iter);
        strcat(kaptonChargesFileChar,iter_char);
        strcat(kaptonChargesFileChar,".txt");
        
        
        std::ofstream kaptonChargesFile(kaptonChargesFileChar);
        
        for (int i=0; i<numberOfSlices;i++){
            kaptonChargesFile<<"Slice "<<i<<" has "<<currentCharges[i]<<endl;
        }
        
        kaptonChargesFile.close();
        
        
        
    }
    
  void SaveGainFile(int iter, double gain, double gain_std){
        char gainFileChar[500]="gain_iter";
        char iter_char[10]="";
        sprintf(iter_char,"%d",iter);
        strcat(gainFileChar,iter_char);
        strcat(gainFileChar,".txt");
        std::ofstream gainFile(gainFileChar);
	  gainFile<<iter<<" "<<gain<<" "<<gain_std<<endl;
	  gainFile.close();
        
    } 
  void SaveTransparencyFile(int iter, double transparency, double transparency_std){

        char transparencyFileChar[500]="transparency_iter";
        char iter_char[10]="";
        sprintf(iter_char,"%d",iter);
        strcat(transparencyFileChar,iter_char);
        strcat(transparencyFileChar,".txt");
        std::ofstream transparencyFile(transparencyFileChar);
	  transparencyFile<<iter<<" "<<transparency<<" "<<transparency_std<<endl;
	  transparencyFile.close();        
        
    }
   
    
    void UpdateFieldMap(double *simulatedCharges){
        
        ChargingUpAnsys::UpdateCurrentCharges(simulatedCharges);
        
        ofstream fieldMapIterable(fieldMapIterableName);
        
        cout<<"Number of nodes "<<numberOfNodes<<endl;
        

        for (int node = 0; node < numberOfNodes; node++) {
    
            double voltageToSave=voltagesUncharged[node];
            
            for (int slice = 1; slice <= numberOfSlices ; slice++ ){
            
               //cout<<simulatedCharges[slice-1]<<endl;;
                
               voltageToSave+=voltages[slice-1][node]*currentCharges[slice-1];
                
            }
            if (node<10){
                
                cout<<nodes[node]<<" "<<voltageToSave<<endl;
            }
            char voltageChar[20]="";
            sprintf(voltageChar,"%.5f",voltageToSave);
            fieldMapIterable<<nodes[node]<<" "<<voltageChar<<endl;
            
        }
        
        
        
        fieldMapIterable.close();
        
 
        //for 
        //to update field map file, it needs to update the charges of each slice, and then sum the charges to the correspondent updated file.
    }


  void UpdateFieldMap(double *simulatedCharges, int iterStr){
        
    ChargingUpAnsys::UpdateCurrentCharges(simulatedCharges, iterStr);
        
        ofstream fieldMapIterable(fieldMapIterableName);
        
        cout<<"Number of nodes "<<numberOfNodes<<endl;
        
        for (int i=0; i<numberOfSlices;i++){
	    cout<<"kaptonChargesFile slice "<<i<<" has "<<currentCharges[i]<<endl;
        }
	  
        for (int node = 0; node < numberOfNodes; node++) {
    
            double voltageToSave=voltagesUncharged[node];
            
            for (int slice = 1; slice <= numberOfSlices ; slice++ ){
		  //cout<<"Current Charge "<<currentCharges[slice-1]<<endl;
		 
		  //cout<<simulatedCharges[slice-1]<<endl;;
		  if (node<10){
		    //   cout<<"Voltages "<<voltages[slice-1][node]<<endl;
		  }
		  voltageToSave+=voltages[slice-1][node]*currentCharges[slice-1];
		  
            }
            if (node<10){
		  
		  //cout<<nodes[node]<<" "<<voltagesUncharged[node]<<endl;
		  //		  cout<<nodes[node]<<" "<<voltageToSave<<endl;
		}
		
            char voltageChar[20]="";
            sprintf(voltageChar,"%.5f",voltageToSave);
            fieldMapIterable<<nodes[node]<<" "<<voltageChar<<endl;
            
        }
        
        
        
        fieldMapIterable.close();
        
 
        //for 
        //to update field map file, it needs to update the charges of each slice, and then sum the charges to the correspondent updated file.
    }

    
    void ClearFieldMap(){
        for (int i=0; i<numberOfSlices;i++){
            currentCharges[i]=0;
        }    
    }
    
};

