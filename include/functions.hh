#include <vector>
#include "TFile.h"
/*
random functions for the builder

 */


struct Sl_Event {
  Int_t channel;
  vector <UShort_t> trace;
  Long64_t jentry;
  Double_t time;
  Double_t softTime;
  Double_t timelow;
  Double_t timehigh;
  Double_t timecfd;
  Double_t energy;
};



Bool_t checkChannels(vector <Sl_Event> &in){

  vector <Bool_t> ch(16,false);  //to make this work with different cable arrangements

  for (int i=0;i<in.size();i++){
      ch[in[i].channel]=true;
  }
  // if it was a good event there should be 4 trues
  //from 4 different channels
  int count=0;
  int liq_scint_count=0; //there should be no  liq scint in this setup
  // at the moment they are pluged into 8 and 9


  for (int i=0;i <ch.size();i++){
    if (ch[i]==true){
      count++;

      if (i==8 || i ==9 ){
        liq_scint_count++;
      }
    }
  }
  //  if (count == 4 && liq_scint_count==0 )
  if ((ch[0] && ch[1]) || (ch[2] &&ch[3]))
    return true;
  else
    return false;

}

void pushRollingWindow(vector <Sl_Event> &previousEvents,Double_t &sizeOfRollingWindow,
		       Double_t &time,Double_t timelow,Double_t timehigh,Double_t timecfd,Int_t &chanid,vector <UShort_t>& trace,Long64_t &jentry,
		       Double_t energy){


    //Keep the previous event info for correlating      
    if (previousEvents.size() < sizeOfRollingWindow  ) 
      {
	Sl_Event e;
	e.channel=chanid;
	e.trace=trace;
	e.jentry=jentry;
	e.time =time;
	e.energy=energy;
	e.timelow=timelow;
	e.timehigh=timehigh;
	e.timecfd=timecfd;
	previousEvents.push_back(e);
      }
    else if (previousEvents.size() >= sizeOfRollingWindow )
      {
	//So starting on the size+1 element 
	previousEvents.erase(previousEvents.begin(),previousEvents.begin() + 1);
	Sl_Event e;
	e.channel=chanid;
	e.trace = trace;
	e.jentry =jentry;
	e.time=time;
	e.energy=energy;
	e.timelow=timelow;
	e.timehigh=timehigh;
	e.timecfd=timecfd;
	previousEvents.push_back(e);	  
      }


}

