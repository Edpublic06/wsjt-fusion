// Fusion continuous receive audio monitor. GPL-3.0-or-later.
#ifndef FUSION_JTTY_AUDIO_DISPLAY_HPP
#define FUSION_JTTY_AUDIO_DISPLAY_HPP
#include <QWidget>
#include <QPainter>
#include <QImage>
#include <QTimer>
#include <QElapsedTimer>
#include <QtEndian>
#include <array>
#include <complex>
#include <cmath>
#include <cstring>
namespace wsjtz {
class AudioDisplay : public QWidget {
public:
  explicit AudioDisplay(QWidget *parent=nullptr): QWidget(parent), image_(512,120,QImage::Format_RGB32) {
    setMinimumHeight(185); image_.fill(Qt::black); clock_.start();
    auto timer=new QTimer(this);
    connect(timer,&QTimer::timeout,this,[this]{update();}); timer->start(250);
  }
  void reset() { used_=0; last_=-1; image_.fill(Qt::black); update(); }
  void append(QByteArray const& pcm) {
    double sum=0; int peak=0, count=pcm.size()/2;
    for(int i=0;i<count;++i) {
      int v=qFromLittleEndian<qint16>(reinterpret_cast<uchar const*>(pcm.constData()+2*i));
      sum+=double(v)*v; peak=qMax(peak,std::abs(v));
      samples_[used_++]=double(v)/32768.;
      if(used_==2048) { spectrum(); used_=0; }
    }
    if(count) { rms_=10*std::log10(qMax(sum/count/(32768.*32768.),1e-12)); peak_=peak; last_=clock_.elapsed(); update(); }
  }
protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this); p.fillRect(rect(),QColor(18,22,28)); p.setPen(Qt::white);
    bool live=last_>=0 && clock_.elapsed()-last_<2000;
    QString text=live ? QString("Audio received — RMS %1 dBFS%2").arg(rms_,0,'f',1).arg(peak_>=32760 ? " — CLIPPING" : "") : "No recent audio samples — check Monitor and audio input";
    if(live && peak_==0) text="Audio stream received — digital silence";
    p.drawText(8,18,text);
    int bar=qBound(0,int((rms_+90)/90*(width()-16)),width()-16);
    p.fillRect(8,25,width()-16,7,Qt::darkGray); if(live)p.fillRect(8,25,bar,7,peak_>=32760?Qt::red:Qt::green);
    p.drawImage(QRect(8,40,width()-16,height()-65),image_);
    p.setPen(Qt::white);
    for(int f=0;f<=3000;f+=500) p.drawText(qBound(8,8+(width()-16)*f/3000-15,width()-45),height()-7,QString::number(f));
    p.drawText(width()-25,38,"Hz");
  }
private:
  void spectrum() {
    std::array<std::complex<double>,2048> a;
    constexpr double pi=3.14159265358979323846;
    for(int i=0;i<2048;++i)a[i]=samples_[i]*(0.5-0.5*std::cos(2*pi*i/2047));
    for(int i=1,j=0;i<2048;++i){int bit=1024;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)std::swap(a[i],a[j]);}
    for(int n=2;n<=2048;n*=2){auto wlen=std::polar(1.,-2*pi/n);for(int i=0;i<2048;i+=n){std::complex<double>w=1.;for(int j=0;j<n/2;++j){auto u=a[i+j],v=a[i+j+n/2]*w;a[i+j]=u+v;a[i+j+n/2]=u-v;w*=wlen;}}}
    // 12 kHz PCM: bins 0..511 cover 0..3 kHz. Newest row at top.
    for(int y=image_.height()-1;y>0;--y)std::memcpy(image_.scanLine(y),image_.constScanLine(y-1),image_.bytesPerLine());
    auto row=reinterpret_cast<QRgb*>(image_.scanLine(0));
    for(int x=0;x<512;++x){double db=20*std::log10(qMax(std::abs(a[x])/512.,1e-12));double t=qBound(0.,(db+100)/85.,1.);row[x]=QColor::fromHsvF((1-t)*0.67,1.,qMin(1.,t*2)).rgb();}
  }
  QImage image_; QElapsedTimer clock_; qint64 last_=-1;
  std::array<double,2048> samples_{}; int used_=0,peak_=0; double rms_=-120;
};
}
#endif
