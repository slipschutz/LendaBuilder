


#include "InputManager.hh"
#include <vector>
#include <string>


//



InputManager::InputManager()
{
  //set defualts
  runNum=-1;
  numFiles=1;//assume 1 file
  timingMode="softwareCFD";

  specificFileName="";

  //defualt Filter settings see pixie manual
  FL=2;
  FG=0;
  d=3; //in clock ticks
  w =0.25;
  ext_flag=false;//defualt to none meta run format

  long_gate =25;
  short_gate=14;
  reMakePulseShape=false;
  sigma=1.0;
  maxEntry=-1;
  ext_sigma_flag=false;
  lean=false;
  validTimingModes.push_back("internalCFD");
  validTimingModes.push_back("softwareCFD");
  validTimingModes.push_back("fitting");
  BuildInputMap();
}
InputManager::~InputManager()
{

}

void InputManager::BuildInputMap(){

  ValidNumericalInputs["numfiles"]=&numFiles;
  ValidNumericalInputs["fl"]=&FL;
  ValidNumericalInputs["fg"]=&FG;
  ValidNumericalInputs["w"]=&w;
  ValidNumericalInputs["d"]=&d;
  ValidNumericalInputs["lg"]=&long_gate;
  ValidNumericalInputs["sg"]=&short_gate;
  ValidNumericalInputs["maxentry"]=&maxEntry;
  
  // ValidBoolInputs["remake"]=&reMakePulseShape;

  ValidBoolInputs["lean"]=&lean;

  ValidStringInputs["timingmode"]=&timingMode;
  ValidStringInputs["inputfile"]=&specificFileName;
  

}


Bool_t InputManager::loadInputs2(vector <string> & inputs){

  vector <string> Flags;
  vector <string> Arguments;


  vector <string> temp(2,"");
  //the first input has to be the run number
  
  if ( atoi(inputs[0].c_str() ) == 0 )
    cout<<"Must supply a integer runNumber"<<endl;
  else
    runNum = atoi(inputs[0].c_str());

  cout<<"num of inputs is "<<inputs.size()<<endl;
  
  for (int i =1;i<(int) inputs.size();++i){
    
    temp = split(inputs[i],':');
    if (temp.size() != 2 ){//input was not a:v
      cout<<"***Warning input "<<inputs[i]<<" not recognized***"<<endl;
    } else{
      Flags.push_back(lowerCase(temp[0]));
      Arguments.push_back(lowerCase(temp[1]));
    }
  }

  for ( int i=0;i<(int)Flags.size();i++){
    if (ValidBoolInputs.find(Flags[i])!=ValidBoolInputs.end()){
      //Bool option found
      if (Arguments[i] == "true" || Arguments[i] == "yes" ||
	  Arguments[i] == "1"){

	*(ValidBoolInputs[Flags[i]])=true;
      }else if ( Arguments[i] == "false" || Arguments[i] == "no" ||
		 Arguments[i] == "0"){
	

	*(ValidBoolInputs[Flags[i]])=false;
      } else {
      // bool argument not found 
	cout<<"***Warning "<<Arguments[i]<<" Is not a valid argument for bool flag: "<<Flags[i]<<endl;
      }
    } else if ( ValidNumericalInputs.find(Flags[i]) != ValidNumericalInputs.end()){
      // Numerical option found
      *(ValidNumericalInputs[Flags[i]])=atof(Arguments[i].c_str());
      
      //Special internal flag setting
      if (Flags[i] == "fl" || Flags[i]=="fg" || Flags[i]=="w" ||Flags[i]=="d"){
	ext_flag=true;
      } else if (Flags[i]=="sg" || Flags[i]=="lg"){
	reMakePulseShape=true;
      }

    } else if (ValidStringInputs.find(Flags[i]) != ValidStringInputs.end()){
      //a string input
      *(ValidStringInputs[Flags[i]]) = Arguments[i];
      
    } else {
      cout<<"Unkown option "<<Flags[i]<<endl;
    }

  }

  Print();

  return checkValues();
}

string InputManager::lowerCase(string data){
  std::transform(data.begin(), data.end(), data.begin(), ::tolower);
  return data;
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
    else if (flags[i] == "inputFile"){
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
    } else if (flags[i] == "LG"){
      long_gate=atof(arguments[i].c_str());
      reMakePulseShape=true;
    } else if (flags[i] == "SG"){
      short_gate=atof(arguments[i].c_str());
      reMakePulseShape=true;
    }else {
      cout<<flags[i] <<" :Unkown option"<<endl;
      return false;
    }  
  }
  
  Print();
  return false;
  return checkValues();
  
}


Bool_t InputManager::checkValues()
{
  Bool_t nothingWrong=true;
  Bool_t timingBool=false;
  
  if (numFiles <=0 || runNum <=0 )
    nothingWrong=false;

  
  for (int i=0;i<(int)validTimingModes.size();++i){
    if (timingMode == validTimingModes[i])
      timingBool=true;
  }
  
  if (timingBool == false ){
    nothingWrong =false;
    dumpValidModes();
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


void InputManager::Print(){

  cout<<"Run Number is "<<runNum<<endl;
  cout<<"Num Files is "<<numFiles<<endl;
  cout<<"ext_flag is "<<ext_flag<<endl;
  cout<<"remake is "<<reMakePulseShape<<endl;
  cout<<"timing mode "<<timingMode<<endl;
  cout<<"FL "<<FL<<endl;
  cout<<"FG "<<FG<<endl;
  cout<<"d "<<d<<endl;
  cout<<"w "<<w<<endl;
  cout<<"filename "<<specificFileName<<endl;

}
