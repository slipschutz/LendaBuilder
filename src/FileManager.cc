
#include "FileManager.h"

#include <iostream>
#include <sstream>

#include "TFile.h"
#include <stdlib.h>

using namespace std;
FileManager::FileManager()
{
  //Constructor for file manager

  outputFileName.str("Defualt");
  fileName.str("Defualt");


}

TString FileManager::loadFile(Int_t runNum,Int_t fileNum) {
  
  fileName.str("");
  outputFileName.str("");
  if (runNum <10){
    fileName<<"run-000"<< runNum;
  } else if (runNum < 100){
    fileName<<"run-00"<< runNum;
  } else if (runNum <1000) {
    fileName<<"run-0"<< runNum;
  } else {
    cout<<"Update run number parsing"<<endl; return "Crap";
  }

  if (fileNum < 10){
    fileName<<"-0"<<fileNum;
  }else if (fileNum >=10 ){
    fileName<<"-"<<fileNum;
  }
    

  outputFileName << fileName.str()<<"-output.root"; //For later                                                                                              
  fileName << ".root";
  
  cout<<"Loading file "<<fileName.str()<<"..."<<endl;
  
  
  
  return fileName.str();


}

TFile * FileManager::getOutputFile(){
 
  TFile * temp = new TFile(outputFileName.str().c_str(),"recreate");
   
  if(!temp)
    {
      cout << "\nCould not open " << outputFileName.str() <<endl;
      exit(-1);
    } else
      cout << "Opened output file " <<outputFileName.str()<< endl;

  return temp;
}

TFile * FileManager::getOutputFile(Double_t FL, Double_t FG, Double_t d, Double_t w){

  std::stringstream st;

  st<<"FL"<<FL<<"FG"<<FG<<"d"<<d<<"w"<<w*10;

  st << outputFileName.str();
  
  outputFileName.str(st.str());
  
  TFile * temp = new TFile(outputFileName.str().c_str(),"recreate");
   
  if(!temp)
    {
      cout << "\nCould not open " << outputFileName.str() <<endl;
      exit(-1);
    } else
    cout << "Opened output file " <<outputFileName.str()<< endl;

  return temp;
}