{
TString folder="ChargingUP_Gas_ar95-co2_t_0.4_rim_0.50_900V_np_50_step1000000_iter5";


TString path=Form("/tmp/%s/",+gSystem->Getenv("LOGNAME"));
double m_atlasX=0.12, m_atlasY=0.83, m_atlasSize=0.07;
TLatex* text_ATLAS=new TLatex(); text_ATLAS->SetNDC(); text_ATLAS->SetTextFont(72); text_ATLAS->SetTextSize(m_atlasSize); 
TPaveText * lumi = new TPaveText(0.496644,0.452797,0.704698,0.872378,"NDC");
lumi->SetFillColor(0); lumi->SetLineColor(0);
lumi->SetTextFont(42); lumi->SetTextSize(0.034965); lumi->SetBorderSize(0);
bool FitSpectra=false; // fit to Polya distribution
bool ConverToTime=true;
bool LargeStepSize=false;
TString postfix="";
bool DoubleExpFit = false;
bool FitPoweLow = false; 

printf("Reading %s\n",folder.Data());
TObjArray* folder_arr=folder.Tokenize("_");
int nItem=folder_arr->GetEntries();
int Voltage = TString(folder_arr->At(nItem-5)->GetName()).ReplaceAll("V","").Atoi();
int step = TString(folder_arr->At(nItem-2)->GetName()).ReplaceAll("step","").Atoi();
int npe = TString(folder_arr->At(nItem-3)->GetName()).Atoi();
const int n=TString(folder_arr->At(nItem-1)->GetName()).ReplaceAll("iter","").Atoi();
TString gas = TString(folder_arr->At(2)->GetName());
float t=TString(folder_arr->At(4)->GetName()).Atof();
float rim= (TString(folder_arr->At(6)->GetName()).Atof()-0.5)*0.5;


float Gerr=2/sqrt(float(npe));
int avalanche_threshold=90;
float GainLimit=1;
int fitrange_max=-1;
int fitrange_min=0;
TGraphErrors * gr;
TF1 * fits;
double minGain=100, maxGain=5000;
double maxTime=0;
double energy_keV = 5.9; // source energy
TString rate="10";// in Hz/per hole (pi/8 mm2)
TString pen_str = "0.0";
TString time_str="min"; double TIME=1/60.0;
TString geo_str = Form("t%02d_a10_d05_h%03d",int(10*t),int(100*rim));
TString gas_str="";
float eps=t/20.0;
float wp=1, wpNE=36, wpCH4=28, wpCO2=33, wpAR=26; bool PureGas=false;
if (gas=="ne") {wp=wpNE; PureGas=true;gas_str="Ne";}
if (gas=="ne95-ch4") {wp=0.95*wpNE+0.05*wpCH4; pen_str="0.4";gas_str="Ne/CH4(5%)";}
if (gas=="ar") {wp=wpAR; PureGas=true;gas_str="Ar";}
if (gas=="ar70-co20") {wp=0.70*wpAR+0.05*wpCO2; pen_str="0.57";gas_str="Ar/CO2(30%)";}
if (gas=="ar93-co2") {wp=0.93*wpAR+0.05*wpCO2; pen_str="0.4";gas_str="Ar/CO2(7%)";}
if (gas=="ar95-co2") {wp=0.95*wpAR+0.05*wpCO2; pen_str="0.35";gas_str="Ar/CO2(5%)";}
if (gas=="ar95-ch4") {wp=0.95*wpAR+0.05*wpCH4; pen_str="0.18";gas_str="Ar/CH4(5%)";}
if(!PureGas) lumi->AddText(Form("THGEM; %s; pen=%s",gas_str.Data(),pen_str.Data()));
else lumi->AddText(Form("THGEM; Gas = %s",gas_str.Data()));
lumi->AddText( Form("t=%1.1f;a=0.1;d=0.5;%s",t,( (rim==0.0) ? "noRim":Form("h=%1.1f",rim))) );
lumi->AddText( "E_{DRIFT} = 0.5 kV/cm, E_{IND} = 0.5 kV/cm" );
lumi->AddText(Form("#DeltaV=%d;n_{AV}=%d",Voltage,npe));
if(ConverToTime) lumi->AddText("Rate="+rate+"[Hz];E_{#gamma}="+Form("%g",(energy_keV))+"[keV]");
float tau, tau2=0,err_tau2=0, err_tau, G0, GF;
bool logY=false;
TString steps_lg;
if(step>1000000000) steps_lg=Form("%dG",step/1000000000);
else if(step>=1000000) steps_lg=Form("%dM",step/1000000);
else if(step>1000.0) steps_lg=Form("%dK",step/1000);
else steps_lg=Form("%d",step);

TString dirname=Form("%s/ChargingUP_Gas_%s_t_%1.1f_rim_%2.2f_%dV_np_%d_step%d_iter%d/output_files",path.Data(),gas.Data(),t,0.5+rim*2,Voltage,npe,step,n);
cout << "Reads: " << dirname.Data() << endl;
TSystemDirectory dir(dirname, dirname);
TList *files = dir.GetListOfFiles();
if(!files){ cout << Form("Error: %s not exists!!!",dirname.Data()) << endl; return 0;}
TSystemFile *file;
TString fname;
TIter next(files);
double iteration[n];
double iteration_err[n];
double gain[n]; 
double gain_err[n];
TH1F * h[n];
TString filename[n];

for(int i=0;i<n;i++){ iteration[i]=-99;iteration_err[i]=0;gain[i]=-99;gain_err[i]=0;}
while ((file=(TSystemFile*)next())) {
  fname = file->GetName();
  if (!fname.Contains("new_ChargingUpTHGEM")) continue;
  TFile * tmp_file = TFile::Open(Form("%s/%s",dirname.Data(),fname.Data()));
  cout << Form("%s/%s",dirname.Data(),fname.Data()) << endl;
  TTree * tree = (TTree *)tmp_file->Get("tree");
  int iter=-1;
  for(int tkn=0;tkn<fname.Tokenize("_")->GetEntries();tkn++){
    if(TString(fname.Tokenize("_")->At(tkn)->GetName()).Contains("iter")){
	  iter=float(TString(fname.Tokenize("_")->At(tkn)->GetName())->ReplaceAll("iter","").Atoi());
	  break;
  }}
  iteration[iter]=iter;
  filename[iter]=fname;
  h[iter] = new TH1F(Form("h_%d",iter),"",100,3,tree->GetMaximum("el_n")+1);
  //tree->Draw(Form("el_n>>h_%d",iter),"el_n>1");
  tree->Draw(Form("Sum$(el_z<-%f)>>h_%d",iter,-0.05*t));
  //tree->Draw(Form("Sum$((el_status!=-7) && ((el_z<%f) || (el_z<%f && el_z>%f)))>>h_%d",-(2.*t-eps)/20,-(t-eps)/20,-(t+eps)/20,iter),"el_n>1");
  if(FitSpectra){
  TF1 * fitSpectra = new TF1("fitSpectra","TMath::Exp([0]-(1+[2])*[1]*x+[2]*log((1+[2])*[1]*x))",2,h[iter]->GetXaxis()->GetXmax());
  fitSpectra->SetParameter(0,log(h[iter]->GetMaximum()));
  fitSpectra->SetParameter(1,1.0/h[iter]->GetMean());
  fitSpectra->SetParameter(2,0.5); fitSpectra->SetParLimits(2,0,5);
  h[iter]->Fit(fitSpectra);
  gain[iter]=1.0/fitSpectra->GetParameter(1);
  }
  else gain[iter]=h[iter]->GetMean();
  gain_err[iter]=gain[iter]/TMath::Sqrt(h[iter]->GetEntries());
  //cout << fname<<": iter= " <<iter<<", " << gain[iter] <<  " h[iter]->GetEntries()= " << h[iter]->GetEntries() <<std::endl;
  delete tmp_file;
}
cout << "Done with reading files, get graphs " << endl;

int n_iterations=0;
for(int i=0;i<n;i++) {if(gain[i]>GainLimit) {n_iterations++;}}
const int n_iter_new = n_iterations; n_iterations=0;
double iteration_new[n_iter_new];
double iteration_err_new[n_iter_new];
double gain_new[n_iter_new]; 
double gain_err_new[n_iter_new];

//Scale iteration to minutes: 1 iteration = step / rate / nprimaries [sec]
double ConvertXaxis = ConverToTime ? (step/rate.Atoi()/(energy_keV*1000.0/wp))*TIME : 1;

for(int i=0;i<n;i++) iteration[i]*=ConvertXaxis;

for(int i=0;i<n;i++) {if(gain[i]>GainLimit){
iteration_new[n_iterations]=iteration[i];
iteration_err_new[n_iterations]=iteration_err[i];
gain_new[n_iterations]=gain[i];
gain_err_new[n_iterations]=gain_err[i];
n_iterations++;
}}
gr = new TGraphErrors(n_iterations,iteration_new,gain_new,iteration_err_new,gain_err_new);
gr->SetLineColor(kBlack); gr->SetLineWidth(1); gr->SetMarkerColor(kBlack); gr->SetMarkerStyle(20); gr->SetLineStyle(2);


fits = new TF1("fit","[0] - [1]*(1-TMath::Exp(-[2]*x))",0,n_iterations);
if(DoubleExpFit)
 fits = new TF1("fit","[0] - [1]*(1-TMath::Exp(-[2]*x))- [3]*(1-TMath::Exp(-[4]*x))",0,n_iterations);
if(FitPoweLow) 
  fits = new TF1("fit","[0] + [1]/TMath::Power(x,[2])",0,n_iterations);
  
fits->SetLineColor(kRed); fits->SetLineWidth(2);
float maxG0=TMath::MaxElement(n_iterations,gr->GetY()); G0= gr->GetY()[0]; GF = gr->GetY()[n_iterations-1], dG=(G0=GF);
//int i=0; while( ((gr->GetY()[i])<(maxG0*0.999)) ) { G0 += gr->GetY()[++i]; } G0/=float(i+1);
fits->SetParLimits(0,(1-Gerr)*G0,(1+Gerr)*maxG0);
if(FitPoweLow) fits->SetParLimits(0,(1-Gerr)*GF,(1+Gerr)*GF);
fits->SetParLimits(2,1/(n_iterations*ConvertXaxis),1/ConvertXaxis);
if(DoubleExpFit) fits->SetParLimits(4,1/(n_iterations*ConvertXaxis),1/ConvertXaxis*10.);

gr->Fit("fit","","-R",(fitrange_min>0 ? fitrange_min : 0)*ConvertXaxis,(fitrange_max>0 ? fitrange_max : n_iterations)*ConvertXaxis);
tau=1./fits->GetParameter(2);
err_tau=TMath::Abs(1./fits->GetParameter(2) - 1./(fits->GetParameter(2)+fits->GetParError(2)));
if(DoubleExpFit) {
tau2= (1./fits->GetParameter(4));
err_tau2 = TMath::Abs(1./fits->GetParameter(4) - 1./(fits->GetParameter(4)+fits->GetParError(4)));
}
G0=fits->GetParameter(0);
float dG0=fits->GetParError(0);
GF=fits->GetParameter(0)-fits->GetParameter(1);
dG=fits->GetParameter(1);
if(DoubleExpFit) GF-=fits->GetParameter(3);
float Rnp=rate.Atoi()*(energy_keV*1000.0/wp)/TIME;
float qtot=tau*Rnp*(G0 - dG/TMath::Exp(1))*1.6e-7;
float qtot_err=err_tau/tau*qtot + (tau*Rnp*dG0)*1.6e-7;
cout << "Error = 1 and second = " << err_tau*Rnp*(G0 - dG/TMath::Exp(1)) << " and " << tau*Rnp*dG0 << endl;
// Compute Chi2:
float chi2=0;
for(int i=0;i<n_iterations;i++) chi2+=TMath::Power(gain_new[i]-fits->Eval(iteration_new[i]),2)/(gain_err_new[i]*gain_err_new[i]);
chi2/=float(n_iterations);
TCanvas * c = new TCanvas("c1","c",600,600); c->SetTicks(1,1); c->SetLogy(logY);
gr->SetTitle(Form(";time [%s];Gain",time_str.Data())); gr->GetYaxis()->SetTitleSize(0.04); gr->GetYaxis()->SetTitleOffset(1.2);
if(!ConverToTime) {time_str="Iteration"; gr->SetTitle(Form(";%s;Gain",time_str.Data()));}
//gr[0]->GetXaxis()->SetLimits(50,10000);
//maxGain=G0*2.2; minGain=GF*0.5;
gr->GetYaxis()->SetRangeUser(minGain,maxGain);
if(maxTime) gr->GetXaxis()->SetRangeUser(0,maxTime);
gr->GetYaxis()->SetTitleSize(0.05);
gr->GetYaxis()->SetTitleOffset(1.0);
gr->GetYaxis()->SetLabelSize(0.04);
gr->GetXaxis()->SetLabelSize(0.04);
gr->GetXaxis()->SetTitleSize(0.05);
gr->GetXaxis()->SetTitleOffset(0.8);
gr->Draw("AP");
//TLegend* lg1 = new TLegend(0.127517,0.746503,0.395973,0.821678,"");
//lg1->AddEntry(gr,Form("step=%s[av]",steps_lg.Data()),"p");
//lg1->SetTextFont(22); lg1->SetTextFont(42); lg1->SetTextSize(0.034965);
//lg1->SetBorderSize(0);
//lg1->SetMargin(0.25);
//lg1->SetFillColor(kWhite);
//lg1->Draw(); 
//text_ATLAS->DrawLatex(m_atlasX,m_atlasY,"Preliminary"); 
// fit parameters:

//TPaveText * lumi2 = new TPaveText(0.562919,0.606643,0.78943,0.737762,"NDC");
//lumi2->SetFillColor(0); lumi2->SetLineColor(0);
//lumi2->SetTextFont(42); lumi->SetTextSize(0.034965); lumi->SetBorderSize(0);
lumi->AddText(Form("G^{FIT}_{0}=%2.2f; G^{FIT}_{F}=%2.2f",G0,GF));
//lumi->AddText(Form("G_{0}=%2.2f; G_{F}=%2.2f",gr->GetY()[0],gr->GetY()[n_iterations-1]));
lumi->AddText(Form("Fit #chi^{2}/n.d.f.=%2.2f; Q_{tot}=%3.2f#pm%3.2f[pC]",chi2,qtot,qtot_err));
if(LargeStepSize){
lumi->AddText(Form("#tau=%2.2f#pm%2.2f[%s]",tau,err_tau,time_str.Data()));
TPaveText * lumi2 = new TPaveText(0.483221,0.370629,0.692953,0.472028,"NDC");
lumi2->SetFillColor(0); lumi2->SetLineColor(0); lumi2->SetTextFont(22); lumi2->SetTextSize(0.054965); lumi2->SetBorderSize(0);
lumi2->AddText(Form("step=%s",steps_lg.Data())); lumi2->Draw();
}
else{lumi->AddText(Form("#tau=%2.2f#pm%2.2f[%s]; step=%s",tau,err_tau,time_str.Data(),steps_lg.Data()));}
if(DoubleExpFit) 
  lumi->AddText(Form("#tau_{2}=%2.2f#pm%2.2f[%s]",tau2,err_tau2,time_str.Data()));

lumi->Draw(); 
c->SaveAs(Form("Plots/png/Folder_%s_%s_dV%d_npe%d_%s_%s%s.png",geo_str.Data(),gas.Data(),Voltage,npe,steps_lg.Data(),time_str.Data(),postfix.Data()));
c->SaveAs(Form("Plots/eps/Folder_%s_%s_dV%d_npe%d_%s_%s%s.eps",geo_str.Data(),gas.Data(),Voltage,npe,steps_lg.Data(),time_str.Data(),postfix.Data()));

double ConvertTau = ConverToTime ? 1 : (step/rate.Atoi()/(energy_keV*1000.0/wp))*TIME;
cout << "gas\t voltage \t step \t G0 \t dG \t tau \t dtau \t Rnp \t np \t npe \t Qtot"<<endl;
cout << gas.Data()<<"\t"<<Voltage<<"\t"<<step<<"\t"<<G0<< "\t" <<GF<< "\t" <<tau*ConvertTau << "\t" <<  err_tau*ConvertTau <<"\t"<<rate.Atoi()*(energy_keV*1000.0/wp)/TIME<< "\t" << (energy_keV*1000.0/wp) << "\t" << npe << "\t" << qtot <<  endl;

TString getTime = Form("python ../code_THGEM/getTime2.py %s",dirname.Data());
getTime.ReplaceAll("output_files","");
cout << getTime.Data() << endl;

}

