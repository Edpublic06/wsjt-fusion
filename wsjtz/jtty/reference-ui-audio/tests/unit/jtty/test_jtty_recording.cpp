#include <QtTest>
#include <QTemporaryDir>
#include <QAudioFormat>
#include <QFileInfo>
#include <algorithm>

#include "Audio/BWFFile.hpp"
#include "JttyRecording.hpp"

namespace
{
using Recording = JttyRecording;

Recording::Settings settings (Recording::SavePolicy policy)
{
  Recording::Settings result;
  result.policy = policy;
  result.directory = QStringLiteral ("/unused-recording-test-path");
  result.myCall = QStringLiteral ("K1ABC");
  result.myGrid = QStringLiteral ("FN42");
  result.frequency = 14074000;
  return result;
}

QDateTime origin ()
{
  return QDateTime {QDate {2026, 9, 11}, QTime {23, 58, 42, 123}, Qt::UTC};
}

std::vector<short> audio (qint64 count)
{
  std::vector<short> result (std::size_t (count), 0);
  for (qint64 i = 0; i < count; ++i) result[std::size_t (i)] = short (i % 30001);
  return result;
}
}

class TestJttyRecording final : public QObject
{
  Q_OBJECT
private slots:
  void overlappingSegmentsRetainExactAudioAndOrigin ()
  {
    std::vector<Recording::Snapshot> saved;
    Recording recorder {nullptr, [&saved] (Recording::Snapshot segment, Recording::Completion done) {
      saved.push_back (segment);
      done ({});
    }};
    recorder.beginReception (17, origin (), settings (Recording::SavePolicy::All));
    auto const pcm = audio (Recording::segmentSamples + 123);
    recorder.feed (0, pcm.data (), int (pcm.size ()));
    QCOMPARE (saved.size (), std::size_t {1});
    recorder.endReception ();
    QCOMPARE (saved.size (), std::size_t {2});
    QCOMPARE (saved[0]->firstSample, qint64 {0});
    QCOMPARE (saved[0]->endSample (), Recording::segmentSamples);
    QCOMPARE (saved[1]->firstSample, Recording::hopSamples);
    QCOMPARE (saved[1]->endSample (), qint64 (pcm.size ()));
    QVERIFY (saved[0]->id != saved[1]->id);
    QVERIFY (saved[0]->fileName != saved[1]->fileName);
    for (auto const& segment : saved) {
      QCOMPARE (segment->reception, quint64 {17});
      QCOMPARE (segment->firstSampleUtc, origin ().addMSecs (segment->firstSample / 12));
      QVERIFY (std::equal (segment->samples.begin (), segment->samples.end (),
                           pcm.begin () + segment->firstSample));
    }
    QCOMPARE (saved[1]->firstSampleUtc.date (), QDate (2026, 9, 12));
  }

  void partialDecodeQualifiesBothOverlappingSegmentsAfterClosure ()
  {
    std::vector<Recording::Snapshot> saved;
    Recording recorder {nullptr, [&saved] (Recording::Snapshot segment, Recording::Completion done) {
      saved.push_back (segment);
      done ({});
    }};
    recorder.beginReception (1, origin (), settings (Recording::SavePolicy::Decoded));
    auto const pcm = audio (Recording::segmentSamples + 100);
    recorder.feed (0, pcm.data (), int (pcm.size ()));
    QVERIFY (saved.empty ());
    recorder.noteDecoded (Recording::segmentSamples - 1000, Recording::segmentSamples + 10);
    recorder.advanceDecoderWatermark (Recording::segmentSamples - 1);
    QVERIFY (saved.empty ());
    recorder.advanceDecoderWatermark (Recording::segmentSamples);
    QCOMPARE (saved.size (), std::size_t {1});
    recorder.endReception ();
    QCOMPARE (saved.size (), std::size_t {2});
    QVERIFY (saved[0]->decoded);
    QVERIFY (saved[1]->decoded);
  }

  void noiseAndNonoverlappingEvidenceDoNotQualify ()
  {
    int writes = 0;
    Recording recorder {nullptr, [&writes] (Recording::Snapshot, Recording::Completion done) {
      ++writes;
      done ({});
    }};
    recorder.beginReception (1, origin (), settings (Recording::SavePolicy::Decoded));
    auto const pcm = audio (1000);
    recorder.feed (0, pcm.data (), int (pcm.size ()));
    recorder.noteDecoded (1000, 2000);
    recorder.noteDecoded (20, 20);
    recorder.endReception ();
    QCOMPARE (writes, 0);
  }

