#include "ReceiveHistory.hpp"
#include <iostream>
int main() {
  wsjtz::JttyReceiveHistory h;
  h.put(1,1,"outside",985);
  h.put(1,2,"inside",1500);
  h.put(1,3,"edge",1499);
  h.put(1,4,"TX",1000,true);
  if(h.all()!="outside\ninside\nedge\nTX")return 1;
  if(h.within(1499,1592.75)!="inside\nedge\nTX")return 2;
  h.put(1,2,"growing",1502);
  if(h.within(1499,1592.75)!="growing\nedge\nTX")return 3;
  if(h.within(980,990)!="outside\nTX")return 4;
  for(int i=0;i<200;++i)h.put(2,i,"new",1500);
  if(h.all().contains("outside") || h.all().contains("TX"))return 5;
  std::cout<<"PASS separate histories, exact bounds, retuning, progressive updates, TX and capacity\n";
}
