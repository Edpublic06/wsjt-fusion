#ifndef JTTY_DECODER_HPP
#define JTTY_DECODER_HPP

#include <QString>
#include <QVector>
#include <QtGlobal>

#include <array>
#include <cstdint>

#include "JttyReceiveTiming.hpp"

extern "C" {
std::int32_t jtty_rx_create ();
void jtty_rx_destroy (std::int32_t handle);
void jtty_rx_begin (std::int32_t handle, std::int64_t sessionId,
                    std::int64_t gridOrigin, std::int64_t firstSearch,
                    std::int32_t samplesPerSymbol);
std::int32_t jtty_rx_process (std::int32_t handle, std::int16_t const * samples,
                             std::int32_t count, std::int64_t firstSample,
                             std::int64_t stopSample, std::int32_t maxSteps,
                             std::int32_t lowFrequency, std::int32_t highFrequency,
                             float centerFrequency, float tolerance);
std::int64_t jtty_rx_next_required_sample (std::int32_t handle);
std::int64_t jtty_rx_next_search_sample (std::int32_t handle);
std::int32_t jtty_rx_take_updates (std::int32_t handle, std::int32_t capacity,
                                  char * text, std::int64_t * ids, float * frequencies,
                                  double * starts, double * latest, std::int32_t * terminals,
                                  std::int32_t * snrs);
void jtty_rx_end (std::int32_t handle, std::int32_t reason);
}

namespace Jtty {

enum class ReceiveTerminal : std::int32_t
{
  Growing = 0,
  Complete = 1,
  Expired = 2,
  ReceptionEnded = 3
};

struct ReceiveUpdate
{
  qint64 messageId {};
  float frequency {};
  QString text;
  double startSeconds {};
  double latestSeconds {};
  ReceiveTerminal terminal {ReceiveTerminal::Growing};
  int snr {-10};
};

// Contexts share DSP scratch and must be called from one serialized decoder owner.
class Decoder
{
public:
  Decoder () : handle_ {jtty_rx_create ()} {}
  ~Decoder () { if (handle_) jtty_rx_destroy (handle_); }
  Decoder (Decoder const&) = delete;
  Decoder& operator= (Decoder const&) = delete;

  bool valid () const { return handle_ != 0; }

  void begin (qint64 sessionId, qint64 gridOrigin, qint64 firstSearch,
              int samplesPerSymbol = receiveSamplesPerSymbol)
  {
    jtty_rx_begin (handle_, sessionId, gridOrigin, firstSearch, samplesPerSymbol);
  }

  int process (qint16 const * samples, int count, qint64 firstSample,
               qint64 stopSample, int maxSteps, int lowFrequency,
               int highFrequency, float centerFrequency, float tolerance)
  {
    return jtty_rx_process (handle_, samples, count, firstSample, stopSample,
                            maxSteps, lowFrequency, highFrequency, centerFrequency, tolerance);
  }

  qint64 nextRequiredSample () const { return jtty_rx_next_required_sample (handle_); }
  qint64 nextSearchSample () const { return jtty_rx_next_search_sample (handle_); }

  QVector<ReceiveUpdate> takeUpdates ()
  {
    constexpr int capacity = 30;
    std::array<char, capacity * 80> text;
    std::array<std::int64_t, capacity> ids;
    std::array<float, capacity> frequencies;
    std::array<double, capacity> starts, latest;
    std::array<std::int32_t, capacity> terminals;
    std::array<std::int32_t, capacity> snrs;
    QVector<ReceiveUpdate> result;
    for (;;) {
      auto const count = jtty_rx_take_updates (handle_, capacity, text.data (), ids.data (),
                                               frequencies.data (), starts.data (), latest.data (),
                                               terminals.data (), snrs.data ());
      for (int i = 0; i < count; ++i) {
        result.push_back ({ids[i], frequencies[i], QString::fromLatin1 (text.data () + i * 80, 80).trimmed (),
                           starts[i], latest[i], static_cast<ReceiveTerminal> (terminals[i]), snrs[i]});
      }
      if (count < capacity) break;
    }
    return result;
  }

  void end (ReceiveTerminal reason = ReceiveTerminal::ReceptionEnded)
  {
    jtty_rx_end (handle_, static_cast<std::int32_t> (reason));
  }

private:
  std::int32_t handle_ {};
};

}

#endif
