#ifndef JTTY_RECEIVE_TIMING_HPP
#define JTTY_RECEIVE_TIMING_HPP

namespace Jtty {

constexpr int receiveSamplesPerSymbol = 384;
constexpr int receiveFrameSamples = 59 * receiveSamplesPerSymbol;

}

#endif
