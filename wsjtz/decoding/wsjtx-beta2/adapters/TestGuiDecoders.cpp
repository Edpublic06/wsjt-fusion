// Offline smoke of the actual GUI-to-upstream ABI, no radio or audio devices.
#include "commons.h"
#include <vector>
#include <cstring>
#include <cmath>
#include <iostream>
decltype(echocom_) echocom_;
extern "C" {
void fusion_freqcal_(short*,int*,int*,int*,int*,char*,size_t);
void fusion_hspec_(short*,int*,int*,int*,int*,int*,bool*,bool*,double const*,int*,char const*,char const*,bool*,bool*,char const*,float*,float*,int*,float*,float*,char*,size_t,size_t,size_t,size_t);
void fusion_avecho_(short*,int*,int*,int*,int*,int*,int*,float*,float*,float*,float*,float*,float*,bool*,bool*,char*,char*,size_t,size_t);
void fusion_save_echo_params_(int*,int*,int*,float*,float*,short*,int*);
}
int main() {
 std::vector<short> audio(120*12000,0);char line[80]={},call[12];std::memset(call,' ',12);
 int k=512,khz=14090,offset=1500,tol=100,utc=0,depth=3015,gain=0,jh=0;
 bool yes=true,no=false;double coeff[5]={};float green[703]={},spectrum[703*64]={},px=0,rms=0;
 fusion_freqcal_(audio.data(),&k,&khz,&offset,&tol,line,80);
 fusion_hspec_(audio.data(),&k,&utc,&depth,&offset,&tol,&yes,&no,coeff,&gain,call,call,&no,&no,".",green,spectrum,&jh,&px,&rms,line,12,12,1,80);
 int dop=0,rit=0,automatic=0,df=0,avg=1,qual=0;float f=1500,level=0,signal=0,snr=0,freq=0,width=0;
 echocom_.nclearave=1;char tx[6]={'F','6','F','D','Y',' '},rx[6]={};
 fusion_avecho_(audio.data(),&dop,&rit,&automatic,&df,&avg,&qual,&f,&level,&signal,&snr,&freq,&width,&no,&no,tx,rx,6,6);
 if(!std::isfinite(level))return 2;
 int total=123,dir=1;dop=42;rit=5;width=2;
 fusion_save_echo_params_(&total,&dop,&rit,&f,&width,audio.data(),&dir);
 total=0;dop=0;rit=0;dir=-1;
 fusion_save_echo_params_(&total,&dop,&rit,&f,&width,audio.data(),&dir);
 if(total!=123 || dop!=42 || rit!=5)return 3;
 std::cout<<"PASS beta2 MSK spectrum, FreqCal, Echo silence and metadata ABI\n";
}
