#include "JttyRecording.hpp"

#include <algorithm>
#include <limits>
#include <utility>

#include <QAudioFormat>
#include <QDir>
#include <QFile>
#include <QFutureWatcher>
#include <QPointer>
#include <QtConcurrent/QtConcurrentRun>

#include "Audio/BWFFile.hpp"

constexpr qint64 JttyRecording::sampleRate;
constexpr qint64 JttyRecording::searchStepSamples;
constexpr qint64 JttyRecording::segmentSamples;
constexpr qint64 JttyRecording::hopSamples;
constexpr int JttyRecording::maximumSnapshots;

JttyRecording::JttyRecording (QObject * parent, Writer writer)
  : QObject {parent}
  , writer_ {std::move (writer)}
{
  if (!writer_) {
    writer_ = [this] (Snapshot segment, Completion complete) {
      auto * watcher = new QFutureWatcher<QString> {this};
      connect (watcher, &QFutureWatcher<QString>::finished, this,
               [watcher, complete] {
        QString const error = watcher->result ();
        watcher->deleteLater ();
        complete (error);
      });
      watcher->setFuture (QtConcurrent::run ([segment] {
        return writeSegment (*segment);
      }));
    };
  }
}

void JttyRecording::setErrorHandler (Completion handler)
{
  errorHandler_ = std::move (handler);
}

void JttyRecording::beginReception (quint64 reception, QDateTime const& sampleZeroUtc,
                                    Settings const& settings)
{
  endReception ();
  reception_ = reception;
  sampleZeroUtc_ = sampleZeroUtc.toUTC ();
  settings_ = settings;
  nextSample_ = -1;
  nextSegment_ = -1;
  watermark_ = -1;
  receiving_ = true;
}

void JttyRecording::startSegment (qint64 firstSample)
{
  auto segment = std::make_shared<Segment> ();
  segment->reception = reception_;
  segment->id = ++nextId_;
  segment->firstSample = firstSample;
  segment->firstSampleUtc = sampleZeroUtc_.addMSecs (firstSample / 12);
  constexpr qint64 samplesPerDay = 86400 * sampleRate;
  segment->samplesSinceMidnight = quint64 (
      (qint64 (sampleZeroUtc_.time ().msecsSinceStartOfDay ()) * 12
       + firstSample) % samplesPerDay);
  segment->settings = settings_;
  segment->fileName = QDir {settings_.directory}.absoluteFilePath (
      segment->firstSampleUtc.toString (QStringLiteral ("yyMMdd_hhmmss_zzz"))
      + QStringLiteral (".wav"));
  segment->samples.reserve (std::size_t (segmentSamples));
  active_.push_back (std::move (segment));
}

void JttyRecording::feed (qint64 firstSample, short const * samples, int count)
{
  if (!receiving_ || blocked_ || settings_.policy == SavePolicy::Off || count <= 0) return;
  if (!samples || firstSample < 0 || !sampleZeroUtc_.isValid ()) {
    fail (tr ("JTTY recording received invalid audio coordinates."));
    return;
  }
  if (nextSample_ < 0) {
    nextSample_ = firstSample;
    // A file's sample zero must preserve the live decoder's quarter-frame search phase.
    nextSegment_ = firstSample + (searchStepSamples - firstSample % searchStepSamples) % searchStepSamples;
  }
  if (firstSample != nextSample_) {
    fail (tr ("JTTY recording received discontinuous audio."));
    return;
  }
  qint64 offset = 0;
  while (offset < count && !blocked_) {
    if (nextSample_ == nextSegment_) {
      startSegment (nextSample_);
      nextSegment_ += hopSamples;
    }
    qint64 amount = qMin (qint64 (count) - offset, nextSegment_ - nextSample_);
    for (auto const& segment : active_) {
      amount = qMin (amount, segment->firstSample + segmentSamples - nextSample_);
    }
    for (auto& segment : active_) {
      segment->samples.insert (segment->samples.end (), samples + offset,
                                samples + offset + amount);
    }
    offset += amount;
    nextSample_ += amount;
    closeSegments (false);
    drain ();
  }
}

void JttyRecording::closeSegments (bool all)
{
  for (auto it = active_.begin (); it != active_.end ();) {
    auto const& segment = *it;
    if (!all && qint64 (segment->samples.size ()) < segmentSamples) {
      ++it;
      continue;
    }
    if (!segment->samples.empty ()) pending_.push_back (segment);
    it = active_.erase (it);
  }
}

