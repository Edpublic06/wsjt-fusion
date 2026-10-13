// Personal WSJT-Z extension, 2026-10-13. GPL-3.0-or-later.
#ifndef WSJTZ_CQ_REPLY_QUEUE_HPP
#define WSJTZ_CQ_REPLY_QUEUE_HPP
#include <QString>
#include <QStringList>
#include <QVector>
#include <QSet>
#include <algorithm>
namespace wsjtz {
struct CqReply { QString call, decode; int signal = -31, distance = 0, highlight = 0; bool psk = false; };
class CqReplyQueue {
public:
  void clear () { waiting_.clear (); seen_.clear (); active_.clear (); session_ = false; }
  void context (QString const& value) { if (value != context_) { clear (); context_ = value; } }
  bool session () const { return session_; }
  QString active () const { return active_; }
  bool empty () const { return waiting_.isEmpty (); }
  void highlight (QString const& call,int rank) { for (auto& r : waiting_) if (r.call == call) r.highlight = rank; }
  void add (CqReply const& reply) {
    session_ = true;
    if (reply.call == active_ || seen_.contains (reply.call)) return;
    for (auto& old : waiting_) if (old.call == reply.call) { old = reply; return; }
    waiting_.push_back (reply);
  }
  CqReply take (int priority, bool prefer_psk) {
    std::stable_sort (waiting_.begin (), waiting_.end (), [=] (CqReply const& a, CqReply const& b) {
      if (a.highlight != b.highlight) return a.highlight > b.highlight;
      if (prefer_psk && a.psk != b.psk) return a.psk;
      if (priority == 1 && a.signal != b.signal) return a.signal > b.signal;
      if (priority == 2 && a.distance != b.distance) return a.distance > b.distance;
      return false;
    });
    if (waiting_.isEmpty ()) return {};
    auto reply = waiting_.takeFirst (); active_ = reply.call; seen_.insert (reply.call); return reply;
  }
  void complete () { active_.clear (); if (waiting_.isEmpty ()) clear (); }
  QStringList calls () const { QStringList result; for (auto const& r : waiting_) result << r.call; return result; }
private:
  QVector<CqReply> waiting_;
  QSet<QString> seen_;
  QString active_,context_;
  bool session_ = false;
};
}
#endif
