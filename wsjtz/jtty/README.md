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
