#include "Sender.hpp"
#include <QSet>
#include <iostream>
int main() {
 QSet<QString> calls {"F4HVY","PE1HUD","R8LCA","F6FDY","K1JT"};
 auto identify=[&](QString s){return wsjtz::jttySender(s,[&](QString c){return calls.contains(c);});};
 if(identify("F4HVY DE PE1HUD PE1HUD K")!="PE1HUD")return 1;
 if(identify("CQ R8LCA CQ")!="R8LCA")return 2;
 if(identify("CQ DX K1JT FN20")!="K1JT")return 3;
 if(identify("F6FDY K1JT -07")!="K1JT")return 4;
 if(!identify("TU THANKS 73").isEmpty())return 5;
 if(!identify("F4HVY DE PE ... OP THEO").isEmpty())return 6;
 if(!identify("THANKS F4HVY 73").isEmpty())return 7;
 std::cout<<"PASS sender identification without guessing from partial or destination calls\n";
}
