#!/usr/bin/env python3
"""Refresh an existing bundled test app without mixing bundled and Homebrew Qt.

Personal integration tooling, 2026-10-13. GPL-3.0-or-later.
Requires an already installed/fixed-up bundle containing its dependencies.
"""
import argparse
from pathlib import Path
import shutil
import subprocess

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("executable", type=Path)
parser.add_argument("bundle", type=Path)
parser.add_argument("--entitlements", required=True, type=Path)
args = parser.parse_args()
bundle = args.bundle.resolve()
target = bundle / "Contents/MacOS/wsjtx"
frameworks = bundle / "Contents/Frameworks"
if not target.is_file() or not frameworks.is_dir():
    parser.error("An existing complete test bundle is required")
if args.executable.resolve() != target:
    shutil.copy2(args.executable, target)
dependencies = subprocess.check_output(["otool", "-L", str(target)], text=True)
for line in dependencies.splitlines()[1:]:
    dependency = line.strip().split(" (", 1)[0]
    if not dependency.startswith("/opt/homebrew/"):
        continue
    suffix = (dependency.split("/lib/", 1)[1]
              if ".framework/" in dependency else Path(dependency).name)
    if not (frameworks / suffix).exists():
        raise RuntimeError(f"Bundled dependency missing: {suffix}")
    subprocess.run(["install_name_tool", "-change", dependency,
                    "@executable_path/../Frameworks/" + suffix, str(target)],
                   check=True, capture_output=True)
subprocess.run(["codesign", "--force", "--deep", "--sign", "-",
                "--timestamp=none", "--entitlements", str(args.entitlements),
                str(bundle)], check=True)
subprocess.run(["codesign", "--verify", "--deep", "--strict", str(bundle)], check=True)
