// Fusion local-audio adapter for WSJT-X v3.3.0-beta1 JTTY transmission.
// Codec, FIFO, stream and lifecycle remain upstream components. GPL-3.0-or-later.
#include "mainwindow.h"
#include "JttyTxLifecycle.hpp"
#include "Modulator/JttyTxStream.hpp"
#include "ui_mainwindow.h"
#include "wsjtz/jtty/tx/Encode.hpp"
#include "wsjtz/jtty/ui/TransmitPanel.hpp"
#include "JttyMessages.hpp"
#include "Audio/soundout.h"
#include "Radio.hpp"
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStatusBar>
extern int g_iptt;

void MainWindow::initializeJttyTransmit ()
{
  m_jttyTxLifecycle.reset (new JttyTxLifecycle);
  qRegisterMetaType<TxAudioQueueEpoch> ("TxAudioQueueEpoch");
  qRegisterMetaType<TxAudioQueueProgress> ("TxAudioQueueProgress");
  qRegisterMetaType<TxAudioQueueDrainState> ("TxAudioQueueDrainState");
  qRegisterMetaType<TxAudioQueueEnqueueFailure> ("TxAudioQueueEnqueueFailure");
  m_jttyTxQueue.reset (new TxAudioQueue);
  m_jttyTxStream = new JttyTxStream {*m_jttyTxQueue};
  m_jttyTxStream->moveToThread (&m_audioThread);
  connect (&m_audioThread,&QThread::finished,m_jttyTxStream,&QObject::deleteLater);
  connect (m_jttyTxStream,&JttyTxStream::enqueueAccepted,this,
    [this] (qint64 id,qint64,TxAudioQueueProgress progress) {
      auto accepted = m_jttyTxLifecycle->accept (progress.epoch,id,progress);
      if (!m_jttyActive || !accepted.pending) return;
      if (m_jttyPanel) { m_jttyPanel->clearEntry (); m_jttyPanel->setProgress (0,progress.total_samples); }
      m_jttyReturnToRx = m_monitoring;
      m_transmitting = true; m_btxok = true; g_iptt = 1;
      m_currentMessage = m_jttyTxText;
      monitor (false);
      transmitDisplay (true);
      updateJttyTxControls (true);
      tx_status_label.setText (tr ("JTTY: waiting for PTT"));
      // Use the same rig acknowledgement -> txDelay -> startTx2 path as WSJT-X.
      m_tx_when_ready = true;
      m_config.transceiver_ptt (true);
      auto epoch = progress.epoch;
      QTimer::singleShot (10000 + int (1000*m_config.txDelay ()),this,[this,epoch] {
        if (m_jttyTxLifecycle->epoch () == epoch && m_jttyTxLifecycle->phase () == JttyTxLifecycle::Phase::Ready) {
          stopJttyTransmit ();
          statusBar ()->showMessage (tr ("JTTY stopped: no transmit acknowledgement from the radio."),15000);
        }
      });
    });
  connect (m_jttyTxStream,&JttyTxStream::enqueueFailed,this,
    [this] (TxAudioQueueEpoch epoch,qint64 id,TxAudioQueueEnqueueFailure) {
      if (!m_jttyTxLifecycle->fail (epoch,id).pending) return;
      stopJttyTransmit ();
      statusBar ()->showMessage (tr ("JTTY audio queue rejected the message."),15000);
    });
  connect (m_jttyTxStream,&JttyTxStream::progress,this,[this] (TxAudioQueueProgress progress) {
    if (m_jttyPanel && progress.epoch == m_jttyTxLifecycle->epoch ())
      m_jttyPanel->setProgress (progress.served_samples,progress.total_samples);
  });
  connect (m_jttyTxStream,&JttyTxStream::drained,this,[this] (TxAudioQueueDrainState drain) {
    if (m_jttyTxLifecycle->observeDrain (drain.epoch,drain.total_at_drain)) stopJttyTransmit ();
  });

}

void MainWindow::updateJttyTxControls (bool busy)
{
  if (m_jttyPanel) m_jttyPanel->setBusy (busy);
  ui->bandComboBox->setEnabled (!busy);
  ui->TxFreqSpinBox->setEnabled (!busy);
  ui->RxFreqSpinBox->setEnabled (!busy);
  ui->pbR2T->setEnabled (!busy);
  ui->pbT2R->setEnabled (!busy);
  ui->cbHoldTxFreq->setEnabled (!busy);
  ui->actionSettings->setEnabled (!busy);
}

void MainWindow::sendJttyText (QString const& text)
{
  if (!m_jttyActive || m_jttyTxLifecycle->active ()) return;
  if (m_tci_audio || m_config.audio_output_device ().isNull ()) {
    statusBar ()->showMessage (tr ("JTTY requires a configured local audio output device."),15000); return;
  }
  if (!m_config.is_transceiver_online ()) {
    statusBar ()->showMessage (tr ("Connect the radio before sending JTTY."),15000); return;
  }
  setXIT (ui->TxFreqSpinBox->value ());
  auto encoded = wsjtz::encodeJtty (text,ui->TxFreqSpinBox->value () - m_XIT);
  if (!encoded.error.isEmpty ()) { statusBar ()->showMessage (encoded.error,15000); return; }
  auto epoch = m_jttyTxLifecycle->begin (JttyTxLifecycle::Backend::Local);
  auto id = ++m_jttyEnqueueId;
  if (!epoch.isValid () || !m_jttyTxLifecycle->addPending (id,id)) return;
  m_jttyTxText = encoded.text;
  m_jttyReturnToRx = m_monitoring;
  updateJttyTxControls (true);
  // clear/reset and enqueue run in order on the one audio consumer thread.
  QMetaObject::invokeMethod (m_jttyTxStream,[this,epoch,bytes=encoded.pcm,id] {
    m_jttyTxStream->clearQueue (epoch);
    m_jttyTxStream->enqueuePcm (bytes,epoch,id);
  },Qt::QueuedConnection);
  QTimer::singleShot (10000,this,[this,epoch] {
    if (m_jttyTxLifecycle->epoch () == epoch && m_jttyTxLifecycle->hasPending ()) {
      stopJttyTransmit ();
      statusBar ()->showMessage (tr ("JTTY stopped: audio queue did not acknowledge the message."),15000);
    }
  });
  if (encoded.substituted) {
    if (m_jttyPanel) m_jttyPanel->showPreparedText (encoded.text);
    statusBar ()->showMessage (tr ("Unsupported characters were replaced; the editor shows the transmitted text."),15000);
  }
}

