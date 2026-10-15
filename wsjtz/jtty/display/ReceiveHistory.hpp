// Fusion receive-pane routing. GPL-3.0-or-later.
#ifndef FUSION_JTTY_RECEIVE_HISTORY_HPP
#define FUSION_JTTY_RECEIVE_HISTORY_HPP
#include <QMap>
#include <QStringList>
namespace wsjtz {
class JttyReceiveHistory {
public:
  void put(quint64 session,qint64 id,QString const& text,double frequency,bool transmit=false) {
    lines_[qMakePair(session,id)]={text,frequency,transmit};
    while(lines_.size()>200)lines_.erase(lines_.begin());
  }
  QString all() const {
    QStringList result;
    for(auto const& line:lines_)result.append(line.text);
    return result.join('\n');
  }
  QString within(double lower,double upper) const {
    QStringList result;
    for(auto const& line:lines_)
      if(line.transmit || (line.frequency>=lower && line.frequency<=upper))result.append(line.text);
    return result.join('\n');
  }
private:
  struct Line {QString text;double frequency;bool transmit;};
  QMap<QPair<quint64,qint64>,Line> lines_;
};
}
#endif
