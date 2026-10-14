#include "SnrHistory.hpp"
#include <cstdlib>
#include <iostream>
int main() {
  using H = wsjtz::JttySnrHistory;
  H history;
  auto check = [](bool ok) { if (!ok) std::abort(); };
  check(history.snrForSelectedWord("N3CH", H::Pane::All) == -10);
  history.observe("CQ N3CH FN10", -12, true);
  history.observe("CQ N3CHA FN10", 5, true);
  history.observe("F6FDY N0IGU EN12", -3, true);
  check(history.snrForSelectedWord("N3CH", H::Pane::All) == -12);
  history.observe("N3CH TU", -7, false);
  check(history.snrForSelectedWord("N3CH", H::Pane::All) == -7);
  check(history.snrForSelectedWord("N3CH", H::Pane::Qso) == -12);
  history.observe("", 9, true);
  check(history.snrForSelectedWord("N3CH", H::Pane::All) == -7);
  for (int n=0; n<500; ++n) history.observe("CQ OTHER", 1, true);
  check(history.snrForSelectedWord("N3CH", H::Pane::All) == -10);
  std::cout << "Selected peer SNR, exact words, pane routing and history bounds passed\n";
}
