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
FT8 extensions. The original core import did not connect the application UI or audio/transmit lifecycle. The current development branch now adds continuous local-audio reception, described below; transmission remains unfinished.

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

Nine regressions passed in the separate build: six upstream JTTY tests, FT2 receiver, incoming-call policy and caller queue. All 34 imported JTTY manifest files retain their original SHA-256 hashes. Beta2 still resolves to commit `8345c155a878185f11736e9dec8df278ab7f1c49`.

## Future upstream updates

See [the JTTY tracking and reimport policy](UPDATE-POLICY.md) for the exact upstream baseline, both source inventories, the read-only comparison command and the per-release update record. Imports and Fusion adaptations must remain separate.

## Continuous live receive adapter — development branch

**Mode → JTTY (receive only — experimental)** enables the configured local audio
input and a dedicated text panel. **Monitor** starts/stops reception. Incoming
text grows progressively using upstream `Jtty::Decoder` updates. RX frequency
changes begin a new receive context. The decoder is serialized in its own
`fusion-jtty-rx` process, isolating JTTY DSP scratch from the timed-mode engine.

The Detector adapter bypasses UTC timed-buffer boundaries and uses the upstream
stateful 48-to-12 kHz FIR with a private symbol/state. Provenance and hashes are
in `live/source-manifest.json`. Audio events carry a session ID so stale queued
blocks cannot enter a new context. Backpressure is bounded; overruns stop the
receiver and ask the operator to restart Monitor. Closing the application stops
the worker. Returning to another mode restores the timed Detector path.

This step supports **local sound-card input only**, not TCI audio. The JTTY
spectrum now feeds Wide Graph through the unchanged upstream JttySpectrum
module and adapted upstream WideGraph/plotter bridge. Interactive waterfall row
re-decoding is not connected yet. Local one-message transmission is now
available through the original F1–F8 / Send message page and main Halt Tx button; see the development guide.
Auto CQ/Auto Call/Auto Seq, Enable Tx and Tune are disabled in JTTY receive mode;
the transmit/PTT paths are also guarded. The normal timed transmit buttons remain disabled; JTTY uses its own
asynchronous upstream stream. Live RF transmission still needs operator testing. Until startup mode restoration is integrated, closing in JTTY stores
FT8 as the next startup mode, preserving compatibility with beta2.

Validation (14 October 2026): eleven tests passed, including six upstream JTTY
tests, FT2 and automation regressions, continuous Detector sample conservation
with irregular partitions/session reset/return to timed mode, and progressive
text decoding through the stream process. Hardware input, interactive mode
switching and radio operation still require operator validation. The earlier
WAV panel was confirmed working by the user.

Spectrum validation (14 October 2026): upstream continuous-spectrum regression
cases pass, with a coherent-tone frequency/power check replacing comparison to
the older Fusion timed-spectrum ABI. See `doc/JTTY-DEVELOPMENT.md` and
`live/source-manifest.json` for the exact import/adaptation boundary.

See [the development guide](../../../doc/JTTY-DEVELOPMENT.md) for transmit limits, validation and upstream/adaptation boundaries.
