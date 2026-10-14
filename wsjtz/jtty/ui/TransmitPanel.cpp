#include "TransmitPanel.hpp"
#include "ui_JttyTransmitPanel.h"
#include "widgets/JttyMessages.hpp"
#include <QSettings>
#include <QShortcut>
#include <QSignalBlocker>
namespace wsjtz {
TransmitPanel::TransmitPanel(QSettings* settings,QWidget* parent):QWidget(parent),ui_(new Ui::JttyTransmitPanel),settings_(settings) {
  ui_->setupUi(this);
  ui_->Tx_Message->setToolTip(tr("Ctrl+K sends the current message once. Ctrl+Shift+K clears the field."));
  connect(ui_->pbSendMessage,&QPushButton::clicked,this,[this]{if(sendText)sendText(ui_->Tx_Message->toPlainText());});
  QLineEdit* fields[]={ui_->msg1,ui_->msg2,ui_->msg3,ui_->msg4,ui_->msg5,ui_->msg6,ui_->msg7,ui_->msg8};
  QPushButton* buttons[]={ui_->pbF1,ui_->pbF2,ui_->pbF3,ui_->pbF4,ui_->pbF5,ui_->pbF6,ui_->pbF7,ui_->pbF8};
  for(int i=0;i<8;++i){
    connect(buttons[i],&QPushButton::clicked,this,[this,i]{triggerMacro(i+1);});
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
  connect(ui_->pbR2T_2,&QPushButton::clicked,this,[this]{ui_->TxFreqSpinBox_2->setValue(ui_->RxFreqSpinBox_2->value());});
  connect(ui_->pbT2R_2,&QPushButton::clicked,this,[this]{ui_->RxFreqSpinBox_2->setValue(ui_->TxFreqSpinBox_2->value());});
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
