//
// Manager class to read in and setup a corrections file
// for each run.

#include "CorrectionManager.hh"


void CorrectionManager::addEntry(string tag,double val){

  if ( findEntry(tag) == -1 )//not already there
    {
      Entry e;e.tag=tag;e.value=val;
      theCorrections.push_back(e);
    }
  else {
    cout<<"Attempted duplicate Entry"<<endl;
  }
  

}



int CorrectionManager::findEntry(string tag){

  int spot=-1;
  for (int i=0;i<(int)theCorrections.size();++i){
    if (theCorrections[i].tag == tag ){
      spot =i;
      break;
    }
  }
  return spot;

}

void CorrectionManager::loadFile(int runNum){

  stringstream s;
  
  s<<"corrections_run_"<<runNum<<".txt";
  
  ifstream file;
  file.open(s.str().c_str());

  if (file.is_open()){
    double val;
    string tag,trash;
    
    while (file.good()){
      file>>trash>>tag>>val;
      addEntry(tag,val);
      
    }
    file.close();
  } else {
    cout<<"Correction file not found!"<<endl;
    cout<<"Using defualts"<<endl;
    s.str("");
    runNum=0;
    s<<"corrections_run_"<<runNum<<".txt";

    ifstream file;
    file.open(s.str().c_str());

    if (file.is_open()){
      double val;
      string tag,trash;

      while (file.good()){
	file>>trash>>tag>>val;
	addEntry(tag,val);

      }
      file.close();
    } else{
      cout<<"No correction files found"<<endl;
    }
  }
}
CorrectionManager::CorrectionManager(){}

void CorrectionManager::dump(){
  for (int i =0;i<(int)theCorrections.size();++i)
    cout<<"Tag "<<theCorrections[i].tag<<" Value "<<theCorrections[i].value<<endl;

}

Double_t CorrectionManager::get(string tag){

  int ind = findEntry(tag);
  if (ind == -1){
    cout<<"**Warning Correction "<<tag<<" not found"<<endl;
    return 1;
  }else {
    return (Double_t) theCorrections[ind].value;
  }
}
