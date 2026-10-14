// Extracted from WSJT-X v3.3.0-beta1 JttyReceiveResultController.cpp.
// See live/source-manifest.json. GPL-3.0-or-later.
#ifndef FUSION_JTTY_SELECTION_SNR_HISTORY_HPP
#define FUSION_JTTY_SELECTION_SNR_HISTORY_HPP
#include <QString>
#include <QStringList>
#include <deque>
namespace wsjtz {
class JttySnrHistory {
public:
  enum class Pane { All, Qso };
  void observe(QString const& text,int snr,bool admitted) {
    if (!text.isEmpty()) {
      snrHistory_.push_back({text, snr, admitted});
      if (snrHistory_.size() > 500) snrHistory_.pop_front();
    }
  }
  int snrForSelectedWord(QString const& word, Pane pane) const {
    for (auto it = snrHistory_.crbegin(); it != snrHistory_.crend(); ++it) {
      if (pane == Pane::Qso && !it->admitted) continue;
      if (it->text.split(QChar{' '}, Qt::SkipEmptyParts).contains(word)) return it->snr;
    }
    return -10;
  }
private:
  struct SnrEntry { QString text; int snr; bool admitted; };
  std::deque<SnrEntry> snrHistory_;
};
}
#endif
