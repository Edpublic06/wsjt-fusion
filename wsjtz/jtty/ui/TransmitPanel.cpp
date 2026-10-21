#include "TransmitPanel.hpp"
#include "ui_JttyTransmitPanel.h"
#include "widgets/JttyMessages.hpp"
#include <QSettings>
#include <QShortcut>
#include <QSignalBlocker>
#include <QLabel>
#include <QGridLayout>
#include <QEvent>
#include <QFontMetrics>
namespace wsjtz {
TransmitPanel::TransmitPanel(QSettings* settings,QWidget* parent):QWidget(parent),ui_(new Ui::JttyTransmitPanel),settings_(settings) {
  ui_->setupUi(this);
  ui_->gridLayout_15->setRowMinimumHeight(3,22);
  for(int n=1;n<=8;++n){
    auto label=findChild<QLabel*>("macroPreview"+QString::number(n));
label->setAutoFillBackground(false);
    label->setStyleSheet("background: transparent; color: #007aff;");
    label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Minimum);
  }
  updatePreviewFonts();
  ui_->label_3->setMaximumWidth(QWIDGETSIZE_MAX);
  ui_->Tx_Message->setToolTip(tr("Ctrl+K sends the current message once. Ctrl+Shift+K clears the field."));
  auto updateSendColour=[this]{
    bool const hasText=!ui_->Tx_Message->toPlainText().trimmed().isEmpty();
    ui_->pbSendMessage->setStyleSheet(hasText
      ? QStringLiteral("QPushButton { background-color: #00cc00; color: black; }")
      : QStringLiteral("QPushButton { background-color: white; color: black; }"));
  };
  connect(ui_->Tx_Message,&QPlainTextEdit::textChanged,this,updateSendColour);
  updateSendColour();
  connect(ui_->pbSendMessage,&QPushButton::clicked,this,[this]{if(sendText)sendText(ui_->Tx_Message->toPlainText());});
  QLineEdit* fields[]={ui_->msg1,ui_->msg2,ui_->msg3,ui_->msg4,ui_->msg5,ui_->msg6,ui_->msg7,ui_->msg8};
  QPushButton* buttons[]={ui_->pbF1,ui_->pbF2,ui_->pbF3,ui_->pbF4,ui_->pbF5,ui_->pbF6,ui_->pbF7,ui_->pbF8};
  for(int i=0;i<8;++i){
    connect(buttons[i],&QPushButton::clicked,this,[this,i]{triggerMacro(i+1);});
    connect(fields[i],&QLineEdit::textChanged,this,[this]{refreshPreviews();});
    connect(fields[i],&QLineEdit::editingFinished,this,[this]{saveTemplates();});
  }
  auto send=new QShortcut(QKeySequence("Ctrl+K"),this);
  connect(send,&QShortcut::activated,this,[this]{if(ui_->pbSendMessage->isEnabled()&&sendText)sendText(ui_->Tx_Message->toPlainText());});
  auto clear=new QShortcut(QKeySequence("Ctrl+Shift+K"),this);
  connect(clear,&QShortcut::activated,this,[this]{clearEntry();});
  connect(ui_->comboBoxJttyStyle,qOverload<int>(&QComboBox::currentIndexChanged),this,[this](int n){loadStyle(n);});
  ui_->comboBoxJttyStyle->setCurrentIndex(settings_->value("Fusion/JTTY/MessageStyle",0).toInt());
  loadStyle(ui_->comboBoxJttyStyle->currentIndex());
  connect(ui_->RxFreqSpinBox_2,qOverload<int>(&QSpinBox::valueChanged),this,[this](int n){if(rxChanged)rxChanged(n);});
  connect(ui_->TxFreqSpinBox_2,qOverload<int>(&QSpinBox::valueChanged),this,[this](int n){if(txChanged)txChanged(n);});
  ui_->sbFtol_2->values({1,2,5,10,20,50,100,200,300,500,1000,1300,2000,3000,5000});
  ui_->sbFtol_2->setValue(1300);
  connect(ui_->sbFtol_2,qOverload<int>(&QSpinBox::valueChanged),this,[this](int n){if(toleranceChanged)toleranceChanged(n);});
  connect(ui_->lineEdit,&QLineEdit::textChanged,this,[this]{refreshPreviews();});
  connect(ui_->sbSerialNumber_2,qOverload<int>(&QSpinBox::valueChanged),this,[this]{refreshPreviews();});
  refreshPreviews();
  connect(ui_->pbR2T_2,&QPushButton::clicked,this,[this]{ui_->TxFreqSpinBox_2->setValue(ui_->RxFreqSpinBox_2->value());});
  connect(ui_->pbT2R_2,&QPushButton::clicked,this,[this]{ui_->RxFreqSpinBox_2->setValue(ui_->TxFreqSpinBox_2->value());});
}
void TransmitPanel::setReports(QString const& call,QString const& sent,QString const& received){
  if(reportCall_!=call.trimmed().toUpper())
    callMacroHighlighted_=!call.trimmed().isEmpty();
  else if(receivedReport_!=received)callMacroHighlighted_=false;
  reportCall_=call.trimmed().toUpper();sentReport_=sent;receivedReport_=received;
  ui_->qsoReports->setTextFormat(Qt::RichText);
  QString text;
  if(!reportCall_.isEmpty()) {
    auto const baseFont=font();
    bool const points=baseFont.pointSizeF()>0;
    double const callSize=(points?baseFont.pointSizeF():baseFont.pixelSize())*1.75;
    text=QStringLiteral("<span style=\"font-weight: bold; color: #007aff; font-size: %1%2;\">%3</span>")
      .arg(callSize).arg(points?QStringLiteral("pt"):QStringLiteral("px"))
      .arg(reportCall_.toHtmlEscaped());
    if(!sent.isEmpty())text+=QStringLiteral(
      "&nbsp;&nbsp;&nbsp;RST S : <span style=\"font-weight: bold; color: #008000;\">%1</span>")
      .arg(sent.toHtmlEscaped());
    if(!received.isEmpty())text+=QStringLiteral(
      "&nbsp;&nbsp;&nbsp;RST R : <span style=\"font-weight: bold; color: #800080;\">%1</span>")
      .arg(received.toHtmlEscaped());
  }
  ui_->qsoReports->setText(text);
  bool const suggest=isFt8Style() && !reportCall_.isEmpty();
  QString const green=QStringLiteral("QPushButton { background-color: #00cc00; color: black; }");
  bool const callActive=suggest && callMacroHighlighted_;
  ui_->pbF1->setStyleSheet(callActive?green:QString{});
  ui_->pbF2->setStyleSheet(suggest && !callActive && received.isEmpty()?green:QString{});
  ui_->pbF3->setStyleSheet(suggest && !callActive && !received.isEmpty()?green:QString{});
  ui_->pbF6->setStyleSheet(!suggest?green:QString{});
}
void TransmitPanel::highlightCallMacro(){
  callMacroHighlighted_=true;
  setReports(reportCall_,sentReport_,receivedReport_);
}
void TransmitPanel::updatePreviewFonts(){
  // Keep the entire report line larger than the configured application font.
  auto reports=font();
  if(reports.pointSizeF()>0)reports.setPointSizeF(reports.pointSizeF()*1.25);
  else if(reports.pixelSize()>0)reports.setPixelSize(qMax(1,qRound(reports.pixelSize()*1.25)));
  ui_->qsoReports->setFont(reports);
  ui_->qsoReports->setStyleSheet(QStringLiteral("font-size: %1%2;")
    .arg(reports.pointSizeF()>0?reports.pointSizeF():reports.pixelSize())
    .arg(reports.pointSizeF()>0?QStringLiteral("pt"):QStringLiteral("px")));
  auto callsignFont=font();
  if(callsignFont.pointSizeF()>0)callsignFont.setPointSizeF(callsignFont.pointSizeF()*1.75);
  else if(callsignFont.pixelSize()>0)callsignFont.setPixelSize(qMax(1,qRound(callsignFont.pixelSize()*1.75)));
  ui_->qsoReports->setMinimumHeight(QFontMetrics(callsignFont).lineSpacing()+6);
  setReports(reportCall_,sentReport_,receivedReport_);
  auto small=font();
  if(small.pointSizeF()>0)small.setPointSizeF(qMax(1.0,small.pointSizeF()*0.80));
  else if(small.pixelSize()>0)small.setPixelSize(qMax(1,qRound(small.pixelSize()*0.80)));
  for(int n=1;n<=8;++n){
    auto label=findChild<QLabel*>("macroPreview"+QString::number(n));
    if(label){label->setFont(small);label->setMinimumHeight(QFontMetrics(small).lineSpacing()*2);}
  }
}
void TransmitPanel::changeEvent(QEvent* event){
  QWidget::changeEvent(event);
  if(event->type()==QEvent::FontChange)updatePreviewFonts();
}
TransmitPanel::~TransmitPanel()=default;
void TransmitPanel::saveTemplates(){
  if(style_<0)return;
  QLineEdit* fields[]={ui_->msg1,ui_->msg2,ui_->msg3,ui_->msg4,ui_->msg5,ui_->msg6,ui_->msg7,ui_->msg8};
  for(int i=0;i<8;++i)settings_->setValue("Fusion/JTTY/"+Jtty::messageStyleSettingsKey(static_cast<Jtty::MessageStyle>(style_),i+1),fields[i]->text());
}
void TransmitPanel::loadStyle(int style){
  if(style==style_)return;
  saveTemplates();style_=style;
  QLineEdit* fields[]={ui_->msg1,ui_->msg2,ui_->msg3,ui_->msg4,ui_->msg5,ui_->msg6,ui_->msg7,ui_->msg8};
  for(int i=0;i<8;++i)fields[i]->setText(settings_->value("Fusion/JTTY/"+Jtty::messageStyleSettingsKey(static_cast<Jtty::MessageStyle>(style),i+1),Jtty::messageStyleDefaultTemplate(static_cast<Jtty::MessageStyle>(style),i+1)).toString());
  settings_->setValue("Fusion/JTTY/MessageStyle",style);
  refreshPreviews();
  setReports(reportCall_,sentReport_,receivedReport_);
}
void TransmitPanel::setMacroContext(QString const& myCall,QString const& hisCall,QString const& grid,int snr){
  if(myCall_==myCall && hisCall_==hisCall && grid_==grid && snr_==snr)return;
  myCall_=myCall;hisCall_=hisCall;grid_=grid;snr_=snr;refreshPreviews();
}
QString TransmitPanel::expandedMacro(QString const& macro) const {
  Jtty::NativeMacroContext context;
  context.myCall=myCall_;context.hisCall=hisCall_;context.grid=grid_;
  context.snr=snr_;context.serialNumber=serial();
  auto expanded=macro;expanded.replace("%Q",nextCall());
  return Jtty::prepareTransmitText(Jtty::expandLiteralMacro(expanded,context)).text;
}
void TransmitPanel::refreshPreviews(){
  QLineEdit* fields[]={ui_->msg1,ui_->msg2,ui_->msg3,ui_->msg4,ui_->msg5,ui_->msg6,ui_->msg7,ui_->msg8};
  for(int i=0;i<8;++i){
    auto label=findChild<QLabel*>("macroPreview"+QString::number(i+1));
    auto const macro=fields[i]->text();QString text;
    if(macro.trimmed().isEmpty())text.clear();
    else if(macro.contains("%H") && hisCall_.isEmpty())text.clear();
    else if(macro.contains("%Q") && nextCall().isEmpty())text=tr("Enter Call next");
    else if(macro.contains("%M") && myCall_.isEmpty())text=tr("Set your callsign");
    else text=expandedMacro(macro);
    label->setText(text);label->setToolTip(text);
  }
}
void TransmitPanel::triggerMacro(int n){
  QLineEdit* fields[]={ui_->msg1,ui_->msg2,ui_->msg3,ui_->msg4,ui_->msg5,ui_->msg6,ui_->msg7,ui_->msg8};
  if(n>=1&&n<=8&&ui_->pbSendMessage->isEnabled()&&sendMacro)sendMacro(fields[n-1]->text());
}
void TransmitPanel::setFrequencies(int rx,int tx){QSignalBlocker a(ui_->RxFreqSpinBox_2),b(ui_->TxFreqSpinBox_2);ui_->RxFreqSpinBox_2->setValue(rx);ui_->TxFreqSpinBox_2->setValue(tx);}
void TransmitPanel::setBusy(bool busy){
  ui_->pbSendMessage->setEnabled(!busy);ui_->widget2->setEnabled(!busy);ui_->widget->setEnabled(!busy);ui_->Tx_Message->setReadOnly(busy);
}
void TransmitPanel::setProgress(qint64 served,qint64 total){ui_->labelJttyFrameCount->setText(total>0?QString("%1 / %2").arg(served/(59*1536)).arg((total+59*1536-1)/(59*1536)):QString{});}
void TransmitPanel::showPreparedText(QString const& text){ui_->Tx_Message->setPlainText(text);}
void TransmitPanel::clearEntry(){ui_->Tx_Message->clear();}
int TransmitPanel::serial() const{return ui_->sbSerialNumber_2->value();}
QString TransmitPanel::nextCall() const{return ui_->lineEdit->text().trimmed().toUpper();}
}
