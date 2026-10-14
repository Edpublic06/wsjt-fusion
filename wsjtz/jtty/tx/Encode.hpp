// Fusion adapter around WSJT-X JTTY text codec and waveform. GPL-3.0-or-later.
#ifndef FUSION_JTTY_ENCODE_HPP
#define FUSION_JTTY_ENCODE_HPP
#include <QByteArray>
#include <QString>
namespace wsjtz {
struct JttyEncoded { QByteArray pcm; QString text, error; bool substituted=false; };
JttyEncoded encodeJtty(QString const& text, int frequency);
}
#endif
