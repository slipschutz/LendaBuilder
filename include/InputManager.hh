

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <string>
#include "TTree.h"

using namespace std;

class InputManager {

public:
  
  InputManager();
  ~InputManager();
  
  Bool_t loadInputs(vector <string> &);

  Int_t numFiles,runNum;

  Bool_t makeTraces,ext_flag;
  Bool_t correction;
  
  string fileNotes;
  string timingMode;
  Double_t FL,FG,d,w;

  string specificFileName;

  Double_t sigma;

  Bool_t ext_sigma_flag;
private:
  vector <string> split (const string &s, char delim, vector<string> &elems);
  vector <string> split (const string &s, char delim);

  Bool_t checkValues();
  
  vector<string> validTimingModes;
  void dumpValidModes();

};
