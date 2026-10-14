#include <QtTest>
#include <QCryptographicHash>
#include <cmath>
#include <memory>
#include <vector>

#include "commons.h"
#include "widgets/JttySpectrum.hpp"

extern "C" {
  void symspec_ (struct dec_data *, int *, double *, int *, int *, bool *, int *,
                 float *, float *, float *, int *, int *, float *, int *);
}

namespace {
std::vector<qint16> fixture (int count)
{
  std::vector<qint16> result (count);
  quint32 random = 17;
  for (int i = 0; i < count; ++i) {
    random = 1664525u * random + 1013904223u;
    result[i] = 1000 * std::sin (2 * 3.141592653589793 * 1234 * i / 12000)
        + int (random >> 24) - 128;
  }
  return result;
}

QByteArray fingerprint (JttySpectrumFrame const& frame)
{
  QCryptographicHash hash {QCryptographicHash::Sha256};
  for (auto const * values : {&frame.bins, &frame.cumulative, &frame.linearAverage}) {
    hash.addData (reinterpret_cast<char const *> (values->data ()), sizeof (*values));
  }
  return hash.result ();
}


}

class TestJttySpectrum final : public QObject
{
  Q_OBJECT
private slots:
  // Fusion retains an older timed symspec ABI and level convention. Validate
  // the imported continuous spectrum against a known coherent tone instead.
  void coherentToneHasExpectedFrequencyAndLevel ()
  {
    int const bin = 1000;
    std::vector<qint16> pcm (8 * JttySpectrum::Hop);
    double energy = 0;
    for (int i=0;i<int(pcm.size());++i) {
      pcm[i] = std::lround (1000 * std::sin (2 * 3.141592653589793 * bin * i / JttySpectrum::FftSize));
      if (i >= int(pcm.size()) - JttySpectrum::Hop) energy += double(pcm[i])*pcm[i];
    }
    JttySpectrum spectrum;
    spectrum.begin (1,0,0);
    int frames = 0;
    QVERIFY (spectrum.process (0,pcm.data (),int(pcm.size ()),0,false,0,
      [&] (JttySpectrumFrame const& frame) {
        ++frames;
        if (frame.row.endSample == qint64(pcm.size ())) {
          auto peak = std::max_element (frame.bins.begin (),frame.bins.end ());
          QCOMPARE (int(peak-frame.bins.begin ()),bin);
          QVERIFY (std::abs (*peak - 2500000.f) < 2000.f);
          QVERIFY (std::abs (frame.powerDb - 10*std::log10(energy/JttySpectrum::Hop)) < 0.002);
          QCOMPARE (frame.row.utcEndMs,qint64(pcm.size()/12));
        }
      }));
    QCOMPARE (frames,8);
  }

  void packetizationAndLargeCoordinatesPreserveSpectrum ()
  {
    auto const pcm = fixture (181 * 12000);
    qint64 const origin = (qint64 (1) << 33) + 117;
    qint64 const utc = 179000;
    JttySpectrum whole;
    JttySpectrum packets;
    whole.begin (7, origin, utc);
    packets.begin (7, origin, utc);
    QVector<QByteArray> expected;
    QVERIFY (whole.process (origin, pcm.data (), int (pcm.size ()), 0, true, 1,
        [&] (JttySpectrumFrame const& frame) { expected.append (fingerprint (frame)); }));
    int frameIndex = 0;
    int offset = 0;
    while (offset < int (pcm.size ())) {
      int const size = std::min (1 + (offset * 17LL % 4001), qint64 (pcm.size () - offset));
      QVERIFY (packets.process (origin + offset, pcm.data () + offset, size, 0, true, 1,
          [&] (JttySpectrumFrame const& frame) {
        QCOMPARE (fingerprint (frame), expected.at (frameIndex));
        QCOMPARE (frame.row.reception, quint64 (7));
        QCOMPARE (frame.row.beginSample, origin + qint64 (frameIndex) * JttySpectrum::Hop);
        QCOMPARE (frame.row.utcEndMs, utc + qint64 (frameIndex + 1) * JttySpectrum::Hop / 12);
        ++frameIndex;
      }));
      offset += size;
    }
    QCOMPARE (frameIndex, expected.size ());
    QVERIFY (!packets.process (origin, pcm.data (), 100, 0, true, 1, [] (JttySpectrumFrame const&) {}));
  }

  void explicitReceptionResetsTailAndAverage ()
  {
    auto const pcm = fixture (12 * JttySpectrum::Hop);
    JttySpectrum reused;
    reused.begin (1, 0, 0);
    QVERIFY (reused.process (0, pcm.data (), int (pcm.size ()) - 13, 0, true, 1,
                            [] (JttySpectrumFrame const&) {}));
    reused.begin (2, 999, 15000);
    JttySpectrum fresh;
    fresh.begin (2, 999, 15000);
    QVector<QByteArray> expected;
    QVERIFY (fresh.process (999, pcm.data (), int (pcm.size ()), 0, true, 1,
        [&] (JttySpectrumFrame const& frame) { expected.append (fingerprint (frame)); }));
    int index = 0;
    QVERIFY (reused.process (999, pcm.data (), int (pcm.size ()), 0, true, 1,
        [&] (JttySpectrumFrame const& frame) { QCOMPARE (fingerprint (frame), expected.at (index++)); }));
    QCOMPARE (index, expected.size ());
  }

  void averagesFollowNewSignalsAfterLongReception ()
  {
    JttySpectrum spectrum;
    spectrum.begin (3, 0, 0);
    std::array<qint16, JttySpectrum::Hop> pcm;
    JttySpectrumFrame latest;
    qint64 nextSample = 0;
    auto receiveTone = [&] (int frequency, int frames) {
      for (int i = 0; i < int (pcm.size ()); ++i) {
        pcm[i] = 1000 * std::sin (2 * 3.141592653589793 * frequency * i / 12000);
      }
      for (int i = 0; i < frames; ++i) {
        QVERIFY (spectrum.process (nextSample, pcm.data (), int (pcm.size ()), 0, true, 0,
            [&] (JttySpectrumFrame const& frame) { latest = frame; }));
        nextSample += pcm.size ();
      }
    };
    int const oldBin = int (1500 / JttySpectrumFrame::BinWidth);
    int const newBin = int (2250 / JttySpectrumFrame::BinWidth);
    receiveTone (1500, 720 * 12000 / JttySpectrum::Hop);
    float const oldPower = latest.cumulative[oldBin];
    QVERIFY (oldPower > 0);
    receiveTone (2250, 180 * 12000 / JttySpectrum::Hop);
    QVERIFY (latest.cumulative[oldBin] < 0.5f * oldPower);
    QVERIFY (latest.cumulative[newBin] > 1.5f * latest.cumulative[oldBin]);
    QVERIFY (latest.linearAverage[newBin] > latest.linearAverage[oldBin]);
    QCOMPARE (latest.row.reception, quint64 (3));
    QCOMPARE (latest.row.endSample, nextSample);
    QCOMPARE (latest.row.utcEndMs, nextSample / 12);
  }
};

QTEST_APPLESS_MAIN (TestJttySpectrum)
#include "test_jtty_spectrum.moc"
