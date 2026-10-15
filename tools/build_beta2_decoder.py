#!/usr/bin/env python3
"""Build the pinned WSJT-X engine without importing its GUI into Fusion."""
import argparse,hashlib,json,os,subprocess,tarfile
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--output',type=Path,required=True);p.add_argument('--fortran',required=True);p.add_argument('--prefix',default='/opt/homebrew');p.add_argument('--qt',required=True);p.add_argument('--upstream-source',type=Path)
a=p.parse_args();root=Path(__file__).resolve().parents[1];module=root/'wsjtz/decoding/wsjtx-beta2'
manifest=json.loads((module/'source-manifest.json').read_text());archive=module/manifest['archive']
if hashlib.sha256(archive.read_bytes()).hexdigest()!=manifest['archive_sha256']:raise SystemExit('Upstream archive hash mismatch')
a.build=a.build.resolve();a.build.mkdir(parents=True,exist_ok=True)
source=a.upstream_source.resolve() if a.upstream_source else a.build/'source'
if not source.exists():
 source.mkdir()
 with tarfile.open(archive) as tf:
  for member in tf.getmembers():
   if '..' in Path(member.name).parts or member.name.startswith('/'):raise SystemExit('Invalid upstream archive path')
 subprocess.run(['tar','-xzf',str(archive),'--strip-components=1','-C',str(source)],check=True)
for relative,digest in manifest['upstream_files'].items():
 if hashlib.sha256((source/relative).read_bytes()).hexdigest()!=digest:raise SystemExit('Changed upstream source: '+relative)
build=a.build/'objects'
def run(args):subprocess.run([str(x) for x in args],check=True)
run(['cmake','-S',source,'-B',build,'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_OSX_ARCHITECTURES=arm64','-DCMAKE_OSX_DEPLOYMENT_TARGET=11.0','-DCMAKE_POSITION_INDEPENDENT_CODE=ON','-DCMAKE_Fortran_COMPILER='+a.fortran,'-DCMAKE_PREFIX_PATH='+a.prefix+';/opt/homebrew/opt/qt@5','-DQt5_DIR='+a.qt,'-DFFTW3_ROOT_DIR='+a.prefix,'-DHamlib_ROOT='+a.prefix,'-DUsb_ROOT='+a.prefix,'-DWSJT_SKIP_MANPAGES=ON','-DWSJT_GENERATE_DOCS=OFF','-DWSJT_ENABLE_TESTS=OFF','-DWSJT_FORTRAN_LIBRARY_VARIANTS=BOTH'])
run(['cmake','--build',build,'--target','wsjt_fort_omp','wsprd','-j6'])
obj=a.build/'DecodeBridge.o'
run([a.fortran,'-O2','-fPIC','-fopenmp','-I'+str(build/'fortran_modules_omp'),'-c',module/'adapters/DecodeBridge.f90','-o',obj])
a.output=a.output.resolve();a.output.parent.mkdir(parents=True,exist_ok=True)
run([a.fortran,'-dynamiclib','-fopenmp','-o',a.output,obj,build/'libwsjt_fort_omp.a',build/'libwsjt_cxx.a','-L'+a.prefix+'/lib','-lfftw3f_threads','-lfftw3f','-lfftw3_threads','-lfftw3','-lc++','-lboost_log_setup','-lboost_log','-lboost_thread','-lboost_filesystem','-lboost_chrono','-lboost_atomic','-Wl,-u,_hspec_,-u,_avecho_,-u,_freqcal_,-u,_save_echo_params_,-u,_fast_decode_'])

import shutil
shutil.copy2(build/"wsprd",a.output.parent/"wsprd-beta2")
