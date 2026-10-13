# WSJT-Z 2.0.19 for Mac — dependency inventory

## Provenance and scope

Versions were collected on the user's Mac using the command below and supplied
as a Terminal screenshot on 13 October 2026. Terminal showed a last login on
9 October at 11:07:05; that does not precisely date execution of the command.

These versions describe installed packages at inventory time. They do not
retroactively prove which versions were used for the successful 3 October 2026
build. No package was installed or modified to prepare this document.

## Command used

```bash
/opt/homebrew/bin/brew list --versions cmake ninja pkg-config gcc boost fftw hamlib libusb portaudio asciidoctor ccache qt@5
```

The output also shows a Homebrew API data download. The command inspected
packages and refreshed Homebrew data; no package version change was reported.

## Reported versions

```text
asciidoctor 2.0.26
boost 1.92.0
ccache 4.14.1
cmake 4.4.3
fftw 3.3.11
gcc 16.2.0
hamlib 4.7.2
libusb 1.0.30
ninja 1.13.2
pkgconf 3.0.7
portaudio 19.7.0
qt@5 5.15.19
```

The command requested `pkg-config`; Homebrew reports the package as `pkgconf`.

## Context of the retained build

- MacBook Pro Apple Silicon, 16 GB; macOS 15.8.1 in the initial screenshot.
- Built on 3 October 2026, according to the user.
- WSJT-Z source: tag `v2.0.19`, commit `b3a06e89ae0174c51c8595d99e40ca179738eb0a`.
- Displayed version: `2.0.19 - b3a06e`.
- Script used: `doc/build-osx.sh`.
- Historical community release: `v2.0.19-macos-m1` (now retained as a draft).

## Use for rebuilds and updates

Keep this inventory with the build documentation. Collect versions again before
each build and compare them. Future availability through Homebrew is not
guaranteed; this list does not provide a pinned-version installation mechanism.

The exact Homebrew and Apple command-line tool versions and the gfortran
compiler actually selected for the initial build were not recorded. The GCC
formula version alone does not establish the compiler path used by CMake.
