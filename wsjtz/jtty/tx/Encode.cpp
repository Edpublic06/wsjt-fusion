#include "Encode.hpp"
#include "widgets/JttyMessages.hpp"
#include <vector>
#include <cmath>
#include <QtEndian>
extern "C" {
  void genjtty_profile_(char*, int const*, int*, int*, int*, int const*, size_t);
  void gen_jttywave_(int*,int*,int*,float*,float*,float*,float*,float*,int*,int*);
}
namespace wsjtz {
JttyEncoded encodeJtty(QString const& text,int frequency) {
  JttyEncoded result;
  if(text.size()>320) {result.error="Please send at most 320 characters per message.";return result;}
  if(frequency<200 || frequency>5000) {result.error="JTTY TX audio frequency must be between 200 and 5000 Hz.";return result;}
  auto prepared=Jtty::prepareTransmitText(text);
  result.text=prepared.text; result.substituted=prepared.substituted;
  if(result.text.trimmed().isEmpty()) {result.error="Enter a message first.";return result;}
  auto lines=result.text.split('\n');
  for(auto const& line:lines) {
    for(int offset=0;offset<line.size();) {
      auto segment=Jtty::nextTransmitTextSegment(line,offset);
      if(segment.text.trimmed().isEmpty()){offset+=segment.length;continue;}
      int tones[944]{}, starts[16]{}, symbols=0;
      for(;;) {
        auto frame=Jtty::transmitFrame(segment.text).toLatin1();
        int const profile=0, finalFlag=(offset+segment.length>=line.size())?1:0;
        genjtty_profile_(frame.data(),&profile,tones,&symbols,starts,&finalFlag,size_t(80));
        if(symbols>0)break;
        if(segment.length<=1){result.error="JTTY text could not be encoded.";result.pcm.clear();return result;}
        segment=Jtty::nextTransmitTextSegment(line,offset,segment.length-1);
      }
      // Same waveform parameters and PCM conversion as mainwindow_jtty.cpp.
      int nsps=4*384,complex=0,count=nsps*symbols;
      float bt=2.f,rate=48000.f,f0=float(frequency);
      if(result.pcm.size()/2+count>60*48000){result.error="Message exceeds the 60-second audio queue. Send a shorter message.";result.pcm.clear();return result;}
      std::vector<float> wave(count);
      gen_jttywave_(tones,&symbols,&nsps,&bt,&rate,&f0,wave.data(),wave.data(),&complex,&count);
      if(count<=0){result.error="JTTY waveform is empty.";result.pcm.clear();return result;}
      for(int i=0;i<count;++i){qint16 sample=qRound(qBound(-32768.f,wave[i]*32767.f,32767.f));result.pcm.append(reinterpret_cast<char const*>(&sample),sizeof(sample));}
      offset+=segment.length;
    }
  }
  return result;
}
}
