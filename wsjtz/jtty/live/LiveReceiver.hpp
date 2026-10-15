// Fusion live receive adapter, 2026-10-14. GPL-3.0-or-later.
#ifndef FUSION_JTTY_LIVE_RECEIVER_HPP
#define FUSION_JTTY_LIVE_RECEIVER_HPP
#include <QObject>
#include <QProcess>
#include <QMap>
#include <functional>
#include "wsjtz/jtty/selection/SnrHistory.hpp"
#include "wsjtz/jtty/display/ReceiveHistory.hpp"
namespace wsjtz {
class LiveReceiver : public QObject {
public:
  explicit LiveReceiver (QObject * parent);
  ~LiveReceiver () override;
  void start (int center,int tolerance);
  void stop ();
  void append (QByteArray const& pcm);
  int snrForSelectedWord (QString const& word) const { return snrHistory_.snrForSelectedWord (word,JttySnrHistory::Pane::All); }
  void appendTransmit (QString const& text,int frequency);
  std::function<void(QString const&,QString const&)> textChanged;
  std::function<void(QString const&)> statusChanged;
  void setSelection(int center,int tolerance);
private:
  JttySnrHistory snrHistory_;
  bool stopping_ = false;
  QProcess process_;
  QByteArray pending_;
  quint64 session_ = 0;
  JttyReceiveHistory lines_;
  int center_=1500,tolerance_=1300;
  void publish();
};
}
#endif
