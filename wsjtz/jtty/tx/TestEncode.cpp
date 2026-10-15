// Offline TX -> receive-worker loopback. Never opens an audio device or radio.
#include "Encode.hpp"
#include "widgets/JttyMessages.hpp"
#include <QCoreApplication>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <iostream>
int main(int argc,char**argv) {
  QCoreApplication app(argc,argv);if(argc!=2)return 2;
  if(wsjtz::encodeJtty("",1500).error.isEmpty())return 3;
  if(wsjtz::encodeJtty("CQ",100).error.isEmpty())return 4;
  if(wsjtz::encodeJtty(QString(321,'A'),1500).error.isEmpty())return 5;
  auto waveform=wsjtz::encodeJtty("CQ F6FDY JN33 CQ",1500);
  if(!waveform.error.isEmpty() || waveform.pcm.isEmpty() || waveform.pcm.size()%2)return 6;
  // 48 kHz output is in the receiver's 0..3 kHz band: sample at 12 kHz.
  QByteArray rx(24000,'\0');
  for(int i=0;i+7<waveform.pcm.size();i+=8)rx.append(waveform.pcm.mid(i,2));
  rx.append(QByteArray(12000*2*4,'\0'));
  QProcess worker;worker.start(QString::fromLocal8Bit(argv[1]),{"1500","200"});
  if(!worker.waitForStarted())return 7;
  worker.write(rx);worker.closeWriteChannel();
  if(!worker.waitForFinished(60000)){worker.kill();return 8;}
  bool found=false;
  for(auto const& line:worker.readAllStandardOutput().split('\n')) {
    auto obj=QJsonDocument::fromJson(line).object();
    if(obj["text"].toString()==waveform.text)found=true;
  }
  if(!found || worker.exitCode()!=0){std::cerr<<"TX loopback failed\n";return 9;}
  bool attempted=false;
  auto segments=Jtty::encodeTransmitLine(QString(100,'Z'),[&](QString const&){attempted=true;return false;});
  if(!attempted || segments)return 10;
  int count=0;
  auto accepted=Jtty::encodeTransmitLine("CQ F6FDY JN33 CQ",[&](QString const&){++count;return true;});
  if(!accepted || count!=1)return 11;
  std::cout<<"PASS JTTY TX loopback, bounds and beta2 segment refusal\n";
}
