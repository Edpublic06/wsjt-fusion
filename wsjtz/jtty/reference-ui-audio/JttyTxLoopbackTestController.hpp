// -*- Mode: C++ -*-
#ifndef JTTY_TX_LOOPBACK_TEST_CONTROLLER_HPP__
#define JTTY_TX_LOOPBACK_TEST_CONTROLLER_HPP__

#include <QObject>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVector>

class FixtureSoundOutput;
class MainWindow;

class JttyTxLoopbackTestController final
  : public QObject
{
  Q_OBJECT

public:
  JttyTxLoopbackTestController (MainWindow * window,
                                FixtureSoundOutput * output,
                                QString capturePath,
                                QObject * parent = nullptr);

  void begin ();
  bool succeeded () const {return m_succeeded;}

private:
  static QString contestExchangeMessage ();
  static QStringList longMessageSegments ();
  static QString unsentDraft ();
  static qint64 encodedSampleFrames (QString const& message);
  void prepareWhenReady ();
  void submitSecondMessage ();
  void observeTransmittedDisplay ();
  bool verifyCancellationPaths ();
  void maybeFinish ();
  bool verifyModeControlsEnabled (bool expected, QString * error) const;
  bool validateCapture (QString * error) const;
  void fail (QString const& reason);
  void checkForUnexpectedModal ();

  MainWindow * m_window;
  FixtureSoundOutput * m_output;
  QString m_capturePath;
  QSet<qint64> m_acceptedRequests;
  QSet<qint64> m_completedRequests;
  QSet<qint64> m_cancelledRequests;
  QVector<qint64> m_acceptedOrder;
  QVector<qint64> m_completedOrder;
  QVector<qint64> m_displayedFrames;
  QVector<qint64> m_segmentEndFrames;
  QString m_displayError;
  QTimer m_timeout;
  QTimer m_prepareTimer;
  QTimer m_modalTimer;
  qint64 m_firstRequestId {0};
  qint64 m_secondRequestId {0};
  qint64 m_modeChangeProbeId {0};
  qint64 m_capturedFrames {0};
  qint64 m_expectedAudioFrames {0};
  qint64 m_firstNonSilentFrame {0};
  int m_captureStartCount {0};
  int m_captureStopCount {0};
  int m_sessionDrainCount {0};
  bool m_prepared {false};
  bool m_modeChangeProbeStarted {false};
  bool m_modeChangeProbeComplete {false};
  bool m_modeChangeProbeSwitching {false};
  bool m_submittingModeChangeProbe {false};
  bool m_modeChangeProbeUnavailable {false};
  bool m_nonSilentAudioSeen {false};
  bool m_checkingCancellation {false};
  bool m_cancellationChecked {false};
  bool m_finished {false};
  bool m_succeeded {false};
};

#endif
