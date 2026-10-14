// Fusion regression, 2026-10-14. GPL-3.0-or-later.
#include "Detector/Detector.hpp"
#include "commons.h"
#include <QCoreApplication>
#include <iostream>
#include <vector>
#include <cstdlib>
dec_data_t dec_data;
static void require (bool ok,char const * message) { if (!ok) { std::cerr<<message<<'\n'; std::exit (1); } }
int main (int argc,char ** argv)
{
  QCoreApplication app {argc,argv};
  Detector detector {12000,0.001,4}; // Deliberately tiny timed period must not truncate continuous audio.
  require (detector.initialize (QIODevice::WriteOnly,AudioDevice::Mono),"initialize");
  detector.setJttyReceive (true,7);
  qint64 samples=0; int timed=0; quint64 lastSession=0;
  QObject::connect (&detector,&Detector::jttyPcm,[&] (quint64 session,QByteArray pcm) {
    samples+=pcm.size ()/2; lastSession=session;
  });
  QObject::connect (&detector,&Detector::framesWritten,[&] (qint64) { ++timed; });
  std::vector<short> input (48000*4,100);
  // Irregular write partitions must conserve every complete downsampled block.
  size_t offset=0;
  while (offset<input.size ()) {
    auto count=qMin<size_t> (386,input.size ()-offset);
    require (detector.write (reinterpret_cast<char const *> (input.data ()+offset),count*2)==qint64(count*2),"write");
    offset+=count;
  }
  require (samples==qint64(input.size ()/2048*512),"continuous sample count");
  require (timed==0 && lastSession==7,"continuous audio leaked into timed decoder");
  detector.setJttyReceive (true,8); samples=0;
  detector.write (reinterpret_cast<char const *> (input.data ()),2048*2);
  require (samples==512 && lastSession==8,"new session retained partial old audio");
  detector.setTRPeriod (15); detector.setJttyReceive (false,9); detector.setBlockSize (512);
  samples=0;
  detector.write (reinterpret_cast<char const *> (input.data ()),4096*2);
  require (samples==0 && timed>0,"timed decoder did not resume after leaving JTTY");
  std::cout<<"PASS continuous partitions, session reset and timed-mode return\n";
}
