# JTTY development and rollback

Published beta2 remains tagged `v1.0.0-beta2` at
`8345c155a878185f11736e9dec8df278ab7f1c49`.
Its branch, tag, release assets and installed application are not changed by
this development branch. `backup/beta2` points to that exact source reference.
A local Git bundle was also created before development and verified complete.

New branch: `fusion-jtty-integration`; application identity: 1.0.0-beta3-dev.
Work is performed in a separate worktree and build directory. Do not overwrite
beta2 bundles or reuse their build directories. To roll back, close the development
application and launch the preserved beta2 app. Development and beta2 still share
the historical WSJT-X settings/log identity; do not run them against the same
radio simultaneously. Back up data before any later live-mode testing.

First step: isolated offline JTTY WAV receive panel. See
[the JTTY module notes](../wsjtz/jtty/README.md#fusion-development-adapter--offline-wav-reception).
That initial step created no transmit/live-audio path. The later development
steps below add those paths; no public release is created here.

### Agreed scope and sequence — 14 October 2026

F6FDY requested that reliable JTTY integration based on the original WSJT-X
code and UI takes priority. Preserve the upstream display/QSO/logging audit
for the next phase. **Do not implement Fusion automatic QSO logging yet.**
Once JTTY operation has been implemented reliably and validated, discuss the
Fusion-specific automatic logging design separately. Manual review through
the existing Log QSO dialog remains available during development.

### Receive dial frequencies

JTTY uses its own entries in Settings → Frequencies. Insert an entry with mode
JTTY and the desired dial frequency in MHz, then select it from the main frequency
list. Existing personal frequency tables are preserved; no FT8 frequency is
substituted. If no JTTY entry exists, entering the mode keeps the current radio
frequency. For reference, upstream WSJT-X v3.3.0-beta1 lists 14.090 MHz for 20 m
JTTY in models/FrequencyList.cpp. The RX audio frequency control is separate
from this radio dial frequency.

### Live spectrum and waterfall

The continuous JTTY spectrum implementation (`widgets/JttySpectrum.hpp/.cpp`)
is compiled unchanged from WSJT-X v3.3.0-beta1. It uses the upstream 16384-point
FFT, 3456-sample hop, windowing, power estimates and rolling spectrum averages.
The upstream WideGraph averaging and drawing methods feed Fusion's existing
Wide Graph and signal meter from the same PCM blocks as the decoder.

The Fusion plotter adds isolated JTTY branches for the upstream cumulative and
linear-average spectra and continuous UTC labels. Timed FT8/FT4 buffers are not
used for JTTY plotting. The temporary separate FFT monitor has been removed.
Upstream interactive row re-decoding/history is not connected yet.
See the transmit section below for the local one-message send adapter.

The imported spectrum regression suite retains its continuous-stream tests.
Its timed-spectrum comparison uses an analytical coherent-tone check instead:
Fusion's baseline symspec has an older ABI and different level conventions.
All imports and adaptations are recorded in `wsjtz/jtty/live/source-manifest.json`.

### Experimental local transmission

The development application now embeds the original upstream JTTY UI page:
RX/TX offsets, F Tol, F1–F8 with editable macros, Contest/FT8 message style,
Send message, frame progress, Call next and Serial Number. F1 sends the CQ
template; Send message sends the editor text once. The existing main-window
Halt Tx button stops transmission. Both use the TX audio offset, configured output
device/channel, output attenuation, radio PTT method and TX lead delay.
The radio must acknowledge PTT before audio playback starts.

The upstream `genjtty_profile` and `gen_jttywave` codec/waveform sources remain
unchanged, as do `JttyMessages`, `TxAudioQueue`, `JttyPcmFifo`, `JttyTxStream`
and `JttyTxLifecycle`. Their exact provenance is recorded in the live manifest.
Fusion's adapters are `wsjtz/jtty/tx/Encode.cpp` and
`widgets/mainwindow_jtty_tx.cpp`; the extracted form and adapter live in
`wsjtz/jtty/ui/JttyTransmitPanel.ui` and `TransmitPanel.cpp`. SoundOutput exposes the device buffer size
so the upstream stream can wait for the full audio tail to drain.

Sending locks frequency/settings controls, suspends reception, then releases
PTT and restores reception if Monitor was active. Halt Tx, radio/audio errors,
mode changes, application close and PTT/queue/completion timeouts cancel the
operation. Stale callbacks are checked against the queue epoch.

Limits: one message at a time, at most 320 characters and 60 seconds of
rendered audio. No continuous typing/filler, queued sends, TCI output or
interactive waterfall replay is claimed. The published beta2 remains unchanged.

The Cumulative 2D spectrum now uses the same scratch-copy/decibel conversion
as upstream; this fixes the previously nearly flat cumulative JTTY curve.

Validation: upstream stream and lifecycle tests plus offline codec/waveform
to continuous-receiver loopback. These tests do not key a radio. Actual
transmit level, PTT operation and a complete over-the-air QSO require operator
validation on the development application. Live receive was confirmed by F6FDY.

### Original UI and remaining receive/logging integration

The imported form keeps the upstream page layout and widget names. Runtime
wiring supports one-shot sends and F1–F8 text expansion, style-specific template
persistence under Fusion/JTTY, RX/TX synchronization, tolerance and progress.
Live typing shortcuts are not advertised until that lifecycle is implemented.
The 200-line RX/TX transcript survives receive-worker restarts.

The native-atom macro encoder, selected-message metadata, QSO-pane filtering,
click-to-call and contest automatic log trigger are not claimed by this step.
See [the upstream QSO audit](JTTY-UPSTREAM-QSO-AUDIT.md) for precisely what
WSJT-X implements and where. The existing Log QSO form can be used manually;
review the fields, since automatic JTTY-to-QSO extraction is not connected yet.

### Embedded panel height fix

Fusion's controls stack has a legacy 220-pixel height cap. The original JTTY
page needs more space at the configured font size, so this cap clipped native
macOS controls. The unchanged upstream `CurrentPageHeightStackedWidget` now
sizes the stack from the active page; entering JTTY removes the cap and leaving
restores the previous maximum. The upstream JTTY page itself is unchanged.
An embedded large-font layout regression checks buttons/fields are not squeezed
below their native minimum size and that the ordinary page can shrink again.

For a changing instantaneous audio trace, select **Current** in Wide Graph.
**Cumulative** uses the upstream long-term average (up to roughly 180 seconds)
and can look nearly stationary; 2D gain controls its visual amplitude.
No personal spectrum setting is rewritten by the layout fix.

### Restore pane proportions when leaving JTTY

The JTTY page is removed from the stacked layout when inactive, so its wide
macro grid cannot impose a hidden minimum width on ordinary modes. Its widget
is retained and reattached on the next JTTY entry. The decoder splitter state
is captured before entering JTTY and restored after the ordinary-mode layout
settles. Closing while in JTTY saves the pre-JTTY splitter state, not the
JTTY-expanded proportions. A splitter/layout regression checks the transition.

### Selected correspondent (upstream parity)
JTTY double-click now fills DX Call using the word captured on mouse press.
The report macro uses the upstream bounded exact-word SNR history lookup,
with -10 dB fallback. Other modes retain their existing selection path.
Automatic QSO logging remains deferred until JTTY is reliable.

The selected JTTY correspondent also updates Fusion’s separate Call Info DX Call
field through the existing lookup path, clearing the previous station’s details.
No grid is inferred from unstructured JTTY text.

In JTTY, a single left click on a callsign highlights it in yellow and updates
DX Call and message variables. A double left click additionally sends the current
F2 template when the message style is FT8, using the existing busy/TX guards.
Other words do not change the correspondent or trigger transmission.
The highlight is cleared when leaving JTTY.

### Configured decode colors
Both JTTY receive panes use the existing Colors settings and their enabled
priorities. TX, own callsign and CQ apply to lines. Other explicit callsigns
are colored individually using the existing logbook new-call, DXCC, continent,
CQ/ITU-zone, band and LoTW rules. Grid coloring is not inferred from free text.
Multiple callsigns on a line are evaluated independently; no sender is assumed.
Colors do not alter selection, logging or automation decisions.

JTTY hides the timed-mode progress bar and restores it on mode exit. Enable Tx
shows red during JTTY audio transmission and restores its previous style on
completion/abort and mode exit; this visual indication does not enable timed TX.
