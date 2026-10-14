// Fusion live receive adapter, 2026-10-14. GPL-3.0-or-later.
#ifndef FUSION_JTTY_LIVE_RECEIVER_HPP
#define FUSION_JTTY_LIVE_RECEIVER_HPP
#include <QObject>
#include <QProcess>
#include <QMap>
#include <functional>
namespace wsjtz {
class LiveReceiver : public QObject {
public:
  explicit LiveReceiver (QObject * parent);
  ~LiveReceiver () override;
  void start (int center,int tolerance);
  void stop ();
  void append (QByteArray const& pcm);
  std::function<void(QString const&)> textChanged, statusChanged;
private:
  bool stopping_ = false;
  QProcess process_;
  QByteArray pending_;
  QMap<qint64,QString> lines_;
};
}
#endif
