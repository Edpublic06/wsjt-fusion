// First three distinct JTTY messages of the selected peer. GPL-3.0-or-later.
#ifndef FUSION_JTTY_PEER_SNR_AVERAGE_HPP
#define FUSION_JTTY_PEER_SNR_AVERAGE_HPP
#include <QMap>
#include <QString>
#include <QtMath>
namespace wsjtz {
class PeerSnrAverage {
public:
 using Key=QPair<quint64,qint64>;
 struct Result { int snr=-10;int count=0; };
 void observe(quint64 session,qint64 id,QString const& sender,int snr) {
  auto key=qMakePair(session,id);
  auto& sample=recent_[key];
  if(!sender.isEmpty())sample.sender=sender.toUpper();
  sample.snr=snr;
  if(selected_.contains(key))selected_[key]=snr;
  else if(!peer_.isEmpty() && sample.sender==peer_ && selected_.size()<3)selected_[key]=snr;
  while(recent_.size()>500)recent_.erase(recent_.begin());
 }
 void select(QString const& peer) {
  if(peer.toUpper()!=peer_){peer_=peer.toUpper();selected_.clear();}
 }
 void seed(Key key) {
  if(!peer_.isEmpty() && selected_.isEmpty() && recent_.contains(key))selected_[key]=recent_[key].snr;
 }
 Result average() const {
  Result result;int total=0;
  for(auto snr:selected_){total+=snr;++result.count;}
  if(result.count)result.snr=qRound(double(total)/result.count);
  return result;
 }
private:
 struct Sample {QString sender;int snr=0;};
 QMap<Key,Sample> recent_;
 QMap<Key,int> selected_;
 QString peer_;
};
}
#endif