void JttyRecording::noteDecoded (qint64 firstSample, qint64 endSample)
{
  if (endSample <= firstSample) return;
  auto qualify = [firstSample, endSample] (std::shared_ptr<Segment> const& segment) {
    if (firstSample < segment->endSample () && endSample > segment->firstSample) {
      segment->decoded = true;
    }
  };
  for (auto const& segment : active_) qualify (segment);
  for (auto const& segment : pending_) qualify (segment);
}

void JttyRecording::advanceDecoderWatermark (qint64 finalizedBefore)
{
  watermark_ = qMax (watermark_, finalizedBefore);
  drain ();
}

void JttyRecording::endReception ()
{
  if (!receiving_) return;
  closeSegments (true);
  watermark_ = (std::numeric_limits<qint64>::max) ();
  drain ();
  receiving_ = false;
}

void JttyRecording::updateMetadata (Settings const& settings)
{
  auto const policy = settings_.policy;
  settings_ = settings;
  settings_.policy = policy;
}

void JttyRecording::updateSavePolicy (SavePolicy policy)
{
  if (settings_.policy != policy || blocked_) {
    closeSegments (true);
    settings_.policy = policy;
    nextSample_ = nextSegment_ = -1;
    blocked_ = false;
    ++armGeneration_;
    drain ();
  }
}

void JttyRecording::drain ()
{
  if (blocked_) return;
  for (auto it = pending_.begin (); it != pending_.end ();) {
    auto segment = *it;
    bool const finalized = segment->settings.policy == SavePolicy::All
      || segment->endSample () <= watermark_;
    if (!finalized) {
      ++it;
      continue;
    }
    it = pending_.erase (it);
    if (segment->settings.policy == SavePolicy::Decoded && !segment->decoded) continue;
    if (inFlight_ == maximumSnapshots) {
      fail (tr ("JTTY recording stopped because the disk writer could not keep up."));
      return;
    }
    ++inFlight_;
    quint64 const generation = armGeneration_;
    QPointer<JttyRecording> guard {this};
    writer_ (std::move (segment), [guard, generation] (QString const& error) {
      if (!guard) return;
      --guard->inFlight_;
      if (!error.isEmpty () && generation == guard->armGeneration_) guard->fail (error);
    });
    if (blocked_) return;
  }
  if (int (pending_.size ()) + inFlight_ > maximumSnapshots) {
    fail (tr ("JTTY recording stopped because decoder finalization could not keep up."));
  }
}

void JttyRecording::fail (QString const& error)
{
  if (blocked_) return;
  blocked_ = true;
  active_.clear ();
  pending_.clear ();
  nextSample_ = nextSegment_ = -1;
  if (errorHandler_) errorHandler_ (error);
}

QString JttyRecording::writeSegment (Segment const& segment)
{
  QAudioFormat format;
  format.setCodec (QStringLiteral ("audio/pcm"));
  format.setSampleRate (sampleRate);
  format.setChannelCount (1);
  format.setSampleSize (16);
  format.setSampleType (QAudioFormat::SignedInt);
  auto const& settings = segment.settings;
  BWFFile::InfoDictionary info {
    {{{'I','S','R','C'}}, (settings.myCall + "; " + settings.myGrid).toLocal8Bit ()},
    {{{'I','C','R','D'}}, segment.firstSampleUtc.toString ("yyyy-MM-ddTHH:mm:ss.zzzZ").toLatin1 ()},
    {{{'I','C','M','T'}}, QStringLiteral ("Mode=%1; Freq=%2; DXCall=%3; DXGrid=%4; Reception=%5; FirstSample=%6")
      .arg (settings.mode).arg (double (settings.frequency) / 1000000., 0, 'f', 6)
      .arg (settings.hisCall).arg (settings.hisGrid).arg (segment.reception)
      .arg (segment.firstSample).toLocal8Bit ()},
    {{{'D','G','R','D'}}, QByteArrayLiteral ("jtty")}
  };
  BWFFile wav {format, segment.fileName, info};
  wav.bext_origination_date_time (segment.firstSampleUtc);
  wav.bext_time_reference (segment.samplesSinceMidnight);
  wav.bext_originator (settings.myCall.toLatin1 ());
  if (!wav.open (QIODevice::WriteOnly | QIODevice::NewOnly)) {
    return segment.fileName + ": " + wav.errorString ();
  }
  qint64 const bytes = qint64 (segment.samples.size ()) * sizeof (short);
  bool const written = wav.write (reinterpret_cast<char const *> (segment.samples.data ()), bytes) == bytes;
  QString const writeError = written ? QString {} : wav.errorString ();
  bool const finalized = wav.finalize ();
  bool const success = written && finalized;
  QString const error = success ? QString {} : segment.fileName + ": "
    + (written ? wav.errorString () : writeError);
  if (!success) QFile::remove (segment.fileName);
  return error;
}
