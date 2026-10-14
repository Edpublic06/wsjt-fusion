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
 // Exercise the embedded controls at a large font, not just the standalone form.
 QWidget window; auto layout=new QVBoxLayout(&window);
 layout->addWidget(new QTextEdit(&window),1);
 auto stack=new CurrentPageHeightStackedWidget(&window);
 auto normal=new QWidget(stack);auto normalLayout=new QVBoxLayout(normal);normalLayout->addWidget(new QLabel("Timed-mode controls",normal));normal->setMinimumHeight(100);stack->addWidget(normal);
 auto embedded=new wsjtz::TransmitPanel(&settings,stack);
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
