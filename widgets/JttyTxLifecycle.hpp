#ifndef JTTY_TX_LIFECYCLE_HPP_
#define JTTY_TX_LIFECYCLE_HPP_

#include <algorithm>
#include <chrono>
#include <optional>
#include <utility>
#include <vector>

#include "Audio/TxAudioQueue.hpp"

class JttyTxLifecycle final
{
public:
  enum class Backend
  {
    None,
    Local,
    Tci
  };

  enum class Phase
  {
    Idle,
    AwaitingEnqueue,
    Ready,
    Transmitting,
    Stopping
  };

  using Clock = std::chrono::steady_clock;
  using Duration = std::chrono::milliseconds;
  using TimePoint = Clock::time_point;

  struct Pending
  {
    qint64 enqueue_id {0};
    qint64 request_id {0};
  };

  struct Drain
  {
    TxAudioQueueEpoch epoch {};
    qint64 total {0};
  };

  struct StopContext
  {
    Backend backend {Backend::None};
    TxAudioQueueEpoch epoch {};
    TxAudioQueueProgress progress {};
  };

  struct EnqueueResolution
  {
    std::optional<Pending> pending;
    std::optional<Drain> drain;
  };

  struct BatchResolution
  {
    std::vector<Pending> pending;
    std::optional<Drain> drain;
  };

  static constexpr Duration defaultPreacceptanceTimeout () noexcept
  {
    return std::chrono::seconds {10};
  }

  explicit JttyTxLifecycle (
    Duration preacceptance_timeout = defaultPreacceptanceTimeout ()) noexcept
    : preacceptance_timeout_ {preacceptance_timeout}
  {
  }

  bool begin (Backend backend, TxAudioQueueEpoch epoch) noexcept
  {
    if (Phase::Idle != phase_ || Backend::None == backend || !epoch.isValid ()) {
      return false;
    }

    backend_ = backend;
    epoch_ = epoch;
    progress_.epoch = epoch;
    last_epoch_ = std::max (last_epoch_, epoch.value ());
    phase_ = Phase::AwaitingEnqueue;
    return true;
  }

  TxAudioQueueEpoch begin (Backend backend) noexcept
  {
    auto const next_epoch = TxAudioQueueEpoch {last_epoch_ + 1};
    return begin (backend, next_epoch) ? next_epoch : TxAudioQueueEpoch::invalid ();
  }

  bool addPending (qint64 enqueue_id, qint64 request_id,
                   TimePoint now = Clock::now ())
  {
    if (!canResolveEnqueue () || enqueue_id <= 0 || request_id <= 0
        || findPending (enqueue_id) != pending_.end ()) {
      return false;
    }

    if (pending_.empty ()) {
      preacceptance_deadline_ = now + preacceptance_timeout_;
    }
    pending_.push_back (Pending {enqueue_id, request_id});
    return true;
  }

  EnqueueResolution accept (TxAudioQueueEpoch epoch, qint64 enqueue_id,
                            TxAudioQueueProgress progress)
  {
    if (!canResolveEnqueue () || epoch != epoch_ || progress.epoch != epoch
        || progress.total_samples < progress_.total_samples) {
      return {};
    }

    auto const found = findPending (enqueue_id);
    if (pending_.end () == found) {
      return {};
    }

    EnqueueResolution result;
    result.pending = *found;
    pending_.erase (found);

    if (progress.total_samples > progress_.total_samples) {
      progress_ = progress;
      deferred_drain_.reset ();
    } else {
      progress_ = progress;
      if (pending_.empty ()) {
        result.drain = takeMatchingDeferredDrain ();
      }
    }

    if (Phase::AwaitingEnqueue == phase_) {
      phase_ = Phase::Ready;
    }
    clearDeadlineWhenSettled ();
    return result;
  }

  EnqueueResolution accept (TxAudioQueueEpoch epoch, qint64 enqueue_id,
                            qint64 committed_total)
  {
    auto progress = progress_;
    progress.epoch = epoch;
    progress.total_samples = committed_total;
    return accept (epoch, enqueue_id, progress);
  }

  EnqueueResolution fail (TxAudioQueueEpoch epoch, qint64 enqueue_id)
  {
    if (!canResolveEnqueue () || epoch != epoch_) {
      return {};
    }

    auto const found = findPending (enqueue_id);
    if (pending_.end () == found) {
      return {};
    }

    EnqueueResolution result;
    result.pending = *found;
    pending_.erase (found);
    if (pending_.empty ()) {
      result.drain = takeMatchingDeferredDrain ();
    }
    clearDeadlineWhenSettled ();
    return result;
  }

  BatchResolution failAll ()
  {
    BatchResolution result;
    if (Phase::Idle == phase_) {
      return result;
    }

    result.pending = std::move (pending_);
    pending_.clear ();
    if (Phase::Stopping == phase_) {
      deferred_drain_.reset ();
    } else {
      result.drain = takeMatchingDeferredDrain ();
    }
    preacceptance_deadline_.reset ();
    return result;
  }

