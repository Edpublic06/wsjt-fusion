# WSJT-Z 2.0.19 Mac rebuild

## References
- Branch: macos-build-maintenance
- Commit at build time: 80ac1bd0a009d6ebfc7b4bdd0f5655a9938cad34
- Script: doc/build-macos-preserve.sh
- Script was untracked at build time, then recorded in commit 31592db.
- No application source changes documented.

## Environment observed in the log
- Architecture: arm64
- C/C++ compilers: AppleClang 17.0.0.17000013
- Fortran: GNU Fortran 16.2.0
- Fortran path: /opt/homebrew/bin/gfortran
- Qt: 5.15.19
- Hamlib: 4.7.2
- Full Homebrew inventory: dependances.txt
- The inventory lists installed packages, not proof that all were used.

## Executed command
```bash
bash doc/build-macos-preserve.sh
```

## Confirmed result at build time
- Build and bundle preparation completed.
- Verification of 20 executables succeeded.
- Signing: codesign result: 0.
- Produced application: build/macos.9Ielpt/stage/wsjtx.app
- Installed application /Applications/wsjtx.app was not replaced.
- No launch or radio operation test had yet been reported in this build record;
  see TEST-2026-10-13.md for the subsequent brief launch test.

## Script behavior
- No brew update or brew install.
- No automatic installation of Homebrew or Apple tools.
- New build directory on every run.
- Records commit, Git state and installed packages.
- Dependencies remain shared with other builds; versions are not pinned.

## Observed warnings
- CMake compatibility and policy warnings.
- C/C++ OpenMP not found; Fortran OpenMP found.
- These messages did not prevent the build.
- "replacing existing signature" accompanied a signing result of 0.

## For a future version
Compare its script and dependency requirements before reusing this script.
Build and test a separate candidate without replacing the working application.
Back up configuration before launch: different bundles can share the same data.
