// Personal WSJT-Z regression, 2026-10-13. GPL-3.0-or-later.
#include "CqReplyQueue.hpp"
#include "QueuedCallBudget.hpp"
#include "AutomationSettings.hpp"
#include <QTemporaryDir>
#include <cstdlib>
#include <iostream>
void check (bool value) { if (!value) std::exit (1); }
wsjtz::CqReply reply (QString call,int signal,int distance,bool psk=false) {
  wsjtz::CqReply r; r.call=call;r.decode=call;r.signal=signal;r.distance=distance;r.psk=psk;return r;
}
int main () {
  QTemporaryDir temporary;
  check (temporary.isValid ());
  QSettings settings {temporary.filePath ("compatibility.ini"),QSettings::IniFormat};
  settings.beginGroup ("Configuration");
  settings.setValue ("wd_FT8",2.0);
  settings.setValue ("wd_Timer",true);
  settings.setValue ("OtherMod/Example",QStringList {"A","B"});
  auto old_keys = settings.allKeys ();
  check (!wsjtz::readCallerPreference (settings,false) && wsjtz::readWdCounter (settings)==0);
  wsjtz::writeAutomationSettings (settings,true,3);
  settings.sync ();
  check (settings.status ()==QSettings::NoError);
  QSettings restored {settings.fileName (),QSettings::IniFormat};
  restored.beginGroup ("Configuration");
  check (wsjtz::readCallerPreference (restored,false) && wsjtz::readWdCounter (restored)==3);
  check (restored.value ("wd_FT8").toDouble ()==2.0 && restored.value ("wd_Timer").toBool ());
  check (restored.value ("OtherMod/Example").toStringList ()==QStringList ({"A","B"}));
  check (restored.allKeys ().size ()==old_keys.size ()+2);
  wsjtz::QueuedCallBudget budget;
  budget.begin ("A");
  budget.sent ("OTHER");check (!budget.exhausted ("A",1));
  budget.sent ("A");check (budget.exhausted ("A",1));
  check (!budget.exhausted ("A",0)); // unspecified: WD Timer
  check (!budget.exhausted ("A",2));
  budget.sent ("A");check (budget.exhausted ("A",2));
  budget.reply ("OTHER");check (budget.exhausted ("A",2));
  budget.reply ("A");check (!budget.exhausted ("A",1));
  budget.begin ("B");check (!budget.owns ("A") && !budget.exhausted ("B",1));
  wsjtz::CqReplyQueue q;
  q.attach_current ("CURRENT");
  q.add (reply ("NEW",-10,100));q.add (reply ("NEW",-8,110));
  q.add (reply ("CURRENT",-1,100));
  check (q.active ()=="CURRENT" && q.calls ().size ()==1);
  q.complete ();check (q.take (0,false).call=="NEW");q.complete ();
  check (q.empty () && !q.session ());
  q.context ("FT8/20m");
  q.add (reply ("A",-10,100));q.add (reply ("B",-5,200));q.add (reply ("C",-5,300));
  q.add (reply ("B",-4,200));
  check (q.calls ().size ()==3);
  check (q.take (1,false).call=="B"); // priority, not raw decode order
  q.add (reply ("B",-1,200));check (q.calls ().size ()==2); // active duplicate
  q.complete ();check (q.take (1,false).call=="C");
  q.complete ();check (q.take (1,false).call=="A");
  q.complete ();check (q.empty () && !q.session ()); // exhaustion
  q.add (reply ("A",-10,100));q.add (reply ("B",-10,100));
  check (q.take (0,false).call=="A");q.complete ();check (q.take (0,false).call=="B");
  q.clear (); // manual selection must cancel both pending and active state
  check (q.empty () && q.active ().isEmpty () && !q.session ());
  q.add (reply ("A",-1,100));q.add (reply ("B",-20,300));
  check (q.take (2,false).call=="B");
  q.context ("FT4/20m");check (q.empty () && q.active ().isEmpty ());
  q.add (reply ("A",-1,100));q.add (reply ("B",-20,100,true));
  check (q.take (1,true).call=="B");
  q.clear ();
  q.add (reply ("A",-1,100));q.add (reply ("B",-20,100));
  q.highlight ("B",3);check (q.take (1,false).call=="B");
  std::cout << "PASS: CQ queue priorities, FIFO ties, deduplication, completion and manual/context cancellation\n";
}
