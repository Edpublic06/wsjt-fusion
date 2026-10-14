#include "Detector.hpp"
#include <algorithm>
#include <QDateTime>
#include <QtAlgorithms>
#include <QDebug>
#include <QMutexLocker>
#include <QVector>
#include <QThread>
#include <cmath>
#include <cstddef>
#include "commons.h"
#include "DecDataMutex.hpp"

#include "moc_Detector.cpp"

extern "C" {
  void   fil4_state_(qint16*, qint32*, qint16*, qint32*, float*);
}

#include "ReceiveAudio.hpp"

Detector::Detector (unsigned frameRate, double periodLengthInSeconds,
                    unsigned downSampleFactor, QObject * parent)
  : AudioDevice (parent)
  , m_frameRate (frameRate)
  , m_period (periodLengthInSeconds)
  , m_downSampleFactor (downSampleFactor)
  , m_samplesPerFFT {max_buffer_size}
  , m_buffer ((downSampleFactor > 1) ?
              new short [max_buffer_size * downSampleFactor] : nullptr)
  , m_bufferPos (0)
{
  (void)m_frameRate;            // quell compiler warning
  clear ();
}

void Detector::setBlockSize (unsigned n)
{
  if (QThread::currentThread () != thread ())
    {
      QMetaObject::invokeMethod (this, [this, n] { setBlockSize (n); },
                                 Qt::QueuedConnection);
      return;
    }
  if (!n || n > max_buffer_size || n == unsigned (m_samplesPerFFT)) return;
  // Finish partial continuous audio before changing the block's write boundary.
  endContinuousReception (JttyReceiveReason::SourceChanged);
  m_bufferPos = 0;
  m_samplesPerFFT = n;
}

void Detector::setTRPeriod (double period)
{
  if (QThread::currentThread () != thread ())
    {
      QMetaObject::invokeMethod (this, [this, period] { setTRPeriod (period); },
                                 Qt::QueuedConnection);
      return;
    }
  if (m_period != period)
    {
      m_period = period;
      m_last_period_offset_ms = -1;
      if (m_receivePolicy == ReceivePolicy::Timed) clear ();
    }
}

void Detector::setReceivePolicy (ReceivePolicy policy)
{
  if (QThread::currentThread () != thread ())
    {
      QMetaObject::invokeMethod (this, [this, policy] { setReceivePolicy (policy); },
                                 Qt::QueuedConnection);
      return;
    }
  if (policy == m_receivePolicy) return;
  bool const endingContinuous = m_receivePolicy == ReceivePolicy::ContinuousJtty;
  endContinuousReception (JttyReceiveReason::ModeChanged);
  m_receivePolicy = policy;
  m_last_period_offset_ms = -1;
  m_downsampleState.fill (0);
  clear ();
  if (endingContinuous) Q_EMIT continuousReceptionStopped (m_jttyPublisher.mailbox ());
}

void Detector::setReceiveContext (quint64 context)
{
  if (QThread::currentThread () != thread ())
    {
      QMetaObject::invokeMethod (this, [this, context] { setReceiveContext (context); },
                                 Qt::QueuedConnection);
      return;
    }
  if (context == m_receiveContext) return;
  endContinuousReception (JttyReceiveReason::SourceChanged);
  m_receiveContext = context;
  m_jttyPublisher.setContext (context);
}

void Detector::setInputStopReason (JttyReceiveReason reason)
{
  if (QThread::currentThread () != thread ())
    {
      QMetaObject::invokeMethod (this, [this, reason] { setInputStopReason (reason); },
                                 Qt::QueuedConnection);
      return;
    }
  m_inputStopReason = reason;
}

void Detector::finishInput ()
{
  endContinuousReception (m_inputStopReason);
  m_inputStopReason = JttyReceiveReason::MonitorStopped;
  if (m_receivePolicy == ReceivePolicy::ContinuousJtty)
    Q_EMIT continuousReceptionStopped (m_jttyPublisher.mailbox ());
}

void Detector::inputInterrupted ()
{
  endContinuousReception (JttyReceiveReason::InputError);
}

void Detector::requestContinuousReceiveDrain (quint64 requestId)
{
  if (QThread::currentThread () != thread ())
    {
      QMetaObject::invokeMethod (this, [this, requestId] { requestContinuousReceiveDrain (requestId); },
                                 Qt::QueuedConnection);
      return;
    }
  Q_EMIT continuousReceiveDrained (requestId, m_jttyPublisher.mailbox ());
}

