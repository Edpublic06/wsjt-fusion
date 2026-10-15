#include "SentReport.hpp"
#include <iostream>
int main() {
 auto rpt=[](QString text){return wsjtz::jttySentReport(text,"W1PID","F6FDY");};
 if(rpt("W1PID F6FDY +04")!="+04")return 1;
 if(rpt("W1PID F6FDY R-03")!="-03")return 2;
 if(rpt("W1PID 599 002")!="599")return 3;
 if(!rpt("CQ F6FDY JN33").isEmpty())return 4;
 if(!rpt("W1PID F6FDY JN33").isEmpty())return 5;
 if(!rpt("K1JT F6FDY -03").isEmpty())return 6;
 if(!rpt("W1PID TU CQ F6FDY CQ").isEmpty())return 7;
 if(!wsjtz::jttySentReport("W1PID F6FDY -03","","F6FDY").isEmpty())return 8;
 std::cout<<"PASS manual JTTY log report from actual TX text, no CQ or other-peer report\n";
}
