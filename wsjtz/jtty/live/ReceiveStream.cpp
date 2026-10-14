// Fusion adapter, 2026-10-14. GPL-3.0-or-later; upstream JTTY credits: NOTICE.md.
// Dedicated serialized decoder process; stdin is 12 kHz mono little-endian PCM16.
#include "JttyDecoder.hpp"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtEndian>
#include <iostream>
#include <vector>
#include <algorithm>
#ifdef Q_OS_WIN
#include <fcntl.h>
#include <io.h>
#endif
int main (int argc,char ** argv)
{
  QCoreApplication app {argc,argv};
#ifdef Q_OS_WIN
  _setmode (_fileno (stdin),_O_BINARY);
#endif
  if (argc != 3) return 2;
  bool ok1=false,ok2=false;
  float center=QString::fromLocal8Bit (argv[1]).toFloat (&ok1);
  float tolerance=QString::fromLocal8Bit (argv[2]).toFloat (&ok2);
  if (!ok1 || !ok2 || center<200 || center>2800 || tolerance<1 || tolerance>2600) return 2;
  Jtty::Decoder decoder;
  if (!decoder.valid ()) return 3;
  decoder.begin (1,0,0);
  std::vector<qint16> history;
  qint64 first=0,end=0;
  auto updates = [&] {
    for (auto const& u : decoder.takeUpdates ()) {
      QJsonObject obj;
      obj["id"]=QString::number (u.messageId); obj["text"]=u.text;
      obj["frequency"]=double (u.frequency); obj["snr"]=u.snr;
      obj["terminal"]=int (u.terminal); obj["start"]=u.startSeconds;
      std::cout << QJsonDocument (obj).toJson (QJsonDocument::Compact).constData () << '\n';
    }
    std::cout.flush ();
  };
  char bytes[2048];
  while (std::cin.read (bytes,sizeof bytes) || std::cin.gcount ()) {
    auto count=std::cin.gcount ();
    if (count%2) return 4;
    for (int n=0;n<count;n+=2)
      history.push_back (qFromLittleEndian<qint16> (reinterpret_cast<uchar const *> (bytes+n)));
    end += count/2;
    constexpr qint64 window=Jtty::receiveFrameSamples*5/4;
    while (decoder.nextSearchSample ()+window <= end) {
      auto required=decoder.nextRequiredSample ();
      auto stop=decoder.nextSearchSample ()+window;
      if (required<first || stop>end) return 5;
      auto processed=decoder.process (history.data ()+(required-first),int (stop-required),
                                      required,stop,1,200,2800,center,tolerance);
      if (processed<0) return 6;
      updates ();
      if (!processed) break;
    }
    auto discard=decoder.nextRequiredSample ()-first;
    if (discard>65536 && discard<=qint64 (history.size ())) {
      history.erase (history.begin (),history.begin ()+discard); first+=discard;
    }
    if (history.size ()>180*12000) return 7;
  }
  decoder.end (); updates ();
  return 0;
}