void Detector::endContinuousReception (JttyReceiveReason reason)
{
  if (QThread::currentThread () != thread ())
    {
      QMetaObject::invokeMethod (this, [this, reason] { endContinuousReception (reason); },
                                 Qt::QueuedConnection);
      return;
    }
  if (m_receivePolicy != ReceivePolicy::ContinuousJtty) return;
  bool notify = false;
  if (m_jttyPublisher.active () && m_bufferPos)
    {
      qint32 inputFrames = m_bufferPos - m_bufferPos % m_downSampleFactor;
      qint32 outputFrames = inputFrames / m_downSampleFactor;
      if (m_downSampleFactor > 1 && inputFrames)
        fil4_state_ (m_buffer.data (), &inputFrames, m_jttyOutput.data (),
                     &outputFrames, m_downsampleState.data ());
      notify = m_jttyPublisher.append (m_jttyOutput.data (), outputFrames);
    }
  notify = m_jttyPublisher.end (reason) || notify;
  m_bufferPos = 0;
  m_downsampleState.fill (0);
  if (notify) Q_EMIT continuousAudioAvailable (m_jttyPublisher.mailbox ());
}

void Detector::setStreamDescriptor (AudioStreamDescriptor descriptor)
{
  endContinuousReception (JttyReceiveReason::SourceChanged);
  m_stream_clock.setDescriptor (descriptor);
  m_last_period_offset_ms = -1;
  clear ();
}

void Detector::flushBufferedFrames (qint64 frameLimit)
{
  if (m_receivePolicy == ReceivePolicy::ContinuousJtty)
    {
      endContinuousReception (JttyReceiveReason::MonitorStopped);
      return;
    }
  qint64 framesWritten {0};
  ReceiveAudio audio;
  {
    QMutexLocker lock {&dec_data_mutex ()};
    auto& producer = m_receiveAudioProducer.data ();
    if (dec_data_input_blocked ()) return;
    if (m_downSampleFactor > 1 && m_bufferPos
        && m_receiveAudioProducer.frames () < frameLimit)
      {
        auto const blockFrames = m_samplesPerFFT * m_downSampleFactor;
        std::fill (m_buffer.data () + m_bufferPos,
                   m_buffer.data () + blockFrames, 0);
        qint32 framesToProcess = blockFrames;
        qint32 framesAfterDownSample = m_samplesPerFFT;
        fil4_state_ (m_buffer.data (), &framesToProcess,
                     &producer.d2[m_receiveAudioProducer.frames ()],
                     &framesAfterDownSample, m_downsampleState.data ());
        m_receiveAudioProducer.setFrames (std::min<qint64> (
          frameLimit, m_receiveAudioProducer.frames () + framesAfterDownSample));
      }
    framesWritten = std::min<qint64> (frameLimit, m_receiveAudioProducer.frames ());
    if (framesWritten <= m_receiveAudioProducer.capturedEnd ()) return;
    audio = m_receiveAudioProducer.capture (framesWritten, m_period);
    m_bufferPos = 0;
  }
  Q_EMIT this->framesWritten (framesWritten);
  Q_EMIT audioBlock (audio);
}

bool Detector::reset ()
{
  endContinuousReception (JttyReceiveReason::SourceChanged);
  clear ();
  // don't call base class reset because it calls seek(0) which causes
  // a warning
  return isOpen ();
}

void Detector::clear ()
{
  QMutexLocker lock {&dec_data_mutex ()};
  m_bufferPos = 0;
  if (dec_data_input_blocked ()) return;

  resetPeriodBuffer ();
}

void Detector::resetPeriodBuffer ()
{
  m_receiveAudioProducer.reset (m_period);
  m_bufferPos = 0;
}

