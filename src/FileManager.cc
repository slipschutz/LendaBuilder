
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

  fileNotes="";//Default to nothing
  timingMode="";//Defualt to nothing InputManager responsible for defualts
  
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
    cout<<"Update run number parsing"<<endl; return "-1";
  }

  
  outputFileName << fileName.str()<<"-"<<fileNotes<<"-"<<timingMode;

  if (fileNum < 10){
    fileName<<"-0"<<fileNum;
  }else if (fileNum >=10 ){
    fileName<<"-"<<fileNum;
  }
    

  fileName << ".root";
  stringstream FullName;
  FullName<<"~/analysis/run"<<runNum<<"/"<<fileName.str();
  
  cout<<"Loading file "<<FullName.str()<<"..."<<endl;
  
  frunNum=runNum;
  return FullName.str();


}

TFile * FileManager::getOutputFile(){
  /*!
    Most Basic output file format
    Has only run-runNum-fileNotes-output.root
   !*/

  outputFileName << "-output.root";

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
  /*!
    
    
  !*/
  
  std::stringstream st;

  st<<"~/analysis/run"<<frunNum<<"/";

  st<<"FL"<<FL<<"FG"<<FG<<"d"<<d<<"w"<<w;

  st << outputFileName.str()<<".root";
  
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

TFile * FileManager::getOutputFile(Double_t sigma){
  std::stringstream st;

  st<<"sigma_"<<sigma<<"_";

  st << outputFileName.str()<<".root";

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
TFile *FileManager::getOutputFile(string s){
  TFile* temp =new TFile(s.c_str(),"recreate");
  outputFileName.str(s);

  if(!temp)
    {
      cout << "\nCould not open " << outputFileName.str() <<endl;
      exit(-1);
    } else
    cout << "Opened output file " <<outputFileName.str()<< endl;

  return temp;

}

TFile * FileManager::getOutputFile(Int_t LG,Int_t SG){

 


  outputFileName.str("");
  outputFileName<<"~/analysis/run"<<frunNum<<"/";
  outputFileName<<fileName.str()<<"-LG"<<LG<<"-SG"<<SG<<".root";

  TFile *temp =new TFile(outputFileName.str().c_str(),"recreate");
  cout<<"THE OUT PUT FILE IS "<<outputFileName.str()<<endl;
  return temp;

}
