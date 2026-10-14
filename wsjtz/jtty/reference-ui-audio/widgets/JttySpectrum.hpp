#ifndef JTTY_SPECTRUM_HPP
#define JTTY_SPECTRUM_HPP

#include <QtGlobal>
#include <array>
#include <functional>

struct JttyWaterfallRow
{
  quint64 reception = 0;
  qint64 beginSample = 0;
  qint64 endSample = 0;
  qint64 utcEndMs = 0;
  bool gap = false;

  bool hasAudio () const { return reception != 0 && !gap && endSample > beginSample; }
};

struct JttySpectrumFrame
{
  static constexpr int BinCount = 6827;
  static constexpr float BinWidth = 12000.0f / 16384;
  JttyWaterfallRow row;
  std::array<float, BinCount> bins {};
  std::array<float, BinCount> cumulative {};
  std::array<float, BinCount> linearAverage {};
  float powerDb = 0;
  float maxPowerDb = 0;
};

// Consumes new PCM once; FFT scratch is used only on the serialized DSP thread.
class JttySpectrum
{
public:
  static constexpr int FftSize = 16384;
  static constexpr int Hop = 3456;

  JttySpectrum ();
  JttySpectrum (JttySpectrum const&) = delete;
  JttySpectrum& operator= (JttySpectrum const&) = delete;

  void begin (quint64 reception, qint64 firstSample, qint64 utcFirstSampleMs);
  bool process (qint64 firstSample, qint16 const * pcm, int count,
                int inputGain, bool lowSidelobes, int smoothing,
                std::function<void (JttySpectrumFrame const&)> const& emitFrame);

private:
  void transform (int inputGain, bool lowSidelobes, int smoothing);

  quint64 reception_ = 0;
  qint64 origin_ = 0;
  qint64 nextSample_ = 0;
  qint64 utcOriginMs_ = 0;
  quint64 frameCount_ = 0;
  int pending_ = 0;
  int write_ = 0;
  double power_ = 0;
  float peak_ = 0;
  std::array<qint16, FftSize> pcm_ {};
  std::array<float, FftSize> window_ {};
  std::array<float, FftSize + 2> fft_ {};
  std::array<double, JttySpectrumFrame::BinCount> average_ {};
  JttySpectrumFrame frame_;
};

#endif
