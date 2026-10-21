// Fusion FIFO indicator, independent of Fox queue state. GPL-3.0-or-later.
#ifndef FUSION_QUEUE_TAB_STYLE_HPP
#define FUSION_QUEUE_TAB_STYLE_HPP
#include <QProxyStyle>
#include <QStyleOptionTab>
#include <QTabBar>
#include <QPainter>

class FusionQueueTabStyle final : public QProxyStyle {
public:
  void drawControl(ControlElement element,QStyleOption const* option,
                   QPainter* painter,QWidget const* widget=nullptr) const override {
    QProxyStyle::drawControl(element,option,painter,widget);
    auto const bar=qobject_cast<QTabBar const*>(widget);
    auto const tab=qstyleoption_cast<QStyleOptionTab const*>(option);
    if(element==CE_TabBarTabShape && bar && tab && bar->count()>1
        && bar->property("fusionFifoWaiting").toBool() && tab->rect==bar->tabRect(1)) {
      painter->save();
      painter->setRenderHint(QPainter::Antialiasing);
      painter->setPen(Qt::NoPen);
      painter->setBrush(QColor(0,204,0));
      painter->drawRoundedRect(tab->rect.adjusted(1,1,-1,-1),4,4);
      painter->restore();
    }
  }
};
#endif
