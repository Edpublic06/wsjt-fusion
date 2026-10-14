#include <chrono>

#include <QtTest>

#include "widgets/JttyTxLifecycle.hpp"

class TestJttyTxLifecycle final : public QObject
{
  Q_OBJECT

private slots:
  void pendingSessionRetainsBackendIdentity ()
  {
    JttyTxLifecycle lifecycle;
    auto const epoch = TxAudioQueueEpoch {7};

    QVERIFY (lifecycle.begin (JttyTxLifecycle::Backend::Tci, epoch));
    QVERIFY (lifecycle.addPending (101, 201));

    QCOMPARE (lifecycle.phase (), JttyTxLifecycle::Phase::AwaitingEnqueue);
    QCOMPARE (lifecycle.backend (), JttyTxLifecycle::Backend::Tci);
    QCOMPARE (lifecycle.epoch (), epoch);
    QCOMPARE (lifecycle.pending ().size (), std::size_t {1});
    QCOMPARE (lifecycle.pending ().front ().request_id, 201);
  }

  void stopSnapshotIsImmutable ()
  {
    JttyTxLifecycle lifecycle;
    auto const epoch = lifecycle.begin (JttyTxLifecycle::Backend::Local);
    QVERIFY (epoch.isValid ());
    QVERIFY (lifecycle.addPending (1, 2));
    TxAudioQueueProgress progress;
    progress.epoch = epoch;
    progress.queued_samples = 3600;
    progress.served_samples = 1200;
    progress.total_samples = 4800;
    auto const accepted = lifecycle.accept (epoch, 1, progress);
    QVERIFY (accepted.pending.has_value ());
    QVERIFY (lifecycle.markTransmitting ());

    auto const first = lifecycle.beginStop ();
    QVERIFY (first.has_value ());
    QCOMPARE (first->backend, JttyTxLifecycle::Backend::Local);
    QCOMPARE (first->epoch, epoch);
    QCOMPARE (first->progress.queued_samples, 3600);
    QCOMPARE (first->progress.served_samples, 1200);
    QCOMPARE (first->progress.total_samples, 4800);

    QVERIFY (!lifecycle.begin (JttyTxLifecycle::Backend::Tci,
                               TxAudioQueueEpoch {99}));
    QVERIFY (!lifecycle.addPending (3, 4));
    QVERIFY (!lifecycle.accept (epoch, 3, 9600).pending.has_value ());

    auto const repeated = lifecycle.beginStop ();
    QVERIFY (repeated.has_value ());
    QCOMPARE (repeated->backend, first->backend);
    QCOMPARE (repeated->epoch, first->epoch);
    QCOMPARE (repeated->progress.total_samples, first->progress.total_samples);
  }

  void acceptanceAdvancingTotalDiscardsDeferredDrain ()
  {
    JttyTxLifecycle lifecycle;
    auto const epoch = lifecycle.begin (JttyTxLifecycle::Backend::Tci);
    QVERIFY (lifecycle.addPending (1, 10));

    QVERIFY (!lifecycle.observeDrain (epoch, 0).has_value ());
    QVERIFY (lifecycle.deferredDrain ().has_value ());

    auto const accepted = lifecycle.accept (epoch, 1, 2400);
    QVERIFY (accepted.pending.has_value ());
    QVERIFY (!accepted.drain.has_value ());
    QVERIFY (!lifecycle.deferredDrain ().has_value ());
    QCOMPARE (lifecycle.committedTotal (), 2400);
  }

  void finalFailureReleasesMatchingDeferredDrain ()
  {
    JttyTxLifecycle lifecycle;
    auto const epoch = lifecycle.begin (JttyTxLifecycle::Backend::Tci);
    QVERIFY (lifecycle.addPending (1, 10));
    QVERIFY (lifecycle.addPending (2, 20));

    QVERIFY (!lifecycle.observeDrain (epoch, 0).has_value ());
    auto const first = lifecycle.fail (epoch, 1);
    QVERIFY (first.pending.has_value ());
    QVERIFY (!first.drain.has_value ());

    auto const last = lifecycle.fail (epoch, 2);
    QVERIFY (last.pending.has_value ());
    QVERIFY (last.drain.has_value ());
    QCOMPARE (last.drain->epoch, epoch);
    QCOMPARE (last.drain->total, 0);
  }

