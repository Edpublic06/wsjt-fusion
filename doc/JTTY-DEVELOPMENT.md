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
