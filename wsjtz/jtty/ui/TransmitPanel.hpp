// Fusion wiring around the original WSJT-X JTTY UI page. GPL-3.0-or-later.
#ifndef FUSION_JTTY_TRANSMIT_PANEL_HPP
#define FUSION_JTTY_TRANSMIT_PANEL_HPP
#include <QWidget>
#include <functional>
#include <memory>
class QSettings;
namespace Ui { class JttyTransmitPanel; }
namespace wsjtz {
class TransmitPanel : public QWidget {
public:
  TransmitPanel(QSettings*,QWidget* parent=nullptr);
  ~TransmitPanel() override;
  std::function<void(QString)> sendText,sendMacro;
  std::function<void(int)> rxChanged,txChanged,toleranceChanged;
  void setFrequencies(int rx,int tx);
  void setBusy(bool busy);
  void setProgress(qint64 served,qint64 total);
  void showPreparedText(QString const&);
  void clearEntry();
  void triggerMacro(int index);
  int serial() const;
  QString nextCall() const;
private:
  std::unique_ptr<Ui::JttyTransmitPanel> ui_;
  QSettings* settings_;
  int style_=-1;
  void loadStyle(int style);
  void saveTemplates();
};
}
#endif
