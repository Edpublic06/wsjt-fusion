#include "TextColors.hpp"
#include <QApplication>
#include <iostream>
int main(int argc,char** argv) {
 QApplication app(argc,argv);QTextEdit view;
 wsjtz::resetJttyText(view,"2509 Hz  TX  CQ F6FDY JN33\n1498 Hz  -9 dB  F4HVY DE PE1HUD\nTU 73",view.font());
 auto first=view.document()->begin();
 wsjtz::colorJttyLine(view.document(),first,Qt::yellow,Qt::black);
 for(int i=0;i<first.text().size();++i) {
  QTextCursor c(view.document());c.setPosition(i);c.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
  if(c.charFormat().background().color()!=Qt::yellow)return 1;
 }
 if(first.blockFormat().background().style()!=Qt::NoBrush)return 2;
 auto second=first.next();
 QTextCursor c(view.document());c.setPosition(second.position()+1);
 if(c.charFormat().background().style()!=Qt::NoBrush)return 3;
 wsjtz::colorJttyLine(view.document(),second,Qt::cyan,Qt::black);
 if(second.blockFormat().background().style()!=Qt::NoBrush)return 4;
 wsjtz::resetJttyText(view,"73 WITHOUT SENDER",view.font());
 c=QTextCursor(view.document());c.movePosition(QTextCursor::Right,QTextCursor::KeepAnchor);
 if(c.charFormat().background().style()!=Qt::NoBrush)return 5;
 std::cout<<"PASS text-only coloring, complete TX message and no inherited highlight\n";
}
