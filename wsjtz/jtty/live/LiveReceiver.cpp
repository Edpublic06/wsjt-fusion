// Fusion live receive adapter, 2026-10-14. GPL-3.0-or-later.
#include "LiveReceiver.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <cmath>
#include "Radio.hpp"
#include "wsjtz/jtty/display/Sender.hpp"
wsjtz::LiveReceiver::LiveReceiver (QObject * parent) : QObject {parent}
{
  connect (&process_,&QProcess::readyReadStandardOutput,this,[this] {
    pending_+=process_.readAllStandardOutput ();
    while (pending_.contains ('\n')) {
      auto end=pending_.indexOf ('\n'); auto line=pending_.left (end); pending_.remove (0,end+1);
      auto object=QJsonDocument::fromJson (line).object ();
      if (object.isEmpty ()) continue;
      auto id=object["id"].toString ().toLongLong ();
      double const frequency=object["frequency"].toDouble ();
      auto const sender=wsjtz::jttySender(object["text"].toString(),[](QString const& call){return Radio::is_callsign(call);});
      average_.observe(session_,id,sender,object["snr"].toInt());
      snrHistory_.observe (object["text"].toString (),object["snr"].toInt (),
        std::abs(frequency-center_) <= tolerance_);
      lines_.put(session_,id,QString ("%1 Hz  %2 dB  %3").arg (frequency,0,'f',0)
        .arg (object["snr"].toInt ()).arg (object["text"].toString ()),frequency);
      if(decodedMessage)decodedMessage(object["text"].toString());
      publish();
    }
  });
  connect (&process_,&QProcess::errorOccurred,this,[this] (QProcess::ProcessError) {
    if (!stopping_ && statusChanged) statusChanged ("JTTY receiver: " + process_.errorString ());
  });
  connect (&process_,QOverload<int,QProcess::ExitStatus>::of (&QProcess::finished),
    this,[this] (int code,QProcess::ExitStatus state) {
      if (!stopping_ && statusChanged && (code || state!=QProcess::NormalExit))
        statusChanged ("JTTY receive process stopped. Toggle Monitor to restart reception.");
    });
}
wsjtz::LiveReceiver::~LiveReceiver () { stop (); }
void wsjtz::LiveReceiver::start (int center,int tolerance)
{
  stop (); pending_.clear (); ++session_;
  setSelection(center,tolerance);
  process_.start (QDir {QCoreApplication::applicationDirPath ()}.filePath ("fusion-jtty-rx"),
                  {QString::number (center),QString::number (tolerance)});
}
void wsjtz::LiveReceiver::stop ()
{
  stopping_=true;
  if (process_.state ()!=QProcess::NotRunning) { process_.kill (); process_.waitForFinished (1000); }
  stopping_=false;
}
void wsjtz::LiveReceiver::append (QByteArray const& pcm)
{
  if (process_.state ()!=QProcess::Running) return;
  if (process_.bytesToWrite ()>3*12000*2) {
    stop ();
    if (statusChanged) statusChanged ("JTTY audio overrun: reception stopped. Toggle Monitor to restart.");
    return;
  }
  process_.write (pcm);
}

void wsjtz::LiveReceiver::appendTransmit (QString const& text,int frequency)
{
  lines_.put(++session_,0,QString ("%1 Hz  TX  %2").arg (frequency).arg (text),frequency,true);
  publish();
}
void wsjtz::LiveReceiver::setSelection(int center,int tolerance)
{
  center_=center; tolerance_=tolerance;
  publish();
}
void wsjtz::LiveReceiver::publish()
{
  if(textChanged)textChanged(lines_.all(),lines_.within(center_-tolerance_,center_+tolerance_));
}

void wsjtz::LiveReceiver::selectPeer(QString const& call,QString const& line)
{
  average_.select(call);
  QPair<quint64,qint64> key;
  if(lines_.keyForText(line,key))average_.seed(key);
}