qint64 Detector::writeData (char const * data, qint64 maxSize)
{
  auto const bytes_per_frame = static_cast<qint64> (bytesPerFrame ());
  Q_ASSERT (!(maxSize % bytes_per_frame));
  qint64 const frames_received = maxSize / bytes_per_frame;
  QVector<qint64> frame_counts;
  QVector<ReceiveAudio> audio;
  bool continuousNotify = false;
  bool const continuous = m_receivePolicy == ReceivePolicy::ContinuousJtty;
  qint64 const now_ms = m_stream_clock.timestamp (
    QDateTime::currentMSecsSinceEpoch ());
  m_stream_clock.advance (frames_received);
  qint64 const period_ms = static_cast<qint64> (1000.0 * m_period);
  qint64 const day_ms = now_ms % 86400000;
  qint64 const mstr = continuous ? 0 : day_ms % period_ms;

  if (dec_data_input_blocked ())
    {
      endContinuousReception (JttyReceiveReason::Replay);
      return maxSize;
    }

  {
    QMutexLocker lock {&dec_data_mutex ()};
    auto& producer = m_receiveAudioProducer.data ();
    if (dec_data_input_blocked ())
      {
        endContinuousReception (JttyReceiveReason::Replay);
        return maxSize;
      }
    if(!continuous && m_last_period_offset_ms >= 0 && mstr < m_last_period_offset_ms) {
      resetPeriodBuffer ();
    }
    m_last_period_offset_ms = mstr;
    if (continuous) continuousNotify = m_jttyPublisher.begin (now_ms);

    // these are in terms of input frames (not down sampled)
    size_t framesAcceptable (continuous ? frames_received : (sizeof producer.d2 /
                              sizeof producer.d2[0] - m_receiveAudioProducer.frames ()) * m_downSampleFactor);
    size_t framesAccepted (qMin (static_cast<size_t> (frames_received), framesAcceptable));

    if (framesAccepted < static_cast<size_t> (frames_received)) {
      auto const frames_dropped = frames_received
        - static_cast<qint64> (framesAccepted);
      qDebug () << "dropped " << frames_dropped
                  << " frames of data on the floor!"
                  << m_receiveAudioProducer.frames () << mstr;
    }

    for (unsigned remaining = framesAccepted; remaining; ) {
      size_t numFramesProcessed (qMin (m_samplesPerFFT *
                                       m_downSampleFactor - m_bufferPos, remaining));

      if(m_downSampleFactor > 1) {
        store (&data[(framesAccepted - remaining) * bytesPerFrame ()],
               numFramesProcessed, &m_buffer[m_bufferPos]);
        m_bufferPos += numFramesProcessed;

        if(m_bufferPos==m_samplesPerFFT*m_downSampleFactor) {
          qint32 framesToProcess (m_samplesPerFFT * m_downSampleFactor);
          qint32 framesAfterDownSample (m_samplesPerFFT);
          if (continuous) {
            fil4_state_ (m_buffer.data (), &framesToProcess, m_jttyOutput.data (),
                         &framesAfterDownSample, m_downsampleState.data ());
            continuousNotify = m_jttyPublisher.append (m_jttyOutput.data (), framesAfterDownSample)
              || continuousNotify;
          } else if(m_downSampleFactor > 1 && m_receiveAudioProducer.frames ()>=0 &&
             m_receiveAudioProducer.frames () < (NTMAX*12000 - framesAfterDownSample)) {
            fil4_state_(&m_buffer[0], &framesToProcess,
                &producer.d2[m_receiveAudioProducer.frames ()],
                &framesAfterDownSample, m_downsampleState.data ());
            m_receiveAudioProducer.setFrames (m_receiveAudioProducer.frames () + framesAfterDownSample);
          } else {
            // qDebug() << "framesToProcess     = " << framesToProcess;
            // qDebug() << "receive audio frames = " << m_receiveAudioProducer.frames ();
            // qDebug() << "secondInPeriod      = " << secondInPeriod();
            // qDebug() << "framesAfterDownSample" << framesAfterDownSample;
          }
          if (!continuous) {
            frame_counts << m_receiveAudioProducer.frames ();
            audio << m_receiveAudioProducer.capture (m_receiveAudioProducer.frames (), m_period);
          }
          m_bufferPos = 0;
        }

      } else {
        store (&data[(framesAccepted - remaining) * bytesPerFrame ()],
               numFramesProcessed, continuous ? m_jttyOutput.data () + m_bufferPos
                 : &producer.d2[m_receiveAudioProducer.frames ()]);
        m_bufferPos += numFramesProcessed;
        if (!continuous) m_receiveAudioProducer.setFrames (m_receiveAudioProducer.frames () + numFramesProcessed);
        if (m_bufferPos == static_cast<unsigned> (m_samplesPerFFT)) {
          if (continuous) {
            continuousNotify = m_jttyPublisher.append (m_jttyOutput.data (), m_bufferPos)
              || continuousNotify;
          } else {
            frame_counts << m_receiveAudioProducer.frames ();
            audio << m_receiveAudioProducer.capture (m_receiveAudioProducer.frames (), m_period);
          }
          m_bufferPos = 0;
        }
      }
      remaining -= numFramesProcessed;
    }
  }

  for (auto frames : frame_counts) {
    Q_EMIT framesWritten (frames);
  }
  for (auto const& block : audio) Q_EMIT audioBlock (block);
  if (continuousNotify) Q_EMIT continuousAudioAvailable (m_jttyPublisher.mailbox ());

    // we drop any data past the end of the buffer on the floor until
    // the next period starts
    return maxSize;
}
