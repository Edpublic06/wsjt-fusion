// Offline UI wiring regression. No audio device, radio or personal settings.
#include "TransmitPanel.hpp"
#include "widgets/CurrentPageHeightStackedWidget.hpp"
#include <QVBoxLayout>
#include <QTextEdit>
#include <QSplitter>
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
 if(!send->styleSheet().contains("white"))return 42;
 entry->setPlainText("test");if(!send->styleSheet().contains("#00cc00"))return 43;
 panel.clearEntry();if(!send->styleSheet().contains("white"))return 44;
 entry->setPlainText("CQ F6FDY CQ");send->click();if(sent!="CQ F6FDY CQ")return 3;
 panel.triggerMacro(1);if(macro!="CQ %M CQ")return 4;
 auto style=panel.findChild<QComboBox*>("comboBoxJttyStyle");auto f1=panel.findChild<QLineEdit*>("msg1");
 if(panel.isFt8Style())return 19;
 f1->setText("TEST CQ %M");style->setCurrentIndex(1);if(f1->text()=="TEST CQ %M")return 5;
 if(!panel.isFt8Style())return 20;
 macro.clear();panel.triggerMacro(2);if(macro!="%H %M %R")return 21;
 style->setCurrentIndex(0);if(f1->text()!="TEST CQ %M")return 6;
 int changes=0;panel.rxChanged=[&](int){++changes;};panel.setFrequencies(1200,1500);if(changes)return 7;
 panel.findChild<QSpinBox*>("RxFreqSpinBox_2")->setValue(1300);if(changes!=1)return 8;
 sent.clear();macro.clear();panel.setBusy(true);send->click();panel.triggerMacro(1);if(!sent.isEmpty()||!macro.isEmpty())return 9;
 panel.setBusy(false);panel.setProgress(59*1536,2*59*1536);if(panel.findChild<QLabel*>("labelJttyFrameCount")->text()!="1 / 2")return 10;
 if(settings.value("WSJTXExisting").toInt()!=42)return 11;
 // The preview must share the actual TX expansion, including live context edits.
 style->setCurrentIndex(1);panel.setMacroContext("F6FDY","W1PID","JN33",4);
 auto preview1=panel.findChild<QLabel*>("macroPreview1");
 auto preview2=panel.findChild<QLabel*>("macroPreview2");
 if(!preview1 || preview1->text()!="W1PID F6FDY JN33" || preview2->text()!="W1PID F6FDY +04")return 22;
 panel.setMacroContext("F6FDY","PE1HUD","JN33",-7);
 if(preview1->text()!="PE1HUD F6FDY JN33" || preview2->text()!="PE1HUD F6FDY -07")return 23;
 if(panel.expandedMacro(panel.findChild<QLineEdit*>("msg2")->text())!=preview2->text())return 24;
 panel.findChild<QLineEdit*>("msg2")->setText("%H %M R%R");
 if(preview2->text()!="PE1HUD F6FDY R-07")return 25;
 f1->setText("%H %M %G");
 panel.setMacroContext("F6FDY","","JN33",-10);
 if(!preview1->text().isEmpty() || !preview1->toolTip().isEmpty())return 26;
 panel.setMacroContext("F6FDY","W1PID","JN33",4);
 style->setCurrentIndex(0);panel.findChild<QLineEdit*>("msg6")->setText("TU NOW %Q %E");
 panel.findChild<QLineEdit*>("lineEdit")->setText("K1JT");
 panel.findChild<QSpinBox*>("sbSerialNumber_2")->setValue(12);
 auto preview6=panel.findChild<QLabel*>("macroPreview6");
 if(preview6->text()!=panel.expandedMacro("TU NOW %Q %E") || !preview6->text().contains("K1JT"))return 27;
 if(preview1->autoFillBackground() || preview1->frameShape()!=QFrame::NoFrame)return 28;
 app.processEvents();
 auto serialWidget=panel.findChild<QSpinBox*>("sbSerialNumber_2");
 auto nextWidget=panel.findChild<QLineEdit*>("lineEdit");
 if(qAbs(serialWidget->mapTo(&panel,QPoint{}).y()-nextWidget->mapTo(&panel,QPoint{}).y())>8)return 29;
 auto f5=panel.findChild<QPushButton*>("pbF5");
 if(f5->mapTo(&panel,QPoint{}).y()-(preview1->mapTo(&panel,QPoint{}).y()+preview1->height())<18)return 30;
 // Peer summary and next-report guidance never transmit by themselves.
 auto summary=panel.findChild<QLabel*>("qsoReports");
 auto f2Button=panel.findChild<QPushButton*>("pbF2");
 auto f3Button=panel.findChild<QPushButton*>("pbF3");
 style->setCurrentIndex(1);panel.setReports("AE2NY","","");
 if(!summary->text().contains("AE2NY") || summary->text().contains("RST"))return 31;
 auto f1Button=panel.findChild<QPushButton*>("pbF1");
 if(f1Button->styleSheet().isEmpty() || !f2Button->styleSheet().isEmpty() || !f3Button->styleSheet().isEmpty())return 32;
 panel.setReports("AE2NY","+10","");
 if(f1Button->styleSheet().isEmpty() || !f2Button->styleSheet().isEmpty())return 40;
 panel.highlightCallMacro();
 panel.setReports("AE2NY","+10","");
 if(f1Button->styleSheet().isEmpty())return 41;
 if(!panel.findChild<QPushButton*>("pbF6")->styleSheet().isEmpty())return 39;
 panel.setReports("AE2NY","+10","-05");
 if(!summary->text().contains("-05") || !summary->text().contains("+10"))return 33;
 if(!f2Button->styleSheet().isEmpty() || f3Button->styleSheet().isEmpty())return 34;
 panel.setReports("K2GAV","","");
 if(summary->text().contains("AE2NY") || summary->text().contains("RST"))return 35;
 panel.setReports("","","");
 if(panel.findChild<QPushButton*>("pbF6")->styleSheet().isEmpty())return 38;
 if(!summary->text().isEmpty() || !f2Button->styleSheet().isEmpty() || !f3Button->styleSheet().isEmpty())return 36;
 panel.setReports("AE2NY","+10","-05");style->setCurrentIndex(0);
 if(!f2Button->styleSheet().isEmpty() || !f3Button->styleSheet().isEmpty())return 37;
 // Exercise the embedded controls at a large font, not just the standalone form.
 QWidget window; auto layout=new QVBoxLayout(&window);
 layout->addWidget(new QTextEdit(&window),1);
 auto stack=new CurrentPageHeightStackedWidget(&window);
 auto normal=new QWidget(stack);auto normalLayout=new QVBoxLayout(normal);normalLayout->addWidget(new QLabel("Timed-mode controls",normal));normal->setMinimumHeight(100);stack->addWidget(normal);
 auto embedded=new wsjtz::TransmitPanel(&settings,stack);
 embedded->setMacroContext("F6FDY","PE1HUD","JN33",4);
 auto font=embedded->font();font.setPointSize(20);embedded->setFont(font);
 stack->addWidget(embedded);stack->setMaximumHeight(QWIDGETSIZE_MAX);
 stack->setCurrentWidget(embedded);layout->addWidget(stack);
 window.resize(1400,950);window.show();app.processEvents();
 if(stack->height()<embedded->minimumSizeHint().height())return 12;
 for(auto control:embedded->findChildren<QPushButton*>())
   if(control->height()<control->minimumSizeHint().height())return 13;
 for(auto control:embedded->findChildren<QLineEdit*>())
   if(control->objectName()!="qt_spinbox_lineedit" && control->height()<control->minimumSizeHint().height()){std::cerr<<control->objectName().toStdString()<<" actual="<<control->height()<<" min="<<control->minimumSizeHint().height()<<"\n";return 14;}
 window.grab().save("/private/tmp/fusion-jtty-embedded-panel.png");
 stack->setCurrentWidget(normal);stack->removeWidget(embedded);embedded->setParent(&window);embedded->hide();
 stack->setMaximumHeight(220);app.processEvents();
 if(stack->minimumSizeHint().height()>220)return 15;
 // A hidden wide JTTY page must not change the ordinary stack's width hint.
 int normalWidth=stack->minimumSizeHint().width();
 stack->addWidget(embedded);stack->setCurrentWidget(embedded);app.processEvents();
 stack->setCurrentWidget(normal);stack->removeWidget(embedded);embedded->setParent(&window);embedded->hide();app.processEvents();
 if(stack->minimumSizeHint().width()!=normalWidth)return 16;
 QSplitter split;split.addWidget(new QTextEdit(&split));split.addWidget(stack);
 split.resize(1600,700);split.show();split.setSizes({800,800});app.processEvents();
 auto before=split.sizes();auto state=split.saveState();
 stack->setMaximumHeight(QWIDGETSIZE_MAX);stack->addWidget(embedded);stack->setCurrentWidget(embedded);app.processEvents();
 stack->setCurrentWidget(normal);stack->removeWidget(embedded);embedded->hide();embedded->setParent(&window);
 stack->setMaximumHeight(220);app.processEvents();
 if(!split.restoreState(state))return 17;
 app.processEvents();auto after=split.sizes();
 int beforeTotal=before[0]+before[1],afterTotal=after[0]+after[1];
 if(qAbs(after[0]*beforeTotal-before[0]*afterTotal)>beforeTotal)return 18;
 panel.grab().save("/private/tmp/fusion-jtty-original-panel.png");
 std::cout<<"PASS original JTTY page wiring, style persistence, frequency synchronization and TX lock\n";
}
