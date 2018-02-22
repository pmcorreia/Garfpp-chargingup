/*
Helper functions:
ChargeIsAttached - this function check the possition of a charged particle,
and depend on the geometry return if this particle possition is on the THGEM wall
ussage: ChargeIsAttached(x,y,z,thickness,rim_size); // all in cm

GetSliceNumber: return number of slice as defined in the thgem_chargingup executable
ussage: GetSliceNumber(z,nSlices,thickness); // all in cm
*/
#include <cmath>
#include "TString.h"
#define EPSILON 1E-4

bool ChargeIsAttached(Double_t,Double_t,Double_t,double,double);
int GetSliceNumber(double, int, double);
float GetPenning(TString);

bool ChargeIsAttached(Double_t x, Double_t y, Double_t z, double kapton, double rim){ // all in cm
float epsilon=EPSILON;
float tOver2=0.5*kapton;

// return false if charge not in the hole area (z)
if( z<(-tOver2-epsilon) ) return false; // charge exit a hole
if( z>( tOver2+epsilon) ) return false; // charge didnt enter a hole

float a=0.1;    // distance between holes in cm (pitch)
float asin60=a*0.5*sqrt(3); // a*Sqrt(3)/2 for hexagonal lattice
float d=0.05; 	// hole diameter in cm
float r=0.025; 	// hole radius in cm
float rim_outer=r+rim; // hole radius[cm] + rim[cm]
float radius_00 = sqrt(x*x+y*y);
float radius_x05 = sqrt((d-fabs(x))*(d-fabs(x))+(asin60-fabs(y))*(asin60-fabs(y)));
float radius_y00 = sqrt((a-fabs(x))*(a-fabs(x))+y*y);

//check if the geometry has rim and the charge is at z->+/-t/2
if(rim && (z<(-tOver2+epsilon) || z>(tOver2-epsilon)) ){
   if( radius_00<(rim_outer+epsilon) && radius_00>(r-epsilon) ) return true;
   if( radius_x05<(rim_outer+epsilon) && radius_x05>(r-epsilon) ) return true;
   if( radius_y00<(rim_outer+epsilon) && radius_y00>(r-epsilon) ) return true;
}
else{
   if( z<(-tOver2+epsilon) || z>( tOver2-epsilon) ) return false;
   if( fabs(radius_00-r)<epsilon ) return true;
   if( fabs(radius_x05-r)<epsilon ) return true;
   if( fabs(radius_y00-r)<epsilon ) return true;
}
return false;
}

int GetSliceNumber(double zend, int nSlices, double kapton){
  float epsilon=EPSILON;
  if (fabs(zend-(-kapton/2))<epsilon)
    return 0;
  else if (fabs(zend-kapton/2)<epsilon)
    return nSlices-1;
  else {
    for (int i=1; i < nSlices-1; i++){
      if ((-kapton/2+(i-1)*(kapton/double(nSlices-2))<zend) && (zend <=-kapton/2+(i)*(kapton/double(nSlices-2))))
        return i;
	}
  }		
  return -1;
}

float GetPenning(TString gas){
float r=0.0;
if(gas.Contains("ne95-ch4")) r=0.4;
else if(gas.Contains("ar95-ch4")) r=0.18;
else if(gas.Contains("ar95-co2")) r=0.35;
else if(gas.Contains("ar93-co2")) r=0.4;
else if(gas.Contains("ar70-co2")) r=0.57;
return r;
}

