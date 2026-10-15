# WSJT-X 3.3.0-beta2 decoder boundary — Fusion beta3

Pinned upstream: `WSJTX/wsjtx`, `v3.3.0-beta2`,
`9d059511f77e2d1be746ebfeaa7f88e77c1babb7`.

This macOS Apple Silicon integration compiles the upstream Fortran/C/C++ library
unchanged into `libfusion_decoder_beta2.dylib`. The compressed official source
snapshot, archive SHA-256 and per-file inventory are in `source-manifest.json`.
COPYING/AUTHORS and original source notices are retained inside that snapshot.
`tools/build_beta2_decoder.py` verifies it and builds it offline using installed
dependencies. No upstream GUI target is compiled or imported into Fusion.

- Standard timed decoders (FT8, FT4, JT4, JT9, JT65, combined JT9/JT65, Q65,
  FST4/FST4W) use `DecodeBridge.f90` via the isolated library.
- The adapter explicitly copies the inherited Fusion parameter fields rather
  than sharing WSJT-X's new IPC memory header. Fusion's own shared-memory
  handshake and output parser remain in place.
- MSK144/fast spectra, FreqCal and Echo use narrow ABI adapters in
  `GuiDecoders.cpp`. Echo spectral buffers are copied across the boundary so
  the existing Fusion graph remains usable. The removed upstream `ndf` argument
  and the nine-argument metadata contract are adapted explicitly.
- WSPR uses the unmodified beta2 `wsprd` executable.
- FT2 remains on the separate WS/Improved 3.2.1 build 261010 adapter. Its codec
  symbols are not substituted by the standard FT4 engine.
- Fusion UI, automation, settings, TX protocols and existing integrations are
  retained. This is not a wholesale WSJT-X application upgrade.
- Other platforms keep the old decoder path until their port is built/tested;
  the beta3 artifact produced here is for macOS Apple Silicon only.

The isolated image is a required component on Apple Silicon. Missing it is an
installation error, not an opportunity to silently label the old decoder beta2.

## Validation

The actual Fusion shared-memory worker is tested offline with synthetic FT8 and
FT4 audio and the expected decoded message plus completion handshake. A second
smoke test exercises the actual MSK spectrum, FreqCal, Echo silence and metadata
adapters. Official upstream engine tests cover FT8/FT4/JT4/JT9/JT65/Q65/FST4,
and upstream MSK spectrum/Echo regressions are also run. None opens a radio or
audio device. Live radio behavior and comparative weak-signal performance must
still be confirmed by the operator.

## Updating

Keep the previous release and bundle. Import a new immutable source snapshot and
inventory, inspect API/parameter changes, rebuild the isolated engine, and rerun
both upstream and Fusion boundary tests. Report upstream interface/ergonomics
changes separately and obtain F6FDY's explicit decision before importing them.
Any WSJT-Z feature/correction import also needs explicit approval.
