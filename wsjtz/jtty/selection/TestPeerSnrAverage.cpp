#include "PeerSnrAverage.hpp"
#include <iostream>
int main() {
 wsjtz::PeerSnrAverage a;
 a.observe(1,1,"W1PID",4);a.select("W1PID");a.seed(qMakePair(quint64(1),qint64(1)));
 a.observe(1,1,"W1PID",6); // one growing message
 if(a.average().count!=1)return 1;
 a.observe(1,2,"OTHER",30);a.observe(1,3,"W1PID",0);a.observe(1,4,"W1PID",3);
 auto result=a.average();if(result.count!=3 || result.snr!=3)return 2;
 a.observe(1,5,"W1PID",30);result=a.average(); // fourth must never replace first
 if(result.count!=3 || result.snr!=3)return 3;
 a.observe(1,1,"",9);if(a.average().count!=3 || a.average().snr!=4)return 4;
 a.select("OTHER");if(a.average().count)return 5;
 a.observe(2,1,"OTHER",-4);if(a.average().snr!=-4)return 6;
 a.select("W1PID");if(a.average().count)return 7;
 a.observe(2,2,"W1PID",5);a.observe(2,2,"",7);
 if(a.average().count!=1 || a.average().snr!=7)return 8;
 std::cout<<"PASS first three peer messages, progressive deduplication, fourth excluded and reset\n";
}
