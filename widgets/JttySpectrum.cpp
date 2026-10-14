#include "JttySpectrum.hpp"

#include <algorithm>
#include <cmath>

constexpr int JttySpectrumFrame::BinCount;
constexpr float JttySpectrumFrame::BinWidth;
constexpr int JttySpectrum::FftSize;
constexpr int JttySpectrum::Hop;

extern "C" {
  void nuttal_window_ (float *, int *);
  void four2a_ (float *, int *, int *, int *, int *);
  void flat1_ (float *, int *, int *, float *);
  void smo_ (float *, int *, float *, int *);
}

JttySpectrum::JttySpectrum ()
{
  int size = FftSize;
  nuttal_window_ (window_.data (), &size);
}

void JttySpectrum::begin (quint64 reception, qint64 firstSample, qint64 utcFirstSampleMs)
{
  reception_ = reception;
  origin_ = nextSample_ = firstSample;
  utcOriginMs_ = utcFirstSampleMs;
  frameCount_ = 0;
  pending_ = write_ = 0;
  power_ = 0;
  peak_ = 0;
  pcm_.fill (0);
  average_.fill (0);
  frame_ = JttySpectrumFrame {};
}

bool JttySpectrum::process (qint64 firstSample, qint16 const * pcm, int count,
                           int inputGain, bool lowSidelobes, int smoothing,
                           std::function<void (JttySpectrumFrame const&)> const& emitFrame)
{
  if (!reception_ || firstSample != nextSample_ || count < 0 || (!pcm && count)) return false;
  for (int i = 0; i < count; ++i) {
    auto const value = pcm[i];
    pcm_[write_] = value;
    write_ = (write_ + 1) % FftSize;
    if (nextSample_ - origin_ >= 10) {
      power_ += double (value) * value;
      peak_ = std::max (peak_, std::abs (float (value)));
    }
    ++nextSample_;
    if (++pending_ == Hop) {
      transform (inputGain, lowSidelobes, smoothing);
      pending_ = 0;
      power_ = 0;
      peak_ = 0;
      emitFrame (frame_);
    }
  }
  return true;
}

void JttySpectrum::transform (int inputGain, bool lowSidelobes, int smoothing)
{
  for (int i = 0; i < FftSize; ++i) {
    fft_[i] = 0.1f * pcm_[(write_ + i) % FftSize];
    if (lowSidelobes) fft_[i] *= window_[i];
  }
  int size = FftSize;
  int dimension = 1;
  int sign = -1;
  int form = 0;
  four2a_ (fft_.data (), &size, &dimension, &sign, &form);
  ++frameCount_;
  // Cap the averaging time at 180 seconds so long receptions remain responsive.
  double const averageFrames = std::min<quint64> (frameCount_, 180 * 12000 / Hop);
  float const factor = (1.0f / FftSize) * (1.0f / FftSize);
  float const gain = std::pow (10.0f, 0.1f * inputGain);
  for (int i = 0; i < JttySpectrumFrame::BinCount; ++i) {
    float const power = factor * (fft_[2 * i] * fft_[2 * i] + fft_[2 * i + 1] * fft_[2 * i + 1]);
    average_[i] += (power - average_[i]) / averageFrames;
    frame_.cumulative[i] = float (average_[i]);
    frame_.bins[i] = 1000.0f * gain * power;
  }
  if (frameCount_ % 10 == 0) {
    static constexpr int widths[] = {1, 2, 4, 9, 18, 36, 72};
    int width = widths[std::max (0, std::min (smoothing, 6))];
    int smooth = 4 * std::min (10 * width, 150);
    int count = JttySpectrumFrame::BinCount;
    auto& yellow = frame_.linearAverage;
    flat1_ (frame_.cumulative.data (), &count, &smooth, yellow.data ());
    if (width >= 2) {
      std::array<float, JttySpectrumFrame::BinCount> scratch;
      smo_ (yellow.data (), &count, scratch.data (), &width);
      smo_ (yellow.data (), &count, scratch.data (), &width);
    }
    std::fill (yellow.begin (), yellow.begin () + 250, 0);
    int const first = int (500.0f / JttySpectrumFrame::BinWidth) - 1;
    int const last = int (2700.0f / JttySpectrumFrame::BinWidth);
    float const minimum = *std::min_element (yellow.begin () + first, yellow.begin () + last);
    float const maximum = *std::max_element (yellow.begin (), yellow.end ());
    if (maximum > minimum && std::isfinite (maximum) && std::isfinite (minimum)) {
      for (auto& value : yellow) value = std::max (0.0f, 50.0f / (maximum - minimum) * (value - minimum));
    } else {
      yellow.fill (0);
    }
  }
  frame_.row = {reception_, nextSample_ - Hop, nextSample_,
                utcOriginMs_ + (nextSample_ - origin_) / 12, false};
  frame_.powerDb = power_ > 0 ? 10.0 * std::log10 (power_ / Hop) : 0;
  frame_.maxPowerDb = peak_ > 0 ? 20.0f * std::log10 (peak_) : 0;
}