  void savePolicyIsFrozenAndOffIntervalsAreNotJoined ()
  {
    std::vector<Recording::Snapshot> saved;
    Recording recorder {nullptr, [&saved] (Recording::Snapshot segment, Recording::Completion done) {
      saved.push_back (segment);
      done ({});
    }};
    recorder.beginReception (1, origin (), settings (Recording::SavePolicy::All));
    auto const pcm = audio (8000);
    recorder.feed (0, pcm.data (), 1000);
    recorder.updateSavePolicy (Recording::SavePolicy::Off);
    QCOMPARE (saved.size (), std::size_t {1});
    QCOMPARE (saved[0]->settings.policy, Recording::SavePolicy::All);
    recorder.feed (1000, pcm.data (), 1000);
    recorder.updateSavePolicy (Recording::SavePolicy::Decoded);
    recorder.feed (2000, pcm.data () + 2000, 6000);
    recorder.noteDecoded (5700, 5800);
    recorder.endReception ();
    QCOMPARE (saved.size (), std::size_t {2});
    QCOMPARE (QFileInfo {saved[0]->fileName}.fileName (), QStringLiteral ("260911_235842_123.wav"));
    QCOMPARE (QFileInfo {saved[1]->fileName}.fileName (), QStringLiteral ("260911_235842_595.wav"));
    QCOMPARE (saved[1]->firstSample, Recording::searchStepSamples);
    QCOMPARE (saved[1]->samples.size (), std::size_t (8000 - Recording::searchStepSamples));
    QVERIFY (std::equal (saved[1]->samples.begin (), saved[1]->samples.end (),
                         pcm.begin () + Recording::searchStepSamples));
    QCOMPARE (saved[1]->settings.policy, Recording::SavePolicy::Decoded);
  }

  void metadataUpdatesApplyOnlyToNewSegments ()
  {
    std::vector<Recording::Snapshot> saved;
    Recording recorder {nullptr, [&saved] (Recording::Snapshot segment, Recording::Completion done) {
      saved.push_back (segment);
      done ({});
    }};
    auto initial = settings (Recording::SavePolicy::All);
    initial.hisCall = QStringLiteral ("K1OLD");
    recorder.beginReception (1, origin (), initial);
    auto const pcm = audio (Recording::segmentSamples + 1);
    recorder.feed (0, pcm.data (), int (Recording::hopSamples));

    auto updated = initial;
    updated.directory = QStringLiteral ("/updated-recording-test-path");
    updated.hisCall = QStringLiteral ("K1NEW");
    updated.policy = Recording::SavePolicy::Off;
    recorder.updateMetadata (updated);
    recorder.feed (Recording::hopSamples, pcm.data () + Recording::hopSamples,
                   int (pcm.size () - Recording::hopSamples));
    recorder.endReception ();

    QCOMPARE (saved.size (), std::size_t {2});
    QCOMPARE (saved[0]->settings.hisCall, QStringLiteral ("K1OLD"));
    QCOMPARE (saved[0]->settings.directory, initial.directory);
    QCOMPARE (saved[1]->settings.hisCall, QStringLiteral ("K1NEW"));
    QCOMPARE (saved[1]->settings.directory, updated.directory);
    QCOMPARE (saved[1]->settings.policy, Recording::SavePolicy::All);
  }

  void delayedFailureStopsSavingUntilExplicitRearm ()
  {
    std::vector<Recording::Completion> completions;
    int errors = 0;
    Recording recorder {nullptr, [&completions] (Recording::Snapshot, Recording::Completion done) {
      completions.push_back (done);
    }};
    recorder.setErrorHandler ([&errors] (QString const&) {++errors;});
    auto const pcm = audio (7000);
    recorder.beginReception (1, origin (), settings (Recording::SavePolicy::All));
    recorder.feed (0, pcm.data (), 1000);
    recorder.endReception ();
    QCOMPARE (recorder.snapshotsInFlight (), 1);
    completions[0] (QStringLiteral ("disk full"));
    QVERIFY (recorder.suspendedAfterError ());
    QCOMPARE (errors, 1);
    recorder.beginReception (2, origin (), settings (Recording::SavePolicy::All));
    recorder.feed (0, pcm.data (), 1000);
    QCOMPARE (completions.size (), std::size_t {1});
    recorder.updateSavePolicy (Recording::SavePolicy::All);
    recorder.feed (1000, pcm.data () + 1000, 6000);
    recorder.endReception ();
    QCOMPARE (completions.size (), std::size_t {2});
    completions[1] ({});
    QVERIFY (!recorder.suspendedAfterError ());
  }

