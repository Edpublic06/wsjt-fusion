# JTTY UI/audio reference sources

Retrieved on 14 October 2026 from official WSJT-X tag **v3.3.0-beta1**, commit
`57ba9337fa020065dc0ceee2c4ef85e433ea32d4`.

The `reference-ui-audio/` tree retains unchanged JTTY receive/transmit controllers,
UI components, PCM FIFO and transmit stream, audio mailbox, recording/replay,
related unit/integration tests, and application integration context (main window,
configuration, detector/modulator, main entry point and CMake).

Exact source paths and SHA-256 values are recorded in
[ui-audio-source-manifest.json](ui-audio-source-manifest.json). This snapshot is
**reference material only** and is not included in the application build. The
previous JTTY core import and beta2 source remain unchanged. Do not replace
Fusion's main window or configuration wholesale with these upstream versions.

Credits: Joe Taylor K1JT, the WSJT Development Group and contributors named in
upstream files/history. Upstream COPYING, AUTHORS and the license notice are
included. Existing file notices remain applicable; see the repository NOTICE.md.

Next integration work: map continuous receive events from Detector into a
serialized decoder owner, connect the operating UI and mode transition lifecycle,
then adapt the transmit queue/PCM stream with explicit PTT and stop handling.
Use upstream tests as behavior references and retain Fusion automation boundaries.
