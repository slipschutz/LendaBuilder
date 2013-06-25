#include <vector>
#include "TFile.h"
/*
random functions for the builder

 */


struct Sl_Event {
  //4~  ddaschannel *dchan;
  ddaschannel dchan2;
  
  //  Int_t channel;
  // vector <UShort_t> trace;
  Long64_t jentry;
  // Double_t time;
  Double_t softTime;
  // Double_t timelow;
  // Double_t timehigh;
  // Double_t timecfd;
  //  Double_t energy;
};



Bool_t checkChannels(vector <Sl_Event>&EventsInWindow,
		     map <Long64_t,bool>&mapOfUsedEntries){

  cout <<"num events "<<EventsInWindow.size()<<endl;
  vector < vector <bool> > ch;
  for (int i=0;i<16;i++){
    vector <bool> temp;
    ch.push_back(temp);
  }


  
  map <int,Long64_t> entries;
  
  bool duplicateChannelsInWindow=false;
  for (int i=0;i<EventsInWindow.size();i++){
    ch[EventsInWindow[i].dchan2.chanid].push_back(true);
    entries[i]=EventsInWindow[i].jentry;
    
    if (ch[EventsInWindow[i].dchan2.chanid].size() >1)
      duplicateChannelsInWindow=true;
  }

  for (int i=0;i<16;i++)
    cout<<"i is "<<i<<" "<<ch[i].size()<<endl;

  if (duplicateChannelsInWindow)
    return false;

  cout<<"the conditions "<<endl;
  cout<<(ch[0].size()==1 && ch[1].size()==1)<<endl;
  cout<<(ch[2].size()==1 && ch[3].size()==1)<<endl;
  
  cout<<"The entries "<<endl;
  for (int i=0;i<entries.size();i++)
    cout<<"entry i "<<entries[i]<<endl;
  

  //allowed combinations 
  if ( (ch[0].size()==1 && ch[1].size()==1)){
    //currently bar11

  } else if (ch[2].size()==1 && ch[3].size()==1){
    //curently bar 23
    
  } else {
    return false;
  }
}

void pushRollingWindow(vector <Sl_Event> &previousEvents,ddaschannel *dchan,Long64_t jentry){

	Sl_Event e;
	//	e.dchan =dchan;
	e.jentry = jentry;
	previousEvents.push_back(e);

}

void packEvent(LendaEvent *Event,vector <Sl_Event> inEvents,
	       Filter theFilter,Double_t FL, Double_t FG,
	       int CFD_delay,Double_t CFD_scale_factor){
  
  //sort the events by channel
  
  vector <Sl_Event*> events2(16,NULL);
  vector <Sl_Event*> events;
  for (int i=0;i<inEvents.size();i++){
    events2[inEvents[i].dchan2.chanid]=&inEvents[i];
  }
  for (int i=0;i<events2.size();i++){
    if (events2[i] !=NULL )
      events.push_back(events2[i]);
  }


  vector <Double_t> thisEventsFF;
  vector <Double_t> thisEventsCFD;
  for (int i=0;i<events.size();++i){
    Double_t thisEventsIntegral=0; //intialize
    Double_t longGate=0; //intialize
    Double_t shortGate=0; //intialize
    Double_t cubicCFD=0;
    Double_t softwareCFD=0;
    Double_t traceDelay=50;
    Double_t start=0;

    thisEventsFF.clear(); //clear
    thisEventsCFD.clear();//clear
    if ((events[i]->dchan2.trace).size()!=0){ //if this event has a trace calculate filters and such
      theFilter.FastFilter(events[i]->dchan2.trace,thisEventsFF,FL,FG); //run FF algorithim
      thisEventsCFD = theFilter.CFD(thisEventsFF,CFD_delay,CFD_scale_factor); //run CFD algorithim
      
      softwareCFD=theFilter.GetZeroCrossing(thisEventsCFD)-traceDelay; //find zeroCrossig of CFD
      
      cubicCFD = theFilter.GetZeroCubic(thisEventsCFD)-traceDelay;
      
      start = TMath::Floor(softwareCFD)+traceDelay -5; // the start point in the trace for the gates
      thisEventsIntegral = theFilter.getEnergy(events[i]->dchan2.trace);
      // longGate = theFilter.getGate(events[i]->dchan->trace,start,25);
      // shortGate = theFilter.getGate(events[i]->dchan->trace,start,14);
            
    }
    
    Event->pushTrace(events[i]->dchan2.trace);//save the trace for later if its there
    //it is 0 if it isn't
    Event->pushFilter(thisEventsFF); //save filter if it is there
    Event->pushCFD(thisEventsCFD); //save CFD if it is there

    //Push other thing into the event
    Event->pushLongGate(longGate); //longer integration window
    Event->pushShortGate(shortGate);//shorter integration window
    Event->pushChannel(events[i]->dchan2.chanid);//the channel for this pulse
    Event->pushEnergy(thisEventsIntegral);;//push trace energy if there
    Event->pushInternEnergy(events[i]->dchan2.energy);//push internal energy
    Event->pushTime(events[i]->dchan2.time);
    Event->pushSoftTime(events[i]->dchan2.timelow +events[i]->dchan2.timehigh*4294967296.0+softwareCFD);
    Event->pushSoftwareCFD(softwareCFD);
    Event->pushCubicTime(events[i]->dchan2.timelow +events[i]->dchan2.timehigh*4294967296.0+cubicCFD);
    Event->pushInternalCFD((events[i]->dchan2.timecfd)/65536.0);
    Event->pushEntryNum(events[i]->jentry);
  }
  
  //  Event->Finalize(); //Finalize Calculates various parameters and applies corrections
 
 
}
/*











	  //for cable arrangement independance
	  //sort the last size of rolling window evens by channel

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
	    


	  }//end timeDiff if




*/