  void delayedStartPreservesSearchPhaseAcrossInputBlocks ()
  {
    std::vector<Recording::Snapshot> saved;
    Recording recorder {nullptr, [&saved] (Recording::Snapshot segment, Recording::Completion done) {
      saved.push_back (segment);
      done ({});
    }};
    recorder.beginReception (1, origin (), settings (Recording::SavePolicy::All));
    auto const pcm = audio (8000);
    recorder.feed (3456, pcm.data () + 3456, 1000);
    recorder.feed (4456, pcm.data () + 4456, 1000);
    recorder.feed (5456, pcm.data () + 5456, 2544);
    recorder.endReception ();
    QCOMPARE (saved.size (), std::size_t {1});
    QCOMPARE (saved[0]->firstSample, Recording::searchStepSamples);
    QCOMPARE (saved[0]->endSample (), qint64 {8000});
    QCOMPARE (saved[0]->firstSampleUtc, origin ().addMSecs (Recording::searchStepSamples / 12));
    QVERIFY (std::equal (saved[0]->samples.begin (), saved[0]->samples.end (),
                         pcm.begin () + Recording::searchStepSamples));
  }

  void slowWriterCannotRetainUnboundedAudio ()
  {
    std::vector<Recording::Completion> completions;
    int errors = 0;
    Recording recorder {nullptr, [&completions] (Recording::Snapshot, Recording::Completion done) {
      completions.push_back (done);
    }};
    recorder.setErrorHandler ([&errors] (QString const&) {++errors;});
    auto const pcm = audio (1000);
    for (quint64 reception = 1; reception <= 3; ++reception) {
      recorder.beginReception (reception, origin (), settings (Recording::SavePolicy::All));
      recorder.feed (0, pcm.data (), 1000);
      recorder.endReception ();
    }
    QCOMPARE (completions.size (), std::size_t {Recording::maximumSnapshots});
    QCOMPARE (recorder.snapshotsInFlight (), Recording::maximumSnapshots);
    QVERIFY (recorder.suspendedAfterError ());
    QCOMPARE (errors, 1);
    for (auto const& done : completions) done ({});
    QCOMPARE (recorder.snapshotsInFlight (), 0);
  }

  void discontinuityRequiresANewReception ()
  {
    int errors = 0;
    Recording recorder {nullptr, [] (Recording::Snapshot, Recording::Completion done) {done ({});}};
    recorder.setErrorHandler ([&errors] (QString const&) {++errors;});
    recorder.beginReception (1, origin (), settings (Recording::SavePolicy::All));
    auto const pcm = audio (1000);
    recorder.feed (0, pcm.data (), 1000);
    recorder.feed (1001, pcm.data (), 1000);
    QVERIFY (recorder.suspendedAfterError ());
    QCOMPARE (errors, 1);
  }

  void writerPreservesCaptureTimeAndNeverOverwrites ()
  {
    QTemporaryDir directory;
    QVERIFY (directory.isValid ());
    Recording::Segment segment;
    segment.settings = settings (Recording::SavePolicy::All);
    segment.fileName = directory.filePath (QStringLiteral ("260911_235842_test.wav"));
    segment.firstSampleUtc = origin ();
    segment.samplesSinceMidnight = quint64 (origin ().time ().msecsSinceStartOfDay ()) * 12 + 7;
    segment.samples = audio (1000);
    QVERIFY2 (Recording::writeSegment (segment).isEmpty (), "initial write failed");
    BWFFile wav {QAudioFormat {}, segment.fileName};
    QVERIFY (wav.open (QIODevice::ReadOnly));
    QCOMPARE (wav.bext_time_reference (), segment.samplesSinceMidnight);
    QCOMPARE (wav.bext_origination_date_time (), origin ().addMSecs (-origin ().time ().msec ()));
    QCOMPARE (wav.size (), qint64 {2000});
    wav.close ();
    segment.samples.resize (10);
    QVERIFY (!Recording::writeSegment (segment).isEmpty ());
    QVERIFY (wav.open (QIODevice::ReadOnly));
    QCOMPARE (wav.size (), qint64 {2000});
  }
};

QTEST_GUILESS_MAIN (TestJttyRecording)
#include "test_jtty_recording.moc"
