// Personal WSJT-Z regression, 2026-10-13. GPL-3.0-or-later.
#include "IncomingCallPolicy.hpp"
#include <iostream>
#include <cstdlib>

static void require (bool condition, char const * message)
{
  if (!condition) { std::cerr << message << '\n'; std::exit (1); }
}
int main ()
{
  wsjtz::IncomingCallPolicy policy;
  auto eligible = [&] (bool enabled = true, bool filtered = false) {
    return policy.should_switch (enabled, true, true, true, true, true, true,
                                 true, filtered, "N2NEW");
  };
  policy.observe ("K1OLD", "N2NEW", true);
  require (!eligible (false), "default/disabled option changed behavior");
  require (eligible (), "unanswered target did not yield to direct caller");
  require (!eligible (true, true), "filtered caller was accepted");
  // Each of the operational gates must prevent an automatic handover.
  for (int gate = 0; gate < 8; ++gate) {
    bool gates[8] = {true,true,true,true,true,true,true,true};
    gates[gate] = false;
    require (!policy.should_switch (gates[0],gates[1],gates[2],gates[3],
              gates[4],gates[5],gates[6],gates[7],false,"N2NEW"),
             "inactive mode, TX, late QSO or invalid message caused a handover");
  }
  require (!policy.should_switch (true,true,true,true,true,true,true,true,false,"K1OLD"),
           "current target treated as another caller");
  policy.observe ("K1OLD", "K1OLD", false);
  require (eligible (), "CQ/traffic not directed to us counted as a reply");
  policy.observe ("K1OLD", "K1OLD", true);
  require (!eligible (), "established exchange interrupted");
  policy.observe ("K3OTHER", "N2NEW", true);
  require (eligible (), "new outgoing attempt retained stale answered state");
  policy.accept_incoming ("N2NEW");
  policy.observe ("N2NEW", "W4NEXT", true);
  require (!policy.should_switch (true,true,true,true,true,true,true,true,false,"W4NEXT"),
           "incoming QSO was stolen by a subsequent caller");
  // QSO completion clears the selected station. Auto Call then selects a new
  // outgoing target through its existing cycle; no old target is restored here.
  policy.reset ();
  require (!eligible (), "empty target triggered a switch after QSO completion");
  policy.observe ("K5NEXT", "N2NEW", true);
  require (eligible (), "normal Auto Call cycle could not resume after completion");
  std::cout << "PASS: optional incoming-call handover, reply protection and cycle reset\n";
}
