#if defined(__APPLE__) && defined(__aarch64__)
// Fusion adapter for the isolated WSJT-X decoder image. GPL-3.0-or-later.
#include <cstdio>
#include <string>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <limits.h>
namespace {
std::string path(char const* text) {
  std::string result(text,500);
  while(!result.empty() && (result.back()==' ' || result.back()=='\0'))result.pop_back();
  return result;
}
}
extern "C" void* fusion_beta2_symbol(char const* symbol,char const* exe) {
  static void* image=nullptr;
  if(!image) {
    std::string directory;
    if(exe)directory=path(exe);
    else {
      char filename[PATH_MAX];uint32_t size=sizeof(filename);
      if(_NSGetExecutablePath(filename,&size))return nullptr;
      directory=filename;directory=directory.substr(0,directory.find_last_of('/'));
    }
    image=dlopen((directory+"/libfusion_decoder_beta2.dylib").c_str(),RTLD_NOW|RTLD_LOCAL);
  }
  return image?dlsym(image,symbol):nullptr;
}
extern "C" int fusion_beta2_dispatch(float* spectrum,short* audio,void const* params,int rate,
                                     char const* exe,char const* data,char const* temp,char const* share) {
  using Decode=void(*)(float*,short*,void const*,int,char const*,char const*,char const*,char const*);
  auto decode=reinterpret_cast<Decode>(fusion_beta2_symbol("fusion_beta2_decode_impl",exe));
  if(!decode) {
    std::fprintf(stderr,"Fusion beta2 decoder unavailable\n");
    std::printf("<DecoderError> Fusion beta2 engine unavailable\n<DecodeFinished>   0   0        0\n");
    std::fflush(stdout);
    return -1;
  }
  decode(spectrum,audio,params,rate,exe,data,temp,share);
  return 1;
}

#else
extern "C" int fusion_beta2_dispatch(float*,short*,void const*,int,char const*,char const*,char const*,char const*) { return 0; }
#endif