void MainWindow::startJttyTransmitAudio ()
{
  if (!m_jttyActive || !m_jttyTxLifecycle->markTransmitting ()) return;
  auto epoch = m_jttyTxLifecycle->epoch ();
  TxEvidence::TxRequest request;
  request.mode = "JTTY"; request.queue_epoch = epoch;
  request.channel = m_config.audio_output_channel ();
  request.synchronize = false;
  request.session_id = TxEvidence::TxSessionId {epoch.value ()};
  request.generation = TxEvidence::TxGeneration {epoch.value ()};
  QMetaObject::invokeMethod (m_jttyTxStream,[this,request] { m_jttyTxStream->start (request,m_soundOutput); },Qt::QueuedConnection);
  tx_status_label.setText (tr ("Tx: JTTY"));
  m_jttyLive->appendTransmit (m_jttyTxText,ui->TxFreqSpinBox->value ());
  write_all ("Tx",m_jttyTxText);
  // Completion comes from the upstream audio queue's device-buffer drain guard.
  int duration = int (m_jttyTxLifecycle->committedTotal () / 48) + 15000;
  QTimer::singleShot (duration,this,[this,epoch] {
    if (m_jttyTxLifecycle->epoch () == epoch && m_jttyTxLifecycle->active ()) {
      stopJttyTransmit ();
      statusBar ()->showMessage (tr ("JTTY stopped: audio completion watchdog expired."),15000);
    }
  });
}

void MainWindow::stopJttyTransmit ()
{
  auto context = m_jttyTxLifecycle->beginStop ();
  if (!context || m_jttyTxLifecycle->backendStopRouted ()) return;
  m_jttyTxLifecycle->markBackendStopRouted (*context);
  m_jttyTxLifecycle->failAll ();
  ptt1Timer.stop (); ptt0Timer.stop ();
  m_tx_when_ready = false;
  m_btxok = false; m_btxok0 = false; g_iptt = 0; m_iptt0 = 0;
  m_transmitting = false;
  // Stop/flush audio before acknowledging stop. PTT release is immediate on abort.
  auto epoch = context->epoch;
  QMetaObject::invokeMethod (m_jttyTxStream,[this,epoch] {
    m_jttyTxStream->stop ();
    QMetaObject::invokeMethod (this,[this,epoch] {
      if (m_jttyTxLifecycle->epoch () != epoch || m_jttyTxLifecycle->phase () != JttyTxLifecycle::Phase::Stopping) return;
      m_jttyTxLifecycle->reset ();
      updateJttyTxControls (false);
      if (m_jttyActive && m_jttyReturnToRx && m_valid) monitor (true);
    },Qt::QueuedConnection);
  },Qt::QueuedConnection);
  m_config.transceiver_ptt (false);
  transmitDisplay (false);
  tx_status_label.setText ("");
  statusUpdate ();
}

void MainWindow::sendJttyMacro (QString const& macro)
{
  if (!m_jttyPanel) return;
  Jtty::NativeMacroContext context;
  context.myCall=m_config.my_callsign (); context.grid=m_config.my_grid ();
  context.hisCall=ui->dxCallEntry->text ().trimmed ().toUpper ();
  context.serialNumber=m_jttyPanel->serial (); context.snr=m_jttyHisCallSnr;
  if ((macro.contains ("%H") && context.hisCall.isEmpty ()) ||
      (macro.contains ("%Q") && m_jttyPanel->nextCall ().isEmpty ())) {
    statusBar ()->showMessage (tr ("Enter the other station's callsign before sending this message."),10000); return;
  }
  auto expanded=macro;
  expanded.replace ("%Q",m_jttyPanel->nextCall ());
  expanded=Jtty::expandLiteralMacro (expanded,context);
  sendJttyText (expanded);
}

void MainWindow::selectJttyCorrespondent (QString const& line,QString const& word,bool sendF2)
{
  Q_UNUSED (line);
  if (!m_jttyActive || !Radio::is_callsign (word)) return;
  // Upstream handleDecodeSelection JTTY branch: DX Call changes first, then
  // restore the matching received SNR (the field-change handler resets it).
  set_dateTimeQSO (-1);
  m_deCall = word;
  ui->dxCallEntry->setText (m_deCall);
  m_jttyHisCallSnr = m_jttyLive->snrForSelectedWord (word);
  // Fusion has a separate Call Info DX Call field, absent from upstream.
  // Use the same lookup path as structured-mode correspondent selection.
  dxLookup (word,QString {});
  if (!ui->w_callInfo->isVisible ()) {
    clearCallInfo ();
    ui->ci_dxcall->setText (word);
  }
  if (sendF2 && m_jttyPanel && m_jttyPanel->isFt8Style ())
    m_jttyPanel->triggerMacro (2);
}
