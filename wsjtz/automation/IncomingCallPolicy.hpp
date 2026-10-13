// Personal WSJT-Z extension, modified 2026-10-13. GPL-3.0-or-later.
#ifndef WSJTZ_INCOMING_CALL_POLICY_HPP
#define WSJTZ_INCOMING_CALL_POLICY_HPP
#include <QString>

namespace wsjtz {
// Receives normalized base calls from the application adapter. Keeping this
// state outside MainWindow makes it independent of upstream UI/decoder layouts.
class IncomingCallPolicy
{
public:
  void reset () { target_.clear (); answered_ = false; }
  void observe (QString const& selected, QString const& sender, bool directed_to_us)
  {
    if (selected != target_) { target_ = selected; answered_ = false; }
    if (!target_.isEmpty () && directed_to_us && sender == target_) answered_ = true;
  }
  bool should_switch (bool enabled, bool auto_call, bool auto_tx, bool auto_sequence,
                      bool live_receive, bool early_qso, bool supported_activity,
                      bool valid_incoming_call, bool filtered,
                      QString const& incoming) const
  {
    return enabled && auto_call && auto_tx && auto_sequence && live_receive
      && early_qso && supported_activity && valid_incoming_call && !filtered
      && !target_.isEmpty () && !answered_ && !incoming.isEmpty () && incoming != target_;
  }
  void accept_incoming (QString const& incoming)
  {
    // The new caller has already addressed us: protect this exchange from
    // further callers, even before its first response to our next message.
    target_ = incoming;
    answered_ = true;
  }
  bool answered (QString const& selected) const { return selected == target_ && answered_; }
private:
  QString target_;
  bool answered_ = false;
};
}
#endif
