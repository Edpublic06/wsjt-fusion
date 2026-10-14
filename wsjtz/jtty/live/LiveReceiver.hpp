// Fusion live receive adapter, 2026-10-14. GPL-3.0-or-later.
#ifndef FUSION_JTTY_LIVE_RECEIVER_HPP
#define FUSION_JTTY_LIVE_RECEIVER_HPP
#include <QObject>
#include <QProcess>
#include <QMap>
#include <functional>
#include "wsjtz/jtty/selection/SnrHistory.hpp"
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
  std::function<void(QString const&)> textChanged, statusChanged;
private:
  JttySnrHistory snrHistory_;
  bool stopping_ = false;
  QProcess process_;
  QByteArray pending_;
  quint64 session_ = 0;
  QMap<QPair<quint64,qint64>,QString> lines_;
};
}
#endif
