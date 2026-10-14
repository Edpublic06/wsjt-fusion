// Offline UI wiring regression. No audio device, radio or personal settings.
#include "TransmitPanel.hpp"
#include <QApplication>
#include <QSettings>
#include <QTemporaryDir>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QLabel>
#include <iostream>
int main(int argc,char**argv) {
 QApplication app(argc,argv);QTemporaryDir dir;
 QSettings settings(dir.filePath("test.ini"),QSettings::IniFormat);
 settings.setValue("WSJTXExisting",42);
 wsjtz::TransmitPanel panel(&settings);panel.resize(850,280);panel.show();app.processEvents();
 QString sent,macro;panel.sendText=[&](QString s){sent=s;};panel.sendMacro=[&](QString s){macro=s;};
 auto entry=panel.findChild<QPlainTextEdit*>("Tx_Message");auto send=panel.findChild<QPushButton*>("pbSendMessage");
 if(!entry||!send)return 2;
 entry->setPlainText("CQ F6FDY CQ");send->click();if(sent!="CQ F6FDY CQ")return 3;
 panel.triggerMacro(1);if(macro!="CQ %M CQ")return 4;
 auto style=panel.findChild<QComboBox*>("comboBoxJttyStyle");auto f1=panel.findChild<QLineEdit*>("msg1");
 f1->setText("TEST CQ %M");style->setCurrentIndex(1);if(f1->text()=="TEST CQ %M")return 5;
 style->setCurrentIndex(0);if(f1->text()!="TEST CQ %M")return 6;
 int changes=0;panel.rxChanged=[&](int){++changes;};panel.setFrequencies(1200,1500);if(changes)return 7;
 panel.findChild<QSpinBox*>("RxFreqSpinBox_2")->setValue(1300);if(changes!=1)return 8;
 sent.clear();macro.clear();panel.setBusy(true);send->click();panel.triggerMacro(1);if(!sent.isEmpty()||!macro.isEmpty())return 9;
 panel.setBusy(false);panel.setProgress(59*1536,2*59*1536);if(panel.findChild<QLabel*>("labelJttyFrameCount")->text()!="1 / 2")return 10;
 if(settings.value("WSJTXExisting").toInt()!=42)return 11;
 panel.grab().save("/private/tmp/fusion-jtty-original-panel.png");
 std::cout<<"PASS original JTTY page wiring, style persistence, frequency synchronization and TX lock\n";
}
