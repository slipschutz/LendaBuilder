#include <stdio.h>
#include <stdlib.h>

#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TMath.h"
#include <TRandom1.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include "TRandom3.h"
#include "TTree.h"
#include "TString.h"
#include "TSystem.h"
#include "TGraph.h"
#include "TChain.h"
#include "TRandom2.h"

//Local Headers
#include "LendaEvent.hh"
#include "ddaschannel.h"
#include "Filter.hh"
#include "FileManager.h"
#include "InputManager.hh"
#include "CorrectionManager.hh"
#include <ctime>
#include <iomanip>

#include "functions.hh"
#define BAD_NUM -10008


using namespace std;


int main(int argc, char **argv){

  vector <string> inputs;
  for (int i=1;i<argc;++i){
    inputs.push_back(string(argv[i]));
  }
  if (inputs.size() == 0 ){ // no argumnets display helpful message
    cout<<"Usage: ./GammaBuilder runNumber [options:value]"<<endl;
    return 0;
  }  
  
  InputManager theInputManager; 
  if ( !  theInputManager.loadInputs2(inputs) ){//load options and check everything
    return 0;
  }



  
  ////////////////////////////////////////////////////////////////////////////////////

  //load settings
  
  Double_t sigma=theInputManager.sigma; // the sigma used in the fitting option

  Int_t runNum=theInputManager.runNum;
  Int_t numFiles=theInputManager.numFiles;

  Long64_t maxentry=theInputManager.maxEntry;

  

  Bool_t extFlag=theInputManager.ext_flag;
  Bool_t ext_sigma_flag=theInputManager.ext_sigma_flag;

  //defualt Filter settings see pixie manual
  Double_t FL=theInputManager.FL;
  Double_t FG=theInputManager.FG;
  int CFD_delay=theInputManager.d; //in clock ticks
  Double_t CFD_scale_factor =theInputManager.w;



  

  //prepare files and output tree
  ////////////////////////////////////////////////////////////////////////////////////
  TFile *outFile=0;
  TTree  *outT;
  FileManager * fileMan = new FileManager();
  fileMan->fileNotes = theInputManager.fileNotes;
  fileMan->timingMode =theInputManager.timingMode;
  //The input trees are put into a TChain
  TChain * inT;
  
  inT= new TChain("dchan");//dchan is name of tree in ddas rootfiles
  if (numFiles == -1 ){
    TString s = fileMan->loadFile(runNum,0);
    inT->Add(s);
  } else {
    for (Int_t i=0;i<numFiles;i++) {
      TString s = fileMan->loadFile(runNum,i);
	inT->Add(s);
    }
  }
  
  
  
  Long64_t nentry=(Long64_t) (inT->GetEntries());
  cout <<"The number of entires is : "<< nentry << endl ;
  
  // Openning output Tree and output file
    
  //Note FileManager expects fileMan->loadFile before getting outputfile
  if (extFlag == false && ext_sigma_flag==false) //if there are no flags
    outFile = fileMan->getOutputFile();
  else if (extFlag == true && ext_sigma_flag==false){
    outFile = fileMan->getOutputFile(FL,FG,CFD_delay,CFD_scale_factor*10);
  } else if (extFlag==false && ext_sigma_flag==true){
    sigma=sigma/10;
    outFile= fileMan->getOutputFile(sigma*10);
  }

  outT = new TTree("flt","Filtered Data Tree --- Comment Description");
  cout << "Creating filtered Tree"<<endl;
  if(!outT)
    {
      cout << "\nCould not create flt Tree in " << fileMan->outputFileName.str() << endl;
      exit(-1);
    }
  ////////////////////////////////////////////////////////////////////////////////////
  
  
  
  // set input tree branvh variables and addresses
  ////////////////////////////////////////////////////////////////////////////////////
  ddaschannel * inChannel = new ddaschannel();
  inT->SetBranchAddress("dchan",&inChannel);

  
  //Specify the output branch
  LendaEvent* Event = new LendaEvent();
  outT->Branch("Event",&Event);
  //  outT->BranchRef();

  ////////////////////////////////////////////////////////////////////////////////////


 
  ////////////////////////////////////////////////////////////////////////////////////
  
  if(maxentry == -1)
    maxentry=nentry;
  else if (maxentry > nentry)
    maxentry=nentry;

  //non branch timing variables 
     ////////////////////////////////////////////////////////////////////////////////////


  Filter theFilter; // Filter object
  ////////////////////////////////////////////////////////////////////////////////////


  //  vector <Sl_Event*> CorrelatedEvents(4,NULL);
  map <Long64_t,bool> mapOfUsedEntries;//Used to prevent double counting
  Sl_Event jentryEvent;
  vector <Sl_Event> EventsInWindow;
  
  clock_t startTime;
  clock_t otherTime;
  double timeRate=0;
  bool timeFlag=true;

  startTime = clock();
  for (Long64_t jentry=0; jentry<maxentry;jentry++) { // Main analysis loop

    inT->GetEntry(jentry); // Get the event from the input tree 
    jentryEvent.jentry=jentry;
    jentryEvent.dchan2 = ddaschannel(*inChannel);//make new one so it doesn't
    bool searching;                              //get overwritten by inT->GetEntry
    if (jentry < maxentry-1)
      searching =true;
    else
      searching=false;
    int countForward=1;//start at the next one
    int countForwardNoDelay=1;//count until end of window ignorin timewindow shift


    //put the jentryEvent in the window of events
    EventsInWindow.push_back(jentryEvent);
    while (searching){
      inT->GetEntry(jentry+countForward);//read event into inChannel
      Double_t TimeDiff =TMath::Abs(jentryEvent.dchan2.time - inChannel->time); // Time between current event and first in window
      
      if ( theInputManager.timeWindowShift == 0 ){ // no shift
	if (TimeDiff < theInputManager.timeWindow){
	  //if still in the window do add to list of events in window 
	  Sl_Event temp;
	  temp.jentry = jentry +countForward;
	  temp.dchan2= ddaschannel(*inChannel);

	  EventsInWindow.push_back(temp);
	  countForward++;
	}else{
	  searching =false;
	  jentry = jentry+countForward-1;
	}
      } else {  // when there is a time window input
	if ( TimeDiff < theInputManager.timeWindow){
	  countForwardNoDelay++;
	}

	if (TimeDiff < theInputManager.timeWindowShift){
	  //while it is less than the shift count 
	  countForward++;
	} else if ( TimeDiff > theInputManager.timeWindowShift && 
		    TimeDiff < theInputManager.timeWindowShift + theInputManager.timeWindow){
	  //push events in the delayed time window
	  Sl_Event temp;
	  temp.jentry = jentry +countForward;
	  temp.dchan2= ddaschannel(*inChannel);
	  
	  EventsInWindow.push_back(temp);
	  countForward++;
	} else{ 
	  searching =false;
	  jentry = jentry+countForwardNoDelay-1;
	}
      }
      
      if (countForward >20){
	cout<<"***Warning run away loop***"<<endl;
	cout<<"***Loop started at "<<jentry<<"***"<<endl;
	cout<<"***Kill loop. Restarting at "<<jentry+1<<"***"<<endl;
	jentry=1+jentry;
	EventsInWindow.clear();
	searching = false;
      }
      
    }//end while
    if (EventsInWindow.size()>=2){
      packEvent(Event,EventsInWindow,theFilter,theInputManager);
      Event->Finalize();
      outT->Fill();
      Event->Clear();//always clear the lenda event
    }
    EventsInWindow.clear();
 
    //Periodic printing
    if (jentry % 10000 <10 && jentry >=20000 && timeFlag){

      timeFlag=false;
      otherTime=clock();
      timeRate = TMath::Abs( double((startTime-otherTime))/double(CLOCKS_PER_SEC));
      
      timeRate = timeRate/jentry;

    }
    //Periodic printing
    if (jentry % 10000 ==0 )
      cout<<right<<"On event "<<setw(9)<<jentry<<" "<<setprecision(2)<<setw(3)<<((double)jentry)/maxentry*100.0<<"% seconds remaining "<<setprecision(4)<<setw(6)<<timeRate*(maxentry-jentry)<<flush<<"\r";

    
  }//End main analysis loop
  

  //Write the tree to file 
  outT->Write();
  //Close the file
  outFile->Close();
  
  cout<<"Number of bad fits "<<theFilter.numOfBadFits<<endl;
  
  cout<<"\n\n**Finished**\n\n";
  
  return  0;
  
}



