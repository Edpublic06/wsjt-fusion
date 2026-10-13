// Personal WSJT-Z extension, 2026-10-13. GPL-3.0-or-later.
#ifndef WSJTZ_QUEUED_CALL_BUDGET_HPP
#define WSJTZ_QUEUED_CALL_BUDGET_HPP
#include <QString>
namespace wsjtz {
class QueuedCallBudget {
public:
  void begin (QString const& call) { call_=call; count_=0; }
  bool owns (QString const& call) const { return !call_.isEmpty () && call_==call; }
  void sent (QString const& call) { if (owns(call)) ++count_; }
  void reply (QString const& call) { if (owns(call)) count_=0; }
  bool exhausted (QString const& call,int limit) const { return limit>0 && owns(call) && count_>=limit; }
private:
  QString call_;
  int count_=0;
};
}
#endif
