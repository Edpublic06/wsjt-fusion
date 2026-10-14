#ifndef JTTY_RECORDING_HPP
#define JTTY_RECORDING_HPP

#include "JttyReceiveTiming.hpp"

#include <functional>
#include <memory>
#include <vector>

#include <QDateTime>
#include <QCoreApplication>
#include <QObject>
#include <QString>

class JttyRecording final : public QObject
{
  Q_DECLARE_TR_FUNCTIONS (JttyRecording)
public:
  static constexpr qint64 sampleRate = 12000;
  static constexpr qint64 searchStepSamples = Jtty::receiveFrameSamples / 4;
  static constexpr qint64 segmentSamples = 381 * searchStepSamples;
  static constexpr qint64 hopSamples = 296 * searchStepSamples;
  static constexpr int maximumSnapshots = 2;

  enum class SavePolicy {Off, All, Decoded};
  struct Settings
  {
    QString directory;
    QString myCall;
    QString myGrid;
    QString hisCall;
    QString hisGrid;
    QString mode {QStringLiteral ("JTTY")};
    qint32 subMode {0};
    quint64 frequency {0};
    SavePolicy policy {SavePolicy::Off};
  };
  struct Segment
  {
    quint64 reception {0};
    quint64 id {0};
    qint64 firstSample {0};
    QDateTime firstSampleUtc;
    quint64 samplesSinceMidnight {0};
    QString fileName;
    Settings settings;
    std::vector<short> samples;
    bool decoded {false};

    qint64 endSample () const {return firstSample + qint64 (samples.size ());}
  };
  using Snapshot = std::shared_ptr<Segment const>;
  using Completion = std::function<void (QString const& error)>;
  // Writers retain immutable audio and complete on the owner's thread.
  using Writer = std::function<void (Snapshot, Completion)>;

  explicit JttyRecording (QObject * parent = nullptr, Writer writer = {});
  void setErrorHandler (Completion handler);
  void beginReception (quint64 reception, QDateTime const& sampleZeroUtc,
                       Settings const& settings);
  void feed (qint64 firstSample, short const * samples, int count);
  void noteDecoded (qint64 firstSample, qint64 endSample);
  // No future decoder evidence may refer to samples before this watermark.
  void advanceDecoderWatermark (qint64 finalizedBefore);
  // Call after the final valid decode windows and updates have been drained.
  void endReception ();
  void updateMetadata (Settings const& settings);
  void updateSavePolicy (SavePolicy policy);
  bool suspendedAfterError () const {return blocked_;}
  int snapshotsInFlight () const {return inFlight_;}

  static QString writeSegment (Segment const& segment);

private:
  void startSegment (qint64 firstSample);
  void closeSegments (bool all);
  void drain ();
  void fail (QString const& error);

  Writer writer_;
  Completion errorHandler_;
  Settings settings_;
  QDateTime sampleZeroUtc_;
  quint64 reception_ {0};
  quint64 nextId_ {0};
  quint64 armGeneration_ {0};
  qint64 nextSample_ {-1};
  qint64 nextSegment_ {-1};
  qint64 watermark_ {-1};
  int inFlight_ {0};
  bool receiving_ {false};
  bool blocked_ {false};
  std::vector<std::shared_ptr<Segment>> active_;
  std::vector<std::shared_ptr<Segment>> pending_;
};

#endif
