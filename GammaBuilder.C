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
#include "Filter.hh"
#include "FileManager.h"
#include "InputManager.hh"
#include "CorrectionManager.hh"



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
  if ( !  theInputManager.loadInputs(inputs) ){
    return 0;
  }
  
  ////////////////////////////////////////////////////////////////////////////////////

  //load correcctions and settings
  
  Double_t sigma=theInputManager.sigma; // the sigma used in the fitting option

  Int_t runNum=theInputManager.runNum;
  Int_t numFiles=theInputManager.numFiles;

  Long64_t maxentry=theInputManager.maxEntry;

  Bool_t makeTraces=theInputManager.makeTraces;

  Bool_t extFlag=theInputManager.ext_flag;
  Bool_t ext_sigma_flag=theInputManager.ext_sigma_flag;

  //defualt Filter settings see pixie manual
  Double_t FL=theInputManager.FL;
  Double_t FG=theInputManager.FG;
  int CFD_delay=theInputManager.d; //in clock ticks
  Double_t CFD_scale_factor =theInputManager.w;

  Int_t traceDelay=50;//In clock ticks

  

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
  
  inT->SetMakeClass(1);//This does something
  
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
  
  Int_t numOfChannels=4;  
  // set input tree branvh variables and addresses
  ////////////////////////////////////////////////////////////////////////////////////
  Int_t chanid;
  Int_t slotid;
  vector<UShort_t> trace;
  UInt_t fUniqueID;
  UInt_t energy;
  Double_t time ; 
  UInt_t timelow; // this used to be usgined long
  UInt_t timehigh; // this used to be usgined long
  UInt_t timecfd ; 
  LendaEvent* Event = new LendaEvent();
  
  //In put tree branches    
  inT->SetBranchAddress("chanid", &chanid);
  inT->SetBranchAddress("fUniqueID", &fUniqueID);
  inT->SetBranchAddress("energy", &energy);
  inT->SetBranchAddress("timelow", &timelow);
  inT->SetBranchAddress("timehigh", &timehigh);
  inT->SetBranchAddress("trace", &trace);
  inT->SetBranchAddress("timecfd", &timecfd);
  inT->SetBranchAddress("slotid", &slotid);
  inT->SetBranchAddress("time", &time);
  
  
  //Specify the output branch
  outT->Branch("Event",&Event);
  //  outT->BranchRef();

  ////////////////////////////////////////////////////////////////////////////////////
  vector <Sl_Event> previousEvents;
  Double_t sizeOfRollingWindow=2;  //Require that a lenda bar fired in both PMTS 
  
  ////////////////////////////////////////////////////////////////////////////////////
  
  if(maxentry == -1)
    maxentry=nentry;
  else if (maxentry > nentry)
    maxentry=nentry;

  //non branch timing variables 
  ////////////////////////////////////////////////////////////////////////////////////
  Double_t softwareCFD;
  Filter theFilter; // Filter object
  ////////////////////////////////////////////////////////////////////////////////////
  
  

  for (Long64_t jentry=0; jentry<maxentry;jentry++) { // Main analysis loop
    
    inT->GetEntry(jentry); // Get the event from the input tree 
    ///////////////////////////////////////////////////////////////////////////////////////////

    //    cout<<"jentry is "<<jentry<<endl;
    // cout<<"time low is "<<timelow<<endl;
    ///int tt ; cin>>tt;

    if ( previousEvents.size() >= sizeOfRollingWindow ) {
      if ( checkChannels(previousEvents) )//prelinary check to see if there are 3 distinict channels in set
	{ 
	  
	  //for cable arrangement independance
	  //sort the last size of rolling window evens by channel
	  vector <Sl_Event*> events_extra(20,(Sl_Event*)0);
	  vector <Sl_Event*> events;
	  Double_t start;
	  Double_t timeDiff;
	  
	  for (int i=0;i<previousEvents.size();++i){
	    events_extra[previousEvents[i].channel]=&(previousEvents[i]);
	  }
	  for (int i=0;i<events_extra.size();++i){
	    if (events_extra[i] != 0 ){
	      events.push_back(events_extra[i]);
	    }
	  }

	  
	  //timeDiff = 0.5*(events[0]->time + events[1]->time - events[2]->time-events[3]->time);
	  timeDiff = (events[1]->time -events[0]->time);	  
	  //timeDiff = 0.5*(events[0]->time + events[1]->time) - events[2]->time;
	  if (TMath::Abs(timeDiff) <10){
	    ///This is now a Good event
	    //Run filters and such on these events 
	    vector <Double_t> thisEventsFF;
	    vector <Double_t> thisEventsCFD;
	    
	    for (int i=0;i<events.size();++i){
	      Double_t thisEventsIntegral=0; //intialize
	      Double_t longGate=0; //intialize
	      Double_t shortGate=0; //intialize
	      thisEventsFF.clear(); //clear
	      thisEventsCFD.clear();//clear
	      if ((events[i]->trace).size()!=0){ //if this event has a trace calculate filters and such
		theFilter.FastFilter(events[i]->trace,thisEventsFF,FL,FG); //run FF algorithim
		thisEventsCFD = theFilter.CFD(thisEventsFF,CFD_delay,CFD_scale_factor); //run CFD algorithim
		softwareCFD = theFilter.GetZeroCrossing(thisEventsCFD)-traceDelay; //find zeroCrossig of CFD


		start = TMath::Floor(softwareCFD) -5; // the start point in the trace for the gates
		thisEventsIntegral = theFilter.getEnergy(events[i]->trace);
		longGate = theFilter.getGate(events[i]->trace,start,25);
		shortGate = theFilter.getGate(events[i]->trace,start,14);
		
		events[i]->energy = thisEventsIntegral; // Over write the energy in this event with the
		                                        // one calculated from the trace
		
		events[i]->softTime = events[i]->timelow +events[i]->timehigh*4294967296.0 + softwareCFD; 

	      }
	      Event->pushTrace(events[i]->trace);//save the trace for later if its there
                                                 //it is 0 if it isn't
	      Event->pushFilter(thisEventsFF); //save filter if it is there
	      Event->pushCFD(thisEventsCFD); //save CFD if it is there

	      //Push other thing into the event
	      Event->pushLongGate(longGate); //longer integration window
	      Event->pushShortGate(shortGate);//shorter integration window
	      Event->pushChannel(events[i]->channel);//the channel for this pulse
	      Event->pushEnergy(events[i]->energy);
	      Event->pushTime(events[i]->time);
	      Event->pushSoftTime(events[i]->softTime);
	      Event->pushSoftwareCFD(softwareCFD);
	      Event->pushInternalCFD((events[i]->timecfd)/65536.0);
	      Event->pushEntryNum(events[i]->jentry);
	    }

	    Event->Finalize(); //Finalize Calculates various parameters and applies corrections
	    outT->Fill();     //Fill the tree
	    Event->Clear();  //Always clear events. if you don't you are pushing events on top of other events
	  }//end timeDiff if
	}//end checkChannels if
    } // end rolling window section
    
    //Push this event (the jentry one in the tree) into the list of 
    //previous events.
    pushRollingWindow(previousEvents,sizeOfRollingWindow,
		      time,timelow,timehigh,timecfd,chanid,trace,jentry,energy);
    
    //Periodic printing
    if (jentry % 10000 == 0 )
      cout<<"On event "<<jentry<<endl;
    
}//End main analysis loop
  

//Write the tree to file 
outT->Write();
//Close the file
outFile->Close();

cout<<"Number of bad fits "<<theFilter.numOfBadFits<<endl;

cout<<"\n\n**Finished**\n\n";

return  0;

}