/*



      if ( TimeDiff < theInputManager.timeWindow ){
	countForwardNoDelay++;//just the count up to the end of the window ignoring the shift
	if ( TimeDiff <theInputManager.timeWindowShift){
	//if still in the window but less than the shift just count up until the shift 
	  countForward++;
	} else if (TimeDiff > theInputManager.timeWindowShift){
	  //if it is less than the window and greater than the shift count AND push the events
	  //this section is where events are pushed when there is no window shift
	  Sl_Event temp;
	  temp.jentry = jentry +countForward;
	  temp.dchan2= ddaschannel(*inChannel);
	  //	mapOfUsedEntries[temp.jentry]=true;
	  EventsInWindow.push_back(temp);
	  countForward++;
	}
      }else if (TimeDiff > theInputManager.timeWindow && TimeDiff< theInputManager.timeWindowShift){
	//just count if you are inbetween end of window and start of delay
	countForward++;
      }else if (TimeDiff>theInputManager.timeWindow && TimeDiff > theInputManager.timeWindowShift&&
		TimeDiff <theInputManager.timeWindow+theInputManager.timeWindowShift){
	//push events when there is a shift and you are in the shifted time window 
	  Sl_Event temp;
	  temp.jentry = jentry +countForward;
	  temp.dchan2= ddaschannel(*inChannel);
	  //	mapOfUsedEntries[temp.jentry]=true;
	  EventsInWindow.push_back(temp);
	  countForward++;
      } else{
	searching =false;
	jentry = jentry+countForward-1;
      }
    


 */

