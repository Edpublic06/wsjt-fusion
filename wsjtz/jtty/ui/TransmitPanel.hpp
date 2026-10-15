// Fusion wiring around the original WSJT-X JTTY UI page. GPL-3.0-or-later.
#ifndef FUSION_JTTY_TRANSMIT_PANEL_HPP
#define FUSION_JTTY_TRANSMIT_PANEL_HPP
#include <QWidget>
#include <functional>
#include <memory>
class QSettings;
class QLabel;
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
  bool isFt8Style() const { return style_ == 1; }
  void setMacroContext(QString const& myCall,QString const& hisCall,QString const& grid,int snr);
  QString expandedMacro(QString const&) const;
  void refreshPreviews();
protected:
  void changeEvent(QEvent*) override;
private:
  void updatePreviewFonts();
public:
  int serial() const;
  QString nextCall() const;
private:
  std::unique_ptr<Ui::JttyTransmitPanel> ui_;
  QSettings* settings_;
  int style_=-1;
  QString myCall_,hisCall_,grid_;
  int snr_=-10;
  void loadStyle(int style);
  void saveTemplates();
};
}
#endif