  std::optional<Drain> observeDrain (TxAudioQueueEpoch epoch, qint64 total)
  {
    if (!isActive () || Phase::Stopping == phase_ || epoch != epoch_
        || total != progress_.total_samples) {
      return {};
    }

    Drain const drain {epoch, total};
    if (!pending_.empty ()) {
      deferred_drain_ = drain;
      return {};
    }
    return drain;
  }

  bool markTransmitting () noexcept
  {
    if (Phase::Ready != phase_) {
      return false;
    }
    phase_ = Phase::Transmitting;
    return true;
  }

  std::optional<StopContext> beginStop () noexcept
  {
    if (Phase::Idle == phase_) {
      return {};
    }
    if (Phase::Stopping != phase_) {
      stop_context_ = StopContext {backend_, epoch_, progress_};
      backend_stop_routed_ = false;
      preacceptance_deadline_.reset ();
      phase_ = Phase::Stopping;
    }
    return stop_context_;
  }

  bool markBackendStopRouted (StopContext const& context) noexcept
  {
    if (Phase::Stopping != phase_ || !stop_context_
        || !sameStopContext (*stop_context_, context)) {
      return false;
    }
    backend_stop_routed_ = true;
    return true;
  }

  bool reset () noexcept
  {
    if (Phase::Stopping != phase_ || !backend_stop_routed_) {
      return false;
    }

    backend_ = Backend::None;
    epoch_ = TxAudioQueueEpoch::invalid ();
    progress_ = {};
    pending_.clear ();
    deferred_drain_.reset ();
    stop_context_.reset ();
    backend_stop_routed_ = false;
    preacceptance_deadline_.reset ();
    phase_ = Phase::Idle;
    return true;
  }

  bool isActive () const noexcept {return Phase::Idle != phase_;}
  bool active () const noexcept {return isActive ();}
  bool hasPending () const noexcept {return !pending_.empty ();}
  Phase phase () const noexcept {return phase_;}
  Backend backend () const noexcept {return backend_;}
  TxAudioQueueEpoch epoch () const noexcept {return epoch_;}
  TxAudioQueueProgress progress () const noexcept {return progress_;}
  qint64 committedTotal () const noexcept {return progress_.total_samples;}
  std::vector<Pending> const& pending () const noexcept {return pending_;}
  std::optional<Drain> const& deferredDrain () const noexcept
  {
    return deferred_drain_;
  }
  std::optional<StopContext> const& stopContext () const noexcept
  {
    return stop_context_;
  }
  bool backendStopRouted () const noexcept {return backend_stop_routed_;}
  Duration preacceptanceTimeout () const noexcept
  {
    return preacceptance_timeout_;
  }
  std::optional<TimePoint> const& preacceptanceDeadline () const noexcept
  {
    return preacceptance_deadline_;
  }
  bool preacceptanceTimedOut (TimePoint now = Clock::now ()) const noexcept
  {
    return preacceptance_deadline_ && now >= *preacceptance_deadline_;
  }

private:
  using PendingIterator = std::vector<Pending>::iterator;

  bool canResolveEnqueue () const noexcept
  {
    return Phase::Idle != phase_ && Phase::Stopping != phase_;
  }

  PendingIterator findPending (qint64 enqueue_id) noexcept
  {
    return std::find_if (pending_.begin (), pending_.end (),
                         [enqueue_id] (Pending const& pending) {
                           return pending.enqueue_id == enqueue_id;
                         });
  }

  std::optional<Drain> takeMatchingDeferredDrain () noexcept
  {
    if (!deferred_drain_ || deferred_drain_->epoch != epoch_
        || deferred_drain_->total != progress_.total_samples) {
      deferred_drain_.reset ();
      return {};
    }
    auto result = deferred_drain_;
    deferred_drain_.reset ();
    return result;
  }

  void clearDeadlineWhenSettled () noexcept
  {
    if (pending_.empty ()) {
      preacceptance_deadline_.reset ();
    }
  }

  static bool sameStopContext (StopContext const& lhs,
                               StopContext const& rhs) noexcept
  {
    return lhs.backend == rhs.backend && lhs.epoch == rhs.epoch
      && lhs.progress.epoch == rhs.progress.epoch
      && lhs.progress.queued_samples == rhs.progress.queued_samples
      && lhs.progress.served_samples == rhs.progress.served_samples
      && lhs.progress.total_samples == rhs.progress.total_samples;
  }

  Duration preacceptance_timeout_;
  Phase phase_ {Phase::Idle};
  Backend backend_ {Backend::None};
  TxAudioQueueEpoch epoch_ {};
  qint64 last_epoch_ {0};
  TxAudioQueueProgress progress_ {};
  std::vector<Pending> pending_;
  std::optional<Drain> deferred_drain_;
  std::optional<StopContext> stop_context_;
  bool backend_stop_routed_ {false};
  std::optional<TimePoint> preacceptance_deadline_;
};

#endif
