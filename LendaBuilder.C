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
#include "DDASEvent.h"
#include "Filter.hh"
#include "FileManager.h"
#include "InputManager.hh"
#include "CorrectionManager.hh"
#include <ctime>
#include <iomanip>
#include "Settings.hh"
#include "LendaPacker.hh"

#include "functions.hh"
#define BAD_NUM -10008


using namespace std;


int main(int argc, char **argv){
  

  vector <string> inputs;
  for (int i=1;i<argc;++i){
    inputs.push_back(string(argv[i]));
  }
  if (inputs.size() == 0 ){ // no argumnets display helpful message
    cout<<"Usage: ./LendaBuilder runNumber [options:value]"<<endl;
    return 0;
  }  
  
  InputManager theInputManager; 
  if ( !  theInputManager.loadInputs2(inputs) ){//load options and check everything
    return 0;
  }
  
  Settings *theSettings=new Settings();
  theInputManager.WriteSettings(theSettings);


  
  ////////////////////////////////////////////////////////////////////////////////////
  //load settings
  //Put some settings into local variables.  Most for control options
  Int_t runNum=theInputManager.runNum;
  Int_t numFiles=theInputManager.numFiles;
  Long64_t maxentry=theInputManager.maxEntry;
  Bool_t extFlag=theInputManager.ext_flag;
  Bool_t ext_sigma_flag=theInputManager.ext_sigma_flag;
  Bool_t isOldFormat=theInputManager.isOldFormat;
  
  //Put Gate and timing variables into the Packer ojbect
  LendaPacker *thePacker = new LendaPacker();
  thePacker->SetFilter(theInputManager.FL,theInputManager.FG,theInputManager.d,theInputManager.w);
  thePacker->SetGates(theInputManager.long_gate,theInputManager.short_gate,
		      theInputManager.long_gate2,theInputManager.short_gate2);

  thePacker->SetTraceDelay(theInputManager.traceDelay);

  

  //prepare files and output tree
  ////////////////////////////////////////////////////////////////////////////////////
  FileManager * fileMan = new FileManager(); //Make New File Manager
  fileMan->fileNotes = theInputManager.fileNotes;
  fileMan->timingMode =theInputManager.timingMode;

  //The input trees are put into a TChain
  //Expects there to be many files containing trees named dchan
  TChain * inT;  
  inT= new TChain("dchan");//dchan is name of tree in ddas rootfiles
  if (numFiles == -1 ){
    TString s = fileMan->loadFile(runNum,0); //look for files of format run-####-##.root
    inT->Add(s);
  } else {
    for (Int_t i=0;i<numFiles;i++) {
      TString s = fileMan->loadFile(runNum,i);
	inT->Add(s);
    }
  }

  bool upPack=false;;
  if (upPack){
    UnPackIt(inT);
    return 0;
  }
  
  
  Long64_t nentry=(Long64_t) (inT->GetEntries());
  cout <<"The number of entires is : "<< nentry << endl ;
  

  // Openning output Tree and output file
  ////////////////////////////////////////////////////////////////////////////////////
  TFile *outFile=0;
  TTree  *outT;
  

  //Note FileManager expects fileMan->loadFile before getting outputfile
  if (theInputManager.reMakePulseShape){
    outFile= fileMan->getOutputFile(theInputManager.long_gate,theInputManager.short_gate);
   
  }else if (extFlag == false && ext_sigma_flag==false) //if there are no flags
    outFile = fileMan->getOutputFile();
  else if (extFlag == true && ext_sigma_flag==false){
    outFile = fileMan->getOutputFile(theInputManager.FL,theInputManager.FG,
				     theInputManager.d,theInputManager.w*10);
  } else if (extFlag==false && ext_sigma_flag==true){
    //    sigma=sigma/10;
    //    outFile= fileMan->getOutputFile(sigma*10);
    cout<<"NO SIGMA SUTFF "<<endl;
    return -123;
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
  ddaschannel * inChannel;
  DDASEvent * inEvent;
  if (isOldFormat){
    inChannel = new ddaschannel();
    inT->SetBranchAddress("dchan",&inChannel);
  } else {
    inEvent= new DDASEvent();
    inT->SetBranchAddress("ddasevent",&inEvent);
  }
  
  //Specify the output branch
  LendaEvent* Event = new LendaEvent();
  outT->Branch("Event",&Event);
  //  outT->BranchRef();

  ////////////////////////////////////////////////////////////////////////////////////
  //If not set by InputManager then let the Event chose its defualt 
  if ( theInputManager.GammaPeakTime != -1 ){
    Event->SetGammaPeakTime(theInputManager.GammaPeakTime);
  }
  if (theInputManager.GammaPeakTimeLiqLiq != -1){
    Event->SetGammaPeakTimeLiqLiq(theInputManager.GammaPeakTimeLiqLiq);
  }
  //if there is a shift given then the peak time should be the shift
  ///Add 0.334448 becuase the Event will take the peakSpot and subtract
  //0.33448 to get the shift.  Shift =GPT-0.333488.  Making the time spectrum
  //after the shift for the random runs uniform starting from 0 and going to
  //the timewidnow width
  if (theInputManager.timeWindowShift !=0){
    Event->SetGammaPeakTime(theInputManager.timeWindowShift+0.334448);
    Event->SetGammaPeakTimeLiqLiq(theInputManager.timeWindowShift+0.334448);
  }
  Event->SetLean(theInputManager.lean);// Set lean inside the lendaEvent
  Event->setGainCorrections(0.369254,12.1052,0);//Set channels 0 gain cor
  Event->setGainCorrections(0.246648,14.6706,1);//Set channels 1 gain cor

  ////////////////////////////////////////////////////////////////////////////////////
  
  if(maxentry == -1)
    maxentry=nentry;
  else if (maxentry > nentry)
    maxentry=nentry;

  //non branch timing variables 
     ////////////////////////////////////////////////////////////////////////////////////


  //  Filter theFilter; // Filter object
  ////////////////////////////////////////////////////////////////////////////////////


  //  vector <Sl_Event*> CorrelatedEvents(4,NULL);
  //  map <Long64_t,bool> mapOfUsedEntries;//Used to prevent double counting
  Sl_Event jentryEvent;
  vector <Sl_Event> EventsInWindow;
  
  clock_t startTime;
  clock_t otherTime;
  double timeRate=0;
  bool timeFlag=true;

  startTime = clock();
  for (Long64_t jentry=theInputManager.startEntry; jentry<maxentry;jentry++) { // Main analysis loop

    if (isOldFormat){
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
    } else { // end is old format
      //Building with Pre-Time Correlated DDASEvents
      inT->GetEntry(jentry);
      vector <ddaschannel*> v=inEvent->GetData();

      for (int i=0;i<v.size();i++){
	jentryEvent.dchan2 = ddaschannel((*v[i]));//Copy over the channels
	jentryEvent.jentry = jentry;
	EventsInWindow.push_back(jentryEvent);
      }
          
    }
    if (EventsInWindow.size()>=theInputManager.minMultiplicity){
      packEvent(Event,EventsInWindow,thePacker);
      Event->Finalize();
      outT->Fill();
      Event->Clear();//always clear the lenda event.  Or Else...
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
    if (jentry % 10000 ==0 ){
      cout<<flush<<"\r"<<"                                                                                          "<<"\r";
      cout<<"On Event "<<jentry<<" "<<((double)jentry)/(maxentry-theInputManager.startEntry)*100<<"% minutes remaining "<<(1.0/60)*timeRate*(maxentry-jentry)<<" hours remaining "<<(1.0/3600)*timeRate*(maxentry-jentry);
    }
    //cout<<right<<"On event "<<setw(9)<<jentry<<" "<<setprecision(2)<<setw(3)<<((double)jentry)/maxentry*100.0<<"% seconds remaining "<<setprecision(4)<<setw(6)<<timeRate*(maxentry-jentry)<<flush<<"\r";

    
  }//End main analysis loop
  
  Event->WriteSettings(theSettings);
  //Write the tree to file 
  outT->Write();
  theSettings->Write();
  //Close the file
  outFile->Close();
  

  
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
