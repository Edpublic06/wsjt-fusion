// End-to-end decoder ABI regression. No radio/audio device or user settings.
#include "commons.h"
#include <QCoreApplication>
#include <QSharedMemory>
#include <QProcess>
#include <QTemporaryDir>
#include <QElapsedTimer>
#include <QThread>
#include <QUuid>
#include <cstring>
#include <iostream>
#include <vector>
extern "C" {
void genft8_(char*,int*,int*,char*,char*,int*,size_t,size_t);
void gen_ft8wave_(int*,int*,int*,float*,float*,float*,void*,float*,int*,int*);
void genft4_(char*,int*,char*,char*,int*,size_t,size_t);
void gen_ft4wave_(int*,int*,int*,float*,float*,void*,float*,int*,int*);
}
int main(int argc,char**argv) {
 QCoreApplication app(argc,argv);if(argc!=3)return 2;
 QTemporaryDir temp;
 for(int variant: {8,108,5}) {
  int mode=variant%100;
  QSharedMemory mem("fusion-beta3-test-"+QUuid::createUuid().toString());
  if(!mem.create(sizeof(dec_data_t))){std::cerr<<mem.errorString().toStdString();return 3;}
  auto* data=static_cast<dec_data_t*>(mem.data());std::memset(data,0,sizeof(*data));
  char message[37],sent[37],bits[77];std::memset(message,' ',37);std::memcpy(message,"CQ K1JT FN20",12);
  int tones[105]={},i3=-1,n3=-1,check=0;
  int symbols=mode==8?79:105,nsps=mode==8?1920:576,nwave=symbols*nsps,complex=0;
  float bt=mode==8?2.f:1.f,rate=12000.f,freq=1500.f;
  std::vector<float> wave(nwave),dummy(2*nwave);
  if(mode==8){genft8_(message,&i3,&n3,sent,bits,tones,37,37);gen_ft8wave_(tones,&symbols,&nsps,&bt,&rate,&freq,dummy.data(),wave.data(),&complex,&nwave);}
  else {genft4_(message,&check,sent,bits,tones,37,37);gen_ft4wave_(tones,&symbols,&nsps,&rate,&freq,dummy.data(),wave.data(),&complex,&nwave);}
  for(int i=0;i<nwave && i+6000<180000;++i)data->d2[i+6000]=short(wave[i]*1500);
  auto& p=data->params;
  p.nmode=mode;p.ntxmode=mode;p.ntrperiod=mode==8?15:7;p.nutc=123000;
  p.kin=mode==8?180000:90000;p.npts8=180000;p.nzhsym=50;p.newdat=true;p.ndiskdat=true;
  p.nfa=200;p.nfb=3000;p.nfqso=1500;p.nftx=1500;p.ntol=100;p.ndepth=3;
  p.lmultift8=(variant==108);p.ndecoderstart=4;
  p.ncandthin=100;p.nft8cycles=1;p.nft8rxfsens=3;p.nft4depth=3;p.nmt=2;p.napwid=50;
  std::memset(p.mycall,' ',12);std::memcpy(p.mycall,"F6FDY",5);std::memset(p.hiscall,' ',12);
  std::memset(p.mygrid,' ',6);std::memcpy(p.mygrid,"JN33",4);std::memset(p.hisgrid,' ',6);
  std::memcpy(p.mybcall,p.mycall,12);std::memcpy(p.hisbcall,p.hiscall,12);
  QProcess worker;worker.setWorkingDirectory(temp.path());
  worker.start(QString::fromLocal8Bit(argv[1]),{"-s",mem.key(),"-e",QString::fromLocal8Bit(argv[2]),"-a",temp.path(),"-t",temp.path()});
  if(!worker.waitForStarted())return 4;
  mem.lock();data->ipc[1]=1;mem.unlock();
  QByteArray output;QElapsedTimer timer;timer.start();
  while(timer.elapsed()<30000 && !output.contains("<DecodeFinished>")) {
   worker.waitForReadyRead(100);output+=worker.readAllStandardOutput();
   if(worker.state()==QProcess::NotRunning)break;
  }
  worker.kill();worker.waitForFinished();
  if(!output.contains("CQ K1JT FN20") || !output.contains("<DecodeFinished>")) {
   std::cerr<<"Mode "<<mode<<" failed: "<<output.constData()<<worker.readAllStandardError().constData();return 5;
  }
  std::cout<<"PASS Fusion shared-memory -> beta2 engine -> legacy decode output, mode "<<variant<<"\n";
 }
}