  void rigCloseCanRejectEveryPendingRequestBeforeAcceptance ()
  {
    JttyTxLifecycle lifecycle;
    auto const epoch = lifecycle.begin (JttyTxLifecycle::Backend::Tci);
    QVERIFY (lifecycle.addPending (1, 11));
    QVERIFY (lifecycle.addPending (2, 22));

    auto const failed = lifecycle.failAll ();
    QCOMPARE (failed.pending.size (), std::size_t {2});
    QCOMPARE (failed.pending.at (0).request_id, 11);
    QCOMPARE (failed.pending.at (1).request_id, 22);
    QVERIFY (!lifecycle.hasPending ());

    auto const stop = lifecycle.beginStop ();
    QVERIFY (stop.has_value ());
    QCOMPARE (stop->backend, JttyTxLifecycle::Backend::Tci);
    QVERIFY (!lifecycle.accept (epoch, 1, 100).pending.has_value ());
  }

  void localStopCanRejectPendingAfterSnapshot ()
  {
    JttyTxLifecycle lifecycle;
    auto const epoch = lifecycle.begin (JttyTxLifecycle::Backend::Tci);
    QVERIFY (lifecycle.addPending (1, 11));

    auto const stop = lifecycle.beginStop ();
    QVERIFY (stop.has_value ());
    QVERIFY (!lifecycle.fail (epoch, 1).pending.has_value ());

    auto const failed = lifecycle.failAll ();
    QCOMPARE (failed.pending.size (), std::size_t {1});
    QCOMPARE (failed.pending.front ().request_id, 11);
    QVERIFY (!failed.drain.has_value ());
  }

  void preacceptanceTimeoutIsConfigurableAndStartsWithFirstPending ()
  {
    using namespace std::chrono_literals;

    JttyTxLifecycle lifecycle {25ms};
    auto const epoch = lifecycle.begin (JttyTxLifecycle::Backend::Local);
    auto const start = JttyTxLifecycle::TimePoint {1s};

    QVERIFY (!lifecycle.preacceptanceDeadline ().has_value ());
    QVERIFY (lifecycle.addPending (1, 1, start));
    QCOMPARE (lifecycle.preacceptanceTimeout (), 25ms);
    QCOMPARE (*lifecycle.preacceptanceDeadline (), start + 25ms);
    QVERIFY (!lifecycle.preacceptanceTimedOut (start + 24ms));
    QVERIFY (lifecycle.preacceptanceTimedOut (start + 25ms));

    QVERIFY (lifecycle.addPending (2, 2, start + 20ms));
    QCOMPARE (*lifecycle.preacceptanceDeadline (), start + 25ms);
    QVERIFY (lifecycle.accept (epoch, 1, 10).pending.has_value ());
    QVERIFY (lifecycle.preacceptanceDeadline ().has_value ());
    QVERIFY (lifecycle.fail (epoch, 2).pending.has_value ());
    QVERIFY (!lifecycle.preacceptanceDeadline ().has_value ());
  }

  void resetRequiresBackendStopRouting ()
  {
    JttyTxLifecycle lifecycle;
    auto const epoch = lifecycle.begin (JttyTxLifecycle::Backend::Local);
    QVERIFY (lifecycle.addPending (1, 1));
    QVERIFY (lifecycle.accept (epoch, 1, 10).pending.has_value ());

    auto const stop = lifecycle.beginStop ();
    QVERIFY (stop.has_value ());
    QVERIFY (!lifecycle.reset ());
    QCOMPARE (lifecycle.phase (), JttyTxLifecycle::Phase::Stopping);
    QCOMPARE (lifecycle.backend (), JttyTxLifecycle::Backend::Local);

    auto wrong = *stop;
    wrong.backend = JttyTxLifecycle::Backend::Tci;
    QVERIFY (!lifecycle.markBackendStopRouted (wrong));
    QVERIFY (!lifecycle.reset ());

    QVERIFY (lifecycle.markBackendStopRouted (*stop));
    QVERIFY (lifecycle.reset ());
    QCOMPARE (lifecycle.phase (), JttyTxLifecycle::Phase::Idle);
    QCOMPARE (lifecycle.backend (), JttyTxLifecycle::Backend::None);
    QVERIFY (!lifecycle.accept (epoch, 1, 10).pending.has_value ());

    auto const next = lifecycle.begin (JttyTxLifecycle::Backend::Tci);
    QCOMPARE (next.value (), epoch.value () + 1);
  }
};

QTEST_GUILESS_MAIN (TestJttyTxLifecycle)

#include "test_jtty_tx_lifecycle.moc"
