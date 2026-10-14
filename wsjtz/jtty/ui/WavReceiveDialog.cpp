// Fusion adapter, 2026-10-14. GPL-3.0-or-later.
// Runs the unchanged WSJT-X receiver in a separate process: no shared DSP scratch,
// no live audio, no transmitter/PTT, and no interference with normal decoding.
#include "WavReceiveDialog.hpp"
#include <QApplication>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QSpinBox>
#include <QTemporaryDir>
#include <QTextCursor>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QtEndian>

namespace {
class WavReceiveDialog final : public QDialog
{
public:
  explicit WavReceiveDialog (QWidget * parent) : QDialog {parent}
  {
    setAttribute (Qt::WA_DeleteOnClose);
    setWindowTitle ("JTTY WAV receive test — experimental");
    resize (760,480);
    auto layout = new QVBoxLayout {this};
    layout->addWidget (new QLabel {"Offline reception only. Live audio and transmission are not connected.",this});
    auto controls = new QHBoxLayout;
    auto open = new QPushButton {"Open WAV…",this};
    auto stop = new QPushButton {"Stop",this};
    auto frequency = new QSpinBox {this};
    frequency->setRange (200,2800); frequency->setValue (1500);
    frequency->setSuffix (" Hz");
    auto tolerance = new QSpinBox {this};
    tolerance->setRange (10,2600); tolerance->setValue (1300);
    tolerance->setSuffix (" Hz");
    controls->addWidget (open);
    controls->addWidget (new QLabel {"Center:",this}); controls->addWidget (frequency);
    controls->addWidget (new QLabel {"Tolerance:",this}); controls->addWidget (tolerance);
    controls->addWidget (stop); layout->addLayout (controls);
    output_ = new QPlainTextEdit {this}; output_->setReadOnly (true);
    layout->addWidget (output_);
    process_.setProcessChannelMode (QProcess::MergedChannels);
    connect (&process_, &QProcess::readyReadStandardOutput, this, [this] {
      auto cursor = output_->textCursor (); cursor.movePosition (QTextCursor::End);
      cursor.insertText (QString::fromLocal8Bit (process_.readAllStandardOutput ()));
      output_->setTextCursor (cursor);
    });
    connect (&process_, &QProcess::errorOccurred,this,[this] (QProcess::ProcessError) {
      output_->appendPlainText ("Receiver error: " + process_.errorString ());
    });
    connect (&process_, QOverload<int,QProcess::ExitStatus>::of (&QProcess::finished),
      this,[this,open] (int code,QProcess::ExitStatus status) {
        output_->appendPlainText (status == QProcess::NormalExit && code == 0
          ? "\nReception test finished." : "\nReceiver stopped or failed.");
        open->setEnabled (true);
      });
    connect (stop,&QPushButton::clicked,this,[this] { process_.kill (); });
    connect (open,&QPushButton::clicked,this,[this,open,frequency,tolerance] {
      if (process_.state () != QProcess::NotRunning) return;
      auto file = QFileDialog::getOpenFileName (this,"JTTY recording (12 kHz mono PCM16)",{},"WAV files (*.wav)");
      if (file.isEmpty ()) return;
      QFile input {file};
      if (!input.open (QIODevice::ReadOnly)) { output_->appendPlainText (input.errorString ()); return; }
      auto h = input.read (44);
      auto u16 = [&h] (int n) { return qFromLittleEndian<quint16> (reinterpret_cast<uchar const *> (h.constData ()+n)); };
      auto u32 = [&h] (int n) { return qFromLittleEndian<quint32> (reinterpret_cast<uchar const *> (h.constData ()+n)); };
      // The upstream command-line reader expects a canonical 44-byte header.
      if (h.size () != 44 || h.left (4) != "RIFF" || h.mid (8,8) != "WAVEfmt "
          || u32 (16) != 16 || u16 (20) != 1 || u16 (22) != 1
          || u32 (24) != 12000 || u16 (32) != 2 || u16 (34) != 16
          || h.mid (36,4) != "data" || u32 (40) == 0 || u32 (40)%2
          || u32 (40) > 180*12000*2 || input.size () < qint64 (44)+u32 (40)) {
        output_->appendPlainText ("Use an uncompressed 12 kHz mono 16-bit WAV with a standard 44-byte header, up to 180 seconds.");
        return;
      }
      if (!temporary_.isValid ()) { output_->appendPlainText ("Cannot create a temporary working folder."); return; }
      auto target = temporary_.filePath ("000000_000000.wav");
      QFile::remove (target);
      if (!QFile::copy (file,target)) { output_->appendPlainText ("Cannot copy the recording."); return; }
      auto tool = QDir {QApplication::applicationDirPath ()}.filePath ("rjtty");
      if (!QFileInfo {tool}.isExecutable ()) {
        output_->appendPlainText ("The rjtty receiver is missing from the application bundle."); return;
      }
      output_->clear ();
      output_->appendPlainText ("Decoding " + QFileInfo {file}.fileName () + "\n");
      process_.setWorkingDirectory (temporary_.path ());
      process_.start (tool,{"4.6","1","384",QString::number (frequency->value ()),
                            QString::number (tolerance->value ()),"000000_000000.wav"});
      if (process_.waitForStarted (1000)) open->setEnabled (false);
    });
  }
  ~WavReceiveDialog () override
  {
    if (process_.state () != QProcess::NotRunning) { process_.kill (); process_.waitForFinished (1000); }
  }
private:
  QPlainTextEdit * output_ {};
  QProcess process_;
  QTemporaryDir temporary_;
};
}
void wsjtz::openJttyWavReceiver (QWidget * parent)
{
  auto dialog = new WavReceiveDialog {parent}; dialog->show ();
}
