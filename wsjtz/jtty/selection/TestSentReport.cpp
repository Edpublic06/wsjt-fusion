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
 auto received=[](QString text){return wsjtz::jttyReceivedReport(text,"K2GAV","F6FDY");};
 if(received("F6FDY K2GAV +03 K")!="+03")return 9;
 if(received("F6FDY K2GAV R+03 KN")!="+03")return 10;
 if(!received("F6FDY W1PID +03 K").isEmpty())return 11;
 if(!received("W1PID K2GAV +03 K").isEmpty())return 12;
 if(!received("CQ K2GAV +03").isEmpty())return 13;
 if(!received("F6FDY K2GAV JN33").isEmpty())return 14;
 if(received("F6FDY DE K2GAV R-07 K")!="-07")return 15;
 std::cout<<"PASS manual JTTY log report from actual TX text, no CQ or other-peer report\n";
}
