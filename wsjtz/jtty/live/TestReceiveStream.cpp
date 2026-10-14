// Fusion regression, 2026-10-14. GPL-3.0-or-later.
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <iostream>
int main (int argc,char ** argv)
{
  QCoreApplication app {argc,argv};
  if (argc!=3) return 2;
  QFile wav {QString::fromLocal8Bit (argv[2])};
  if (!wav.open (QIODevice::ReadOnly)) return 3;
  auto data=wav.readAll ().mid (44);
  QProcess receiver;
  receiver.start (QString::fromLocal8Bit (argv[1]),{"1500","1300"});
  if (!receiver.waitForStarted ()) return 4;
  for (int i=0;i<data.size ();i+=700) receiver.write (data.mid (i,700));
  receiver.closeWriteChannel ();
  if (!receiver.waitForFinished (60000)) { receiver.kill (); return 5; }
  bool found=false,growing=false;
  for (auto const& line : receiver.readAllStandardOutput ().split ('\n')) {
    auto obj=QJsonDocument::fromJson (line).object ();
    auto text=obj["text"].toString ();
    if (text=="RAN ALL NIGHT ON BAND NOISE - NO FALSE DECODES!") found=true;
    if (text.startsWith ("RAN ALL") && text.size ()<35) growing=true;
  }
  if (receiver.exitCode () || receiver.exitStatus ()!=QProcess::NormalExit || !found || !growing) {
    std::cerr<<receiver.readAllStandardError ().constData (); return 6;
  }
  std::cout<<"PASS progressive JTTY text from streamed PCM\n";
}
