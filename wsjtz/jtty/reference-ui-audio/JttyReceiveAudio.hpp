#ifndef JTTY_RECEIVE_AUDIO_HPP
#define JTTY_RECEIVE_AUDIO_HPP

#include <QMetaType>
#include <QMutex>
#include <QMutexLocker>
#include <QtGlobal>
#include <atomic>
#include <deque>
#include <memory>
#include <utility>
#include <vector>

enum class ReceivePolicy { Timed, ContinuousJtty };
Q_DECLARE_METATYPE (ReceivePolicy)

enum class JttyReceiveReason
{
  Started, MonitorStopped, SourceChanged, ModeChanged, InputError, Overrun,
  Transmission, FrequencyChanged, Replay
};
Q_DECLARE_METATYPE (JttyReceiveReason)

struct JttyReceiveEvent
{
  enum class Kind { Begin, Samples, End, Gap };
  Kind kind = Kind::Begin;
  quint64 session = 0;
  quint64 context = 0;
  qint64 firstSample = 0;
  qint64 anchorUtcMs = 0;
  JttyReceiveReason reason = JttyReceiveReason::Started;
  std::vector<short> samples;
  qint64 endSample () const { return firstSample + qint64 (samples.size ()); }
};

class JttyReceiveMailbox
{
public:
  static constexpr qint64 capacitySamples = 180 * 12000;

  std::shared_ptr<JttyReceiveMailbox> detachPending ()
  {
    auto pending = std::make_shared<JttyReceiveMailbox> ();
    QMutexLocker lock {&mutex_};
    pending->events_.swap (events_);
    pending->samples_ = samples_;
    samples_ = 0;
    notified_ = false;
    return pending;
  }

  void discardPending ()
  {
    QMutexLocker lock {&mutex_};
    events_.clear ();
    samples_ = 0;
    notified_ = false;
  }

  bool take (JttyReceiveEvent& event)
  {
    QMutexLocker lock {&mutex_};
    if (events_.empty ())
      {
        notified_ = false;
        return false;
      }
    event = std::move (events_.front ());
    events_.pop_front ();
    samples_ -= qint64 (event.samples.size ());
    return true;
  }

  struct Publication { bool notify; bool overflow; };
  Publication publish (JttyReceiveEvent event)
  {
    QMutexLocker lock {&mutex_};
    if (samples_ + qint64 (event.samples.size ()) > capacitySamples
        || events_.size () >= 1024)
      {
        events_.clear ();
        samples_ = 0;
        return {false, true};
      }
    samples_ += qint64 (event.samples.size ());
    events_.push_back (std::move (event));
    bool const notify = !notified_;
    notified_ = true;
    return {notify, false};
  }

private:
  QMutex mutex_;
  std::deque<JttyReceiveEvent> events_;
  qint64 samples_ = 0;
  bool notified_ = false;
};

using JttyReceiveMailboxPtr = std::shared_ptr<JttyReceiveMailbox>;
Q_DECLARE_METATYPE (JttyReceiveMailboxPtr)

// Confined to the input thread; the mailbox is the only shared mutable state.
class JttyReceivePublisher
{
public:
  JttyReceivePublisher () : mailbox_ {std::make_shared<JttyReceiveMailbox> ()}
  {
    qRegisterMetaType<JttyReceiveMailboxPtr> ("JttyReceiveMailboxPtr");
    qRegisterMetaType<ReceivePolicy> ("ReceivePolicy");
    qRegisterMetaType<JttyReceiveReason> ("JttyReceiveReason");
  }

  JttyReceiveMailboxPtr mailbox () const { return mailbox_; }
  bool active () const { return active_; }
  void setContext (quint64 context) { context_ = context; }

  bool begin (qint64 anchorUtcMs, JttyReceiveReason reason = JttyReceiveReason::Started)
  {
    if (active_) return false;
    static std::atomic<quint64> nextSession {0};
    session_ = ++nextSession;
    nextSample_ = 0;
    anchorUtcMs_ = anchorUtcMs;
    active_ = true;
    return publish (event (JttyReceiveEvent::Kind::Begin, reason));
  }

  bool append (short const * samples, int count)
  {
    if (!active_ || count <= 0) return false;
    auto block = event (JttyReceiveEvent::Kind::Samples, JttyReceiveReason::Started);
    block.samples.assign (samples, samples + count);
    auto result = mailbox_->publish (std::move (block));
    bool notify = result.notify;
    if (result.overflow)
      {
        notify = recoverOverflow () || notify;
        block.session = session_;
        block.firstSample = 0;
        block.anchorUtcMs = anchorUtcMs_;
        block.samples.assign (samples, samples + count);
        notify = mailbox_->publish (std::move (block)).notify || notify;
      }
    nextSample_ += count;
    return notify;
  }

  bool end (JttyReceiveReason reason)
  {
    if (!active_) return false;
    auto const result = mailbox_->publish (event (JttyReceiveEvent::Kind::End, reason));
    bool notify = result.notify;
    if (result.overflow)
      {
        notify = mailbox_->publish (event (JttyReceiveEvent::Kind::Gap,
                                            JttyReceiveReason::Overrun)).notify || notify;
        notify = mailbox_->publish (event (JttyReceiveEvent::Kind::End, reason)).notify || notify;
      }
    active_ = false;
    return notify;
  }

private:
  JttyReceiveEvent event (JttyReceiveEvent::Kind kind, JttyReceiveReason reason) const
  {
    JttyReceiveEvent value;
    value.kind = kind;
    value.session = session_;
    value.context = context_;
    value.firstSample = nextSample_;
    value.anchorUtcMs = anchorUtcMs_;
    value.reason = reason;
    return value;
  }

  bool recoverOverflow ()
  {
    bool notify = mailbox_->publish (event (JttyReceiveEvent::Kind::Gap,
                                            JttyReceiveReason::Overrun)).notify;
    auto const nextAnchor = anchorUtcMs_ + nextSample_ * 1000 / 12000;
    active_ = false;
    return begin (nextAnchor, JttyReceiveReason::Overrun) || notify;
  }

  bool publish (JttyReceiveEvent value)
  {
    auto const result = mailbox_->publish (std::move (value));
    return result.overflow ? recoverOverflow () : result.notify;
  }

  JttyReceiveMailboxPtr mailbox_;
  quint64 session_ = 0;
  quint64 context_ = 0;
  qint64 nextSample_ = 0;
  qint64 anchorUtcMs_ = 0;
  bool active_ = false;
};

#endif
