#!/usr/bin/env python3
"""Create a fresh beta3 test bundle; never overwrite an installed/running app."""
import argparse,hashlib,json,plistlib,shutil,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--base',type=Path,required=True);p.add_argument('--build',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
root=Path(__file__).resolve().parents[1];a.output=a.output.resolve();a.build=a.build.resolve()
if a.output.exists():raise SystemExit('Choose a new output bundle')
shutil.copytree(a.base,a.output,symlinks=True);contents=a.output/'Contents';mac=contents/'MacOS';frameworks=contents/'Frameworks'
for source,name in [(a.build/'wsjtx.app/Contents/MacOS/wsjtx','wsjtx'),(a.build/'jt9','jt9'),(a.build/'libfusion_decoder_beta2.dylib','libfusion_decoder_beta2.dylib'),(a.build/'wsprd-beta2','wsprd'),(a.build/'fusion-jtty-rx','fusion-jtty-rx')]:shutil.copy2(source,mac/name)
info=plistlib.loads((contents/'Info.plist').read_bytes());info.update(CFBundleName='WSJT-Fusion',CFBundleDisplayName='WSJT-Fusion 1.0.0-beta3',CFBundleLongVersionString='WSJT-Fusion 1.0.0-beta3',CFBundleShortVersionString='1.0.0',CFBundleVersion='1.0.0.3');(contents/'Info.plist').write_bytes(plistlib.dumps(info))
queue=list(mac.iterdir());seen=set()
while queue:
 target=queue.pop()
 if target in seen or not target.is_file():continue
 seen.add(target)
 process=subprocess.run(['otool','-L',str(target)],capture_output=True,text=True)
 if process.returncode:continue
 for line in process.stdout.splitlines()[1:]:
  dep=line.strip().split(' (',1)[0]
  if not dep.startswith('/opt/homebrew/'):continue
  suffix=dep.split('/lib/',1)[1] if '.framework/' in dep else Path(dep).name
  dest=frameworks/suffix
  if not dest.exists():
   if '.framework/' in dep:
    prefix=dep.split('.framework/',1)[0]+'.framework';shutil.copytree(prefix,frameworks/Path(prefix).name,symlinks=True)
   else:shutil.copy2(dep,dest)
   queue.append(dest)
  subprocess.run(['install_name_tool','-change',dep,'@executable_path/../Frameworks/'+suffix,str(target)],check=True,capture_output=True)
subprocess.run(['install_name_tool','-id','@executable_path/libfusion_decoder_beta2.dylib',str(mac/'libfusion_decoder_beta2.dylib')],check=True)
resources=contents/'Resources'/'Fusion-Source-Notices';resources.mkdir(exist_ok=True)
for name in ['NOTICE.md','COPYING','UPSTREAM-SOURCES.md']:shutil.copy2(root/name,resources/name)
source=root/'wsjtz/decoding/wsjtx-beta2/source-manifest.json';shutil.copy2(source,resources/'decoder-source-manifest.json')
subprocess.run(['codesign','--force','--deep','--sign','-','--timestamp=none','--entitlements',str(root/'Darwin/entitlements.plist'),str(a.output)],check=True)
subprocess.run(['codesign','--verify','--deep','--strict',str(a.output)],check=True)
print(a.output)
