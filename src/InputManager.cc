


#include "InputManager.hh"
#include <vector>
#include <string>
#include <iomanip>

//



InputManager::InputManager()
{
  //set defualts
  runNum=-1;
  numFiles=1;//assume 1 file
  timingMode="softwareCFD";

  GammaPeakTime=-1;
  GammaPeakTimeLiqLiq=-1;


  specificFileName="";

  //defualt Filter settings see pixie manual
  FL=3;
  FG=0;
  d=3; //in clock ticks
  w =0;
  traceDelay=50;
  ext_flag=false;//defualt to none meta run format
  
  timeWindow=100; //defualt time window to be 100 ns
  timeWindowShift=0;//shift time window defaulted to 0

  fileNotes="";

  isOldFormat=false;

  long_gate =17;
  short_gate=7;

  long_gate2 =17;
  short_gate2=5;


  reMakePulseShape=false;
  sigma=1.0;
  maxEntry=-1;
  ext_sigma_flag=false;

  fast=false;
  lean=false;

  minMultiplicity=2;

  startEntry=0;

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
  ValidNumericalInputs["lg2"]=&long_gate2;
  ValidNumericalInputs["sg2"]=&short_gate2;

  ValidNumericalInputs["maxentry"]=&maxEntry;
  ValidNumericalInputs["timewindow"]=&timeWindow;
  ValidNumericalInputs["tracedelay"]=&traceDelay;
  
  ValidNumericalInputs["timewindowshift"]=&timeWindowShift;
  ValidNumericalInputs["twshift"]=&timeWindowShift;
  ValidNumericalInputs["startentry"]=&startEntry;
  ValidNumericalInputs["minmultiplicity"]=&minMultiplicity;
  ValidNumericalInputs["minn"]=&minMultiplicity;

  ValidNumericalInputs["gammapeaktime"]=&GammaPeakTime;
  ValidNumericalInputs["gammapeaktimeliqliq"]=&GammaPeakTimeLiqLiq;


  
  
  // ValidBoolInputs["remake"]=&reMakePulseShape;

  ValidBoolInputs["lean"]=&lean;
  ValidBoolInputs["fast"]=&fast;
  ValidBoolInputs["isoldformat"]=&isOldFormat;

  ValidStringInputs["timingmode"]=&timingMode;
  ValidStringInputs["inputfile"]=&specificFileName;
  ValidStringInputs["notes"]=&fileNotes;

}


Bool_t InputManager::loadInputs2(vector <string> & inputs){

  for (int i=0;i<(int)inputs.size();i++){
    if (lowerCase(inputs[i]) == "help"){
      DumpAllOpitions();
      return false;
    }
  }
  
  vector <string> Flags;
  vector <string> Arguments;


  vector <string> temp(2,"");
  //the first input has to be the run number
  
  if ( atoi(inputs[0].c_str() ) == 0 ){
    cout<<"Must supply a integer runNumber"<<endl;
    return false;
  } else
    runNum = atoi(inputs[0].c_str());


  
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

  PrintValues();

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


void InputManager::PrintValues(){
  
  int width=15;
  
  cout<<"\n####Settings for Builder####\n"<<endl;

  for (map<string,Double_t *>::iterator ii=ValidNumericalInputs.begin();
       ii!=ValidNumericalInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<"   Value "<<setw(width)<<*ii->second<<endl;
  }

  
  for (map<string,Bool_t *>::iterator ii=ValidBoolInputs.begin();
       ii!=ValidBoolInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<"   Value "<<setw(width)<<*ii->second<<endl;
  }

  
  for (map<string,string *>::iterator ii=ValidStringInputs.begin();
       ii!=ValidStringInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<"   Value "<<setw(width)<<*ii->second<<endl;
  }
  
  cout<<"\n############################\n"<<endl;
}

void InputManager::DumpAllOpitions(){

  int width =15;
  cout<<"\n####Valid Option Information for Builder####\n"<<endl;

  cout<<"\nValid Flags that take numerical inputs are: \n"<<endl;
  for (map<string,Double_t *>::iterator ii=ValidNumericalInputs.begin();
       ii!=ValidNumericalInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<" Default Value "<<setw(width)<<*ii->second<<endl;
  }

  cout<<"\nValid Flags that take bool inputs are: \n"<<endl;
  for (map<string,Bool_t *>::iterator ii=ValidBoolInputs.begin();
       ii!=ValidBoolInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<" Default Value "<<setw(width)<<*ii->second<<endl;
  }

  cout<<"\nValid Flags that take string inputs are: \n"<<endl;
  for (map<string,string *>::iterator ii=ValidStringInputs.begin();
       ii!=ValidStringInputs.end();ii++){
    cout<<"Flag "<<setw(width)<<ii->first<<" Default Value "<<setw(width)<<*ii->second<<endl;
  }

  cout<<"\n#############################################\n"<<endl;
}

void InputManager::WriteSettings(Settings* theSettings){

  int width =15;
  stringstream ss;

  theSettings->PushLine("\nFlags that take numerical inputs are: \n");
  for (map<string,Double_t *>::iterator ii=ValidNumericalInputs.begin();
       ii!=ValidNumericalInputs.end();ii++){
    ss<<"Flag "<<setw(width)<<ii->first<<" Value "<<setw(width)<<*ii->second<<endl;
    theSettings->PushLine(ss.str());
    ss.str("");
  }

  theSettings->PushLine("\nFlags that take bool inputs are: \n");
  for (map<string,Bool_t *>::iterator ii=ValidBoolInputs.begin();
       ii!=ValidBoolInputs.end();ii++){
    ss<<"Flag "<<setw(width)<<ii->first<<" Value "<<setw(width)<<*ii->second<<endl;
    theSettings->PushLine(ss.str());
    ss.str("");
  }

  theSettings->PushLine("\nFlags that take string inputs are: \n");
  for (map<string,string *>::iterator ii=ValidStringInputs.begin();
       ii!=ValidStringInputs.end();ii++){
    ss<<"Flag "<<setw(width)<<ii->first<<"ma Value "<<setw(width)<<*ii->second<<endl;
    theSettings->PushLine(ss.str());
    ss.str("");
  }

  theSettings->PushLine("\n#############################################\n");
}
