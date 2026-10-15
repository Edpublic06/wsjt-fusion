#if defined(__APPLE__) && defined(__aarch64__)
// Fusion ABI adapters, no imported UI. GPL-3.0-or-later.
#include "commons.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>
extern "C" void* fusion_beta2_symbol(char const*,char const*);
namespace {
template<class F> F get(char const* name) {
  auto p=reinterpret_cast<F>(fusion_beta2_symbol(name,nullptr));
  if(!p){std::fprintf(stderr,"Missing Fusion beta2 engine symbol %s\n",name);std::abort();}
  return p;
}
}
extern "C" {
void fusion_hspec_(short* d,int* k,int* utc,int* depth,int* rx,int* tol,bool* msk,bool* train,
 double const* coeff,int* gain,char const* my,char const* his,bool* sh,bool* swl,char const* dir,
 float* green,float* spectrum,int* jh,float* px,float* rms,char* line,size_t a,size_t b,size_t c,size_t e) {
 using F=void(*)(short*,int*,int*,int*,int*,int*,bool*,bool*,double const*,int*,char const*,char const*,bool*,bool*,char const*,float*,float*,int*,float*,float*,char*,size_t,size_t,size_t,size_t);
 get<F>("hspec_")(d,k,utc,depth,rx,tol,msk,train,coeff,gain,my,his,sh,swl,dir,green,spectrum,jh,px,rms,line,a,b,c,e);
}
void fusion_freqcal_(short* d,int* k,int* khz,int* offset,int* tol,char* line,size_t n) {
 using F=void(*)(short*,int*,int*,int*,int*,char*,size_t);
 get<F>("freqcal_")(d,k,khz,offset,tol,line,n);
}
void fusion_fast_decode_(short* d,int* arg,double* period,char* line,char* my,char* his,size_t a,size_t b,size_t c) {
 using F=void(*)(short*,int*,double*,char*,char*,char*,size_t,size_t,size_t);
 get<F>("fast_decode_")(d,arg,period,line,my,his,a,b,c);
}
void fusion_avecho_(short* d,int* dop,int* rit,int* automatic,int*,int* avg,int* qual,float* f1,float* level,float* signal,float* snr,float* freq,float* width,bool* disk,bool* call,char* tx,char* rx,size_t a,size_t b) {
 using F=void(*)(short*,int*,int*,int*,int*,int*,float*,float*,float*,float*,float*,float*,bool*,bool*,char*,char*,size_t,size_t);
 auto common=get<void*>("echocom_");
 std::memcpy(common,&echocom_,sizeof(echocom_));
 get<F>("avecho_")(d,dop,rit,automatic,avg,qual,f1,level,signal,snr,freq,width,disk,call,tx,rx,a,b);
 std::memcpy(&echocom_,common,sizeof(echocom_));
}
void fusion_save_echo_params_(int* total,int* dop,int* rit,float* f1,float* spread,short* d,int* direction) {
 using F=void(*)(int*,int*,int*,float*,float*,int*,int*,short*,int*);
 int spacing=0,tones[6]={};
 get<F>("save_echo_params_")(total,dop,rit,f1,spread,&spacing,tones,d,direction);
}
}

#endif
