# JTTY core from WSJT-X 3.3.0-beta1

The imported sources are unchanged from tag `v3.3.0-beta1`, commit
`57ba9337fa020065dc0ceee2c4ef85e433ea32d4`. Their paths and SHA-256 checksums
are recorded in `source-manifest.json`. Credits: Joe Taylor K1JT, the WSJT
Development Group and the contributors identified by upstream history and
notices. Licence: GPL-3.0-or-later; see NOTICE.md and COPYING in the root.

The current import covers source encoding, FEC, wave generation, continuous
decoding, the receive-context API, standalone `sjtty`/`rjtty` tools, selected
unchanged upstream tests, documentation and the upstream WAV sample.

The core compiles alongside the existing Z decoder without replacing its
FT8 extensions. The application UI and live audio/transmit lifecycle are
**not yet connected**. No JTTY Mode menu entry is exposed as usable at this
stage. This is not the completed mode integration.

Validation on macOS arm64, 13 October 2026: source-codec, frequency-window,
continuous-decode, update-delivery, receive-state and TBCC-decoder tests passed.
The standalone receiver also decoded text from the upstream sample. These
checks do not validate GUI operation, live audio, CAT/PTT or radio transmission.

The C++ receive wrapper and timing constants are imported unchanged to support
the next adapter stage. Continuous reception must have one serialized decoder
owner, as documented by the upstream wrapper.

## Fusion development adapter — offline WAV reception

Development branch: **fusion-jtty-integration**, separate from published beta2.
In this branch, **Decode → JTTY WAV receive test (experimental)…** opens an
offline receiver panel with center frequency, tolerance, output and Stop.
It accepts standard 44-byte-header, 12 kHz mono PCM16 WAV files up to 180 s.
The upstream sample is `samples/JTTY/260807_134110.wav`.

The adapter runs the unchanged `rjtty` tool in a separate process, copying the
selected recording into a temporary folder with a short filename. This avoids
the upstream filename length limit and separates DSP scratch from normal
application decoding. Closing the panel stops its child process. No live audio,
CAT/PTT, transmission, Auto Call or settings conversion is performed by the panel.

Build with `WSJT_BUILD_UTILS=ON`; bundle `rjtty` beside the application executable.
The adapter lives in `wsjtz/jtty/ui/`; imported upstream sources remain unchanged.
This is a first application receive integration step, not a completed live mode.
Next steps are a continuous receive owner/audio adapter, the JTTY operating UI,
and transmit lifecycle/radio integration with explicit stop and ownership tests.

Validation of the offline adapter step (14 October 2026): application/rjtty build passed on macOS arm64; upstream sample decoded with the panel command parameters, producing `RAN ALL NIGHT ON BAND NOISE - NO FALSE DECODES!`. Full interactive panel testing remains to be performed.
