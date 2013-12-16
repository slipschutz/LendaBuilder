

#include <algorithm>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <string>
#include "TTree.h"
#include <map>
#include "Settings.hh"
using namespace std;

class InputManager {

public:
  
  InputManager();
  ~InputManager();
  
  Bool_t loadInputs(vector <string> &);
  Bool_t loadInputs2(vector <string> &);
  
  void WriteSettings(Settings*);

  Double_t numFiles,runNum;

  Bool_t makeTraces,ext_flag;
  Bool_t correction;
  
  Bool_t isOldFormat;
  
  string fileNotes;
  string timingMode;
  Double_t FL,FG,d,w;
  Double_t traceDelay;
  
  Double_t timeWindow;
  Double_t timeWindowShift;
  
  Bool_t reMakePulseShape;

  Double_t long_gate,short_gate;
  Double_t long_gate2,short_gate2;
  
  string specificFileName;

  Double_t minMultiplicity;

  Bool_t lean;
  Bool_t fast;

  Double_t maxEntry;
  Double_t startEntry; 

  Double_t sigma;

  Double_t GammaPeakTime;
  Double_t GammaPeakTimeLiqLiq;

  Bool_t ext_sigma_flag;
private:
  vector <string> split (const string &s, char delim, vector<string> &elems);
  vector <string> split (const string &s, char delim);

  map <string ,Double_t*> ValidNumericalInputs;
  map <string, Bool_t* > ValidBoolInputs;
  map <string,string* >ValidStringInputs;
  
  void BuildInputMap();
  string lowerCase(string);

  void Print();
  Bool_t checkValues();
  
  vector<string> validTimingModes;
  void dumpValidModes();
  void PrintValues();
  void DumpAllOpitions();
};
