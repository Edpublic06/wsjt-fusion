#include <QtTest>

#include "widgets/JttyTransmitQueue.hpp"

class TestJttyTransmitQueue final
  : public QObject
{
  Q_OBJECT

  static Jtty::TransmitSegment segment (QString const& text, int toneCount = 2)
  {
    return {text, QVector<int> (toneCount, 1), 0};
  }

private slots:
  void fifoPreservesFullSegmentsAndLaterRequests ()
  {
    Jtty::TransmitQueue queue;
    auto const full = QString (79, 'A') + 'Z';
    queue.append (11, {segment (full), segment ("TAIL")});
    QCOMPARE (queue.nextRequestId (), qint64 {11});
    QCOMPARE (queue.nextSegment ()->text, full);
    QVERIFY (queue.commitNext (3072));

    queue.append (12, {segment ("NEW MESSAGE")});
    QCOMPARE (queue.nextRequestId (), qint64 {11});
    QCOMPARE (queue.nextSegment ()->text, QString {"TAIL"});
    QVERIFY (!queue.commitNext (6144));
    QCOMPARE (queue.nextRequestId (), qint64 {12});
    QCOMPARE (queue.nextSegment ()->text, QString {"NEW MESSAGE"});
    QVERIFY (queue.commitNext (9216));
    QVERIFY (!queue.hasUncommitted ());
    QVERIFY (!queue.nextSegment ());
    QCOMPARE (queue.nextRequestId (), qint64 {0});
    QCOMPARE (queue.requests ().at (0).segments.at (0).text, full);
  }

  void progressDoesNotCompleteRequests ()
  {
    Jtty::TransmitQueue queue;
    queue.append (21, {segment ("FIRST"), segment ("LAST", 3)});
    QCOMPARE (queue.segmentCount (), 2);
    QCOMPARE (queue.nextSegment ()->sampleCount (), qint64 {3072});
    QCOMPARE (queue.remainingSegments (0), 2);
    QCOMPARE (queue.pendingText (0), QString {"FIRST\nLAST"});
    queue.commitNext (3072);
    QCOMPARE (queue.remainingSegments (3071), 2);
    QCOMPARE (queue.remainingSegments (3072), 1);
    QCOMPARE (queue.pendingText (3072), QString {"LAST"});
    QVERIFY (queue.complete (3072).isEmpty ());
    QCOMPARE (queue.nextSegment ()->sampleCount (), qint64 {4608});
    queue.commitNext (7680);
    QCOMPARE (queue.remainingSegments (7680), 0);
    QVERIFY (queue.pendingText (7680).isEmpty ());
    QVERIFY (!queue.empty ());
    QCOMPARE (queue.segmentCount (), 2);
    QVERIFY (queue.complete (7679).isEmpty ());
    QCOMPARE (queue.complete (7680), QVector<qint64> {21});
    QVERIFY (queue.empty ());
    QVERIFY (queue.complete (7680).isEmpty ());
  }

  void drainCompletesOnlyCommittedPrefix ()
  {
    Jtty::TransmitQueue queue;
    queue.append (31, {segment ("FIRST")});
    queue.append (32, {segment ("SECOND"), segment ("THIRD")});
    queue.append (33, {segment ("FOURTH")});
    queue.commitNext (3072);
    queue.commitNext (6144);
    QCOMPARE (queue.complete (6144), QVector<qint64> {31});
    QCOMPARE (queue.requests ().size (), 2);
    QCOMPARE (queue.nextSegment ()->text, QString {"THIRD"});
    queue.commitNext (9216);
    queue.commitNext (12288);
    QCOMPARE (queue.complete (9216), QVector<qint64> {32});
    QCOMPARE (queue.complete (12288), QVector<qint64> {33});
    QVERIFY (queue.empty ());
  }

  void cancellationReportsAcceptedAndUnsubmittedOnce ()
  {
    Jtty::TransmitQueue queue;
    queue.append (41, {segment ("ACCEPTED")});
    queue.append (42, {segment ("PENDING"), segment ("UNSUBMITTED")});
    queue.append (43, {segment ("LATER")});
    queue.commitNext (3072);
    queue.commitNext (6144);
    auto const cancelled = queue.cancel ();
    QCOMPARE (cancelled, (QVector<qint64> {41, 42, 43}));
    QVERIFY (queue.empty ());
    QVERIFY (!queue.hasUncommitted ());
    QCOMPARE (queue.segmentCount (), 0);
    QCOMPARE (queue.remainingSegments (0), 0);
    QVERIFY (queue.pendingText (0).isEmpty ());
    QVERIFY (queue.cancel ().isEmpty ());
    QVERIFY (queue.complete (6144).isEmpty ());
    queue.append (44, {segment ("FRESH")});
    QCOMPARE (queue.nextSegment ()->text, QString {"FRESH"});
    QVERIFY (queue.commitNext (3072));
    QCOMPARE (queue.complete (3072), QVector<qint64> {44});
  }
};

QTEST_GUILESS_MAIN (TestJttyTransmitQueue)

#include "test_jtty_transmit_queue.moc"
