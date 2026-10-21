// Personal WSJT-Z regression, 2026-10-13. GPL-3.0-or-later.
#include "CqReplyQueue.hpp"
#include "QueuedCallBudget.hpp"
#include "AutomationSettings.hpp"
#include <QTemporaryDir>
#include <cstdlib>
#include <iostream>
void checkAt (bool value,int line) { if (!value) { std::cerr << "Failure at line " << line << "\n"; std::exit (1); } }
#define check(value) checkAt((value),__LINE__)
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
  q.complete ();check (q.take (1,false).call=="A"); // strict FIFO after first selection
  q.complete ();check (q.take (1,false).call=="C");
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
  q.attach_current ("CURRENT");
  q.add (reply ("FIRST",-20,10));q.add (reply ("LAST",5,900,true));
  q.add (reply ("FIRST",-15,20));q.highlight ("LAST",9);
  q.complete ();check (q.take (2,true).call=="FIRST");
  q.complete ();check (q.take (1,true).call=="LAST");
  q.clear ();
  q.add (reply ("A",-1,100));q.add (reply ("B",-20,100));
  q.highlight ("B",3);check (q.take (1,false).call=="B");
  q.clear ();
  auto known=reply ("KNOWN",-5,100);
  auto fresh=reply ("NEW",-5,100);fresh.newCall=true;
  q.add (known);q.add (fresh);
  check (q.take (1,false).call=="NEW");
  q.complete ();check (q.take (1,false).call=="KNOWN");
  q.clear ();known.signal=-4;q.add (fresh);q.add (known);
  check (q.take (1,false).call=="KNOWN"); // stronger signal still wins
  q.clear ();q.attach_current ("CURRENT");known.signal=-5;
  q.add (known);q.add (fresh);q.complete ();
  check (q.take (1,false).call=="KNOWN"); // waiting-list FIFO is preserved
  q.clear ();known.distance=100;fresh.distance=100;q.add (known);q.add (fresh);
  check (q.take (2,false).call=="NEW");
  q.clear ();known.distance=0;fresh.distance=0;q.add (known);q.add (fresh);
  check (q.take (2,false).call=="NEW"); // both distances unknown
  q.clear ();known.distance=200;q.add (fresh);q.add (known);
  check (q.take (2,false).call=="KNOWN"); // known distance wins over unknown
  q.clear ();q.attach_current ("CURRENT");
  q.add (reply ("A",-1,100));q.add (reply ("B",-10,200));q.add (reply ("C",-20,300));
  check (q.promote ("C"));
  check (q.calls () == (QStringList{} << "C" << "A" << "B"));
  check (q.active () == "CURRENT"); // no interruption of the current QSO
  q.add (reply ("C",-15,300)); // refreshed decode keeps its promoted position
  check (!q.promote ("MISSING"));
  q.complete ();check (q.take (1,false).call == "C");
  q.complete ();check (q.take (1,false).call == "A");
  q.complete ();check (q.take (1,false).call == "B");
  q.clear ();q.add (reply ("A",5,100));q.add (reply ("B",-20,200));
  check (q.promote ("B"));check (q.promote ("B"));
  check (q.take (1,false).call == "B"); // overrides initial priority too
  q.clear ();q.attach_current ("CURRENT");
  auto deferred=reply ("LATE",-18,100);
  deferred.deferredSignoff=true;deferred.sentReport="-16";deferred.receivedReport="-22";
  deferred.grid="AA00";deferred.started=QDateTime::currentDateTimeUtc ();
  q.add (reply ("FIRST",-10,200));q.add (deferred);
  q.add (reply ("LATE",-15,100)); // ordinary decode must not destroy signoff metadata
  check (q.active ()=="CURRENT" && q.calls ()==(QStringList{} << "FIRST" << "LATE"));
  q.complete ();auto finishing=q.take (0,false);
  check (finishing.call=="LATE" && finishing.deferredSignoff && finishing.sentReport=="-16" && finishing.receivedReport=="-22");
  check (finishing.started==deferred.started && finishing.grid=="AA00");
  q.complete ();check (q.take (0,false).call=="FIRST");
  q.clear ();q.add (reply ("LATE",-10,100));q.take (0,false);
  q.add (reply ("CURRENT",-5,100));q.complete ();q.take (0,false);
  q.add (deferred);check (q.calls ()==(QStringList{} << "LATE")); // previously attempted caller may return for closure
  q.clear ();q.attach_current ("BUSY");
  q.add (reply ("NEWCALL",5,200));q.add (deferred);
  check (q.hasDeferredSignoff () && q.active ()=="BUSY");
  q.complete ();check (q.take (1,false).deferredSignoff); // closure before stronger new callers
  q.complete ();check (q.take (1,false).call=="NEWCALL");
  q.clear ();q.add (reply ("NEWCALL",5,200));q.add (deferred);
  check (q.take (2,true).deferredSignoff); // idle CQ uses the same closure priority
  q.clear ();q.attach_current ("BUSY");
  auto lateReport=deferred;lateReport.completionTx=4;lateReport.receivedReport="-12";
  q.add (lateReport);check (q.active ()=="BUSY");q.complete ();
  auto acknowledge=q.take (0,false);
  check (acknowledge.completionTx==4 && acknowledge.receivedReport=="-12");
  q.clear ();q.add (lateReport);q.add (deferred);q.add (lateReport);
  check (q.calls ().size ()==1);
  check (q.take (0,false).completionTx==5); // late RR73 upgrades, never downgrades
  std::cout << "PASS: CQ queue priorities, strict FIFO after first selection, deduplication, completion and manual/context cancellation\n";
}
