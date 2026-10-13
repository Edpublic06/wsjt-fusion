// Personal WSJT-Z regression, 2026-10-13. GPL-3.0-or-later.
#include "CqReplyQueue.hpp"
#include <cstdlib>
#include <iostream>
void check (bool value) { if (!value) std::exit (1); }
wsjtz::CqReply reply (QString call,int signal,int distance,bool psk=false) {
  wsjtz::CqReply r; r.call=call;r.decode=call;r.signal=signal;r.distance=distance;r.psk=psk;return r;
}
int main () {
  wsjtz::CqReplyQueue q;
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
