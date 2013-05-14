


#include "InputManager.hh"
#include <vector>
#include <string>


//



InputManager::InputManager()
{
  //set defualts
  runNum=-1;
  numFiles=1;//assume 1 file

  fileNotes="2Ch-G";

  makeTraces=false;
  correction=false;
  maxEntry=-1;

  specificFileName="";
  timingMode="softwareCFD";
  //defualt Filter settings see pixie manual
  FL=2;
  FG=0;
  d=3; //in clock ticks
  w =0.25;
  ext_flag=false;//defualt to none meta run format

  sigma=1.0;

  ext_sigma_flag=false;

  validTimingModes.push_back("internalCFD");
  validTimingModes.push_back("softwareCFD");
  validTimingModes.push_back("fitting");


}
InputManager::~InputManager()
{

}




vector <string>  InputManager::split (const string &s, char delim, vector<string> &elems) {
  stringstream ss(s);
  string item;
  while(getline(ss, item, delim)) {
    elems.push_back(item);
  }
  return elems;
}


vector <string> InputManager::split(const string &s, char delim) {
  vector<string> elems;
  return split(s, delim, elems);
}





Bool_t InputManager::loadInputs(vector <string> & inputs){

  vector <string> flags;
  vector <string> arguments;
  vector <string> temp(2);
  stringstream stream;
  //the first input has to be the run number
  
  if ( atoi(inputs[0].c_str() ) == 0 )
    cout<<"Must supply a integer runNumber"<<endl;
  else
    runNum = atoi(inputs[0].c_str());

  
  
  for (int i =1;i<(int) inputs.size();++i){
    
    temp = split(inputs[i],':');
    flags.push_back(temp[0]);
    arguments.push_back(temp[1]);
  }
  
  for (int i=0;i<(int)flags.size();++i){
    
    if (flags[i] == "numFiles")
      numFiles = atoi (arguments[i].c_str() );
    else if (flags[i] == "timingMode") 
      timingMode=arguments[i];
    else if (flags[i] == "numFiles")
      numFiles =atoi( arguments[i].c_str());
    else if (flags[i] == "makeTraces"){
      if (arguments[i] == "true" ){
	makeTraces=true;
      } else if (arguments[i] =="false" ){
	makeTraces =false;
      }else {
	cout<<"makeTraces needs to be true or false"<<endl;
	return false;
      }
    } else if (flags[i]=="correction"){
      if (arguments[i] == "true" ){
        correction=true;
      } else if (arguments[i] =="false" ){
        correction =false;
      }else {
        cout<<"correction needs to be true or false"<<endl;
        return false;
      }
    } else if (flags[i] == "inputFile"){
      specificFileName=arguments[i];
    } else if (flags[i] == "FL"){
      FL=atof(arguments[i].c_str() );ext_flag=true;
    } else if (flags[i] == "FG" ){
      FG=atof(arguments[i].c_str() );ext_flag=true;
    } else if (flags[i] == "d"){
      d=atof(arguments[i].c_str() );ext_flag=true;
    } else if (flags[i] == "w"){
      w=atof(arguments[i].c_str() );ext_flag=true;
    } else if (flags[i] == "sigma"){
      sigma = atof(arguments[i].c_str());ext_sigma_flag=true;
    } else if (flags[i]=="notes"){
      stream<<fileNotes<<"_"<<arguments[i];fileNotes=stream.str();
    } else if (flags[i]=="maxEntry"){
      maxEntry=atof(arguments[i].c_str());
    }else {
      cout<<flags[i] <<" :Unkown option"<<endl;
      return false;
    }  
  }
  
  return checkValues();
  
}


Bool_t InputManager::checkValues()
{
  Bool_t nothingWrong=true;
  Bool_t timingBool=false;
  
  if (numFiles <=0 || runNum <=0 )
    nothingWrong=false;

  if (makeTraces != true && makeTraces != false)
    nothingWrong =false;
  
  for (int i=0;i<(int)validTimingModes.size();++i){
    if (timingMode == validTimingModes[i])
      timingBool=true;
  }
  
  if (timingBool == false ){
    nothingWrong =false;
    dumpValidModes();
  }

  if (correction==true && specificFileName==""){
    nothingWrong =false;
    cout<<"You must specify an inputFile when running a corrections run"<<endl;
  }
  
  if (correction==false && specificFileName!=""){
    nothingWrong=false;
    cout<<"Specifying File only valid for correction run mode"<<endl;
  }
   

  if (timingMode != "fitting" && ext_sigma_flag == true ){
    cout<<"Can't set sigma without setting the timingMode to fitting "<<endl;
    nothingWrong = false;
  }

  if (timingMode != "softwareCFD" && ext_flag == true ){
    cout<<"Can't set filter paramters when timmingMode is not set to softwareCFD"<<endl;
    nothingWrong=false;
  }

  return nothingWrong;
}

void InputManager::dumpValidModes(){
  cout<<"The valid timing modes are "<<endl;
  for (int i=0;i<(int) validTimingModes.size();++i)
    cout<<validTimingModes[i]<<endl;
}
