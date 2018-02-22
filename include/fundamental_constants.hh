#ifndef FUNDAMENTAL_CONSTANTS
#define FUNDAMENTAL_CONSTANTS
////////////////////////////////////////////////////////////////
///         conversions (according to NIST)                   //
////////////////////////////////////////////////////////////////

class fundamental_constants {
  //pressure
  double Torr2Pa;
  double Bar2Pa;
  
  ////////////////////////////////////////////////////////////////
  ///         constants   (according to NIST)                   //
  ////////////////////////////////////////////////////////////////
  double Kb;   //Boltzman's constant [JK-1]
 public:
  fundamental_constants(){
    //pressure
    Torr2Pa = 1.333224e2; //1 torr = 1.333224e2 Pa
    Bar2Pa = 1.0e5;       //1 bar = 10,000 Pa
    ////////////////////////////////////////////////////////////////
    ///         constants   (according to NIST)                   //
    ////////////////////////////////////////////////////////////////
    Kb = 1.3806504e-23;   //Boltzman's constant [JK-1]
    
    std::cout << "fundamental constants:\n";
    std::cout << "  Torr2Pa: " << Torr2Pa << "\n";
    std::cout << "  Bar2Pa: " << Bar2Pa << "\n";
    std::cout << "  Kb: " << Kb << " [J.K-1]\n";
  }

  double GetTorr2Pa(){
    return Torr2Pa;}
  double GetBar2Pa(){
    return Bar2Pa;}
  double GetKb(){
    return Kb;}
  
};
#endif
