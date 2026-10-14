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
This step creates no JTTY transmit or live-audio path and is not a public release.

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
Upstream interactive row re-decoding/history and transmit are not connected yet.

The imported spectrum regression suite retains its continuous-stream tests.
Its timed-spectrum comparison uses an analytical coherent-tone check instead:
Fusion's baseline symspec has an older ABI and different level conventions.
All imports and adaptations are recorded in `wsjtz/jtty/live/source-manifest.json`.