/*
else { //out side of window plus clock tic
	searching =false;// stop looping 
	if (JitteredEventNums.size()==0){
	  jentry =jentry+countForward-1;
	} else {
	  //should go back to first jitter event
	  for (int i=0;i<JitteredEventNums.size();i++){
	    cout<<JitteredEventNums[i]<<endl;
	  }
	  jentry = JitteredEventNums[0]-1;
	  cout<<"in jittered land"<<endl;
	  int t ;cin>>t;
	  
	}
      }

 */

/*
    ///////////////////////////////////////////////////////////////////////////////////////////
    if (CorrelatedEvents[inChannel->chanid]==NULL){
      Sl_Event *e=new Sl_Event();
      e->jentry=jentry;
      e->dchan =  new ddaschannel(*inChannel);//copy the info
      CorrelatedEvents[inChannel->chanid] =e;
    } else {
      if (TMath::Abs(CorrelatedEvents[inChannel->chanid]->dchan->time 
		     -inChannel->time)<10){
	//repeat channel within window throw away
	delete CorrelatedEvents[inChannel->chanid];
	CorrelatedEvents[inChannel->chanid]=NULL;
      }else {
	//update the spot if the current event happened after the stored one
	//ie keep the most recent one
	delete CorrelatedEvents[inChannel->chanid];
	Sl_Event *e=new Sl_Event();
	e->jentry=jentry;
	e->dchan =  new ddaschannel(*inChannel);//copy the info
	CorrelatedEvents[inChannel->chanid] =e;
      }
    }
  
  
    //bar 11 check
    if(CorrelatedEvents[0]!=NULL && CorrelatedEvents[1] !=NULL&&
       TMath::Abs(CorrelatedEvents[0]->dchan->time - CorrelatedEvents[1]->dchan->time)<10){
       for (int i=0;i<2;i++){
	cout<<"I is "<<i<<endl;
	cout<<"Jentry "<<CorrelatedEvents[i]->jentry<<endl;
	cout<<"time is "<<CorrelatedEvents[i]->dchan->time<<endl;
	}
      vector<Sl_Event*>::const_iterator first = CorrelatedEvents.begin() + 0;
      vector<Sl_Event*>::const_iterator last = CorrelatedEvents.begin() + 2;

      vector<Sl_Event*> newVec(first, last);
      packEvent(Event,newVec,theFilter,FL,FG,CFD_delay,CFD_scale_factor);
      outT->Fill();
      Event->Clear();
      delete CorrelatedEvents[0];CorrelatedEvents[0]=NULL;
      delete CorrelatedEvents[1];CorrelatedEvents[1]=NULL;
    }

    //bar 23 check
    if(CorrelatedEvents[2]!=NULL && CorrelatedEvents[3] !=NULL&&
       TMath::Abs(CorrelatedEvents[2]->dchan->time - CorrelatedEvents[3]->dchan->time)<10){
      for (int i=2;i<4;i++){
	cout<<"I is "<<i<<endl;
	cout<<"Jentry "<<CorrelatedEvents[i]->jentry<<endl;
	cout<<"time is "<<CorrelatedEvents[i]->dchan->time<<endl;
	}
      vector<Sl_Event*>::const_iterator first = CorrelatedEvents.begin() + 2;
      vector<Sl_Event*>::const_iterator last = CorrelatedEvents.begin() + 4;

      vector<Sl_Event*> newVec(first, last);
      packEvent(Event,newVec,theFilter,FL,FG,CFD_delay,CFD_scale_factor);
      outT->Fill();
      Event->Clear();
      //remove those events from CorrelatedEvents to prevent over counting
      delete CorrelatedEvents[2];CorrelatedEvents[2]=NULL;
      delete CorrelatedEvents[3];CorrelatedEvents[3]=NULL;

    }
*/
