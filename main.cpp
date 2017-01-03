#include "audioeffect.h"

extern AudioEffect* createEffectInstance(audioMasterCallback audioMaster);

extern "C" {

#if defined(__GNUC__) && \
    ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
#define VST_EXPORT __attribute__((visibility("default")))
#else
#define VST_EXPORT
#endif

VST_EXPORT AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
  if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0)) return 0;  // old version

  AudioEffect* effect = createEffectInstance(audioMaster);
  if (!effect) return 0;

  return effect->getAeffect();
}

#if (TARGET_API_MAC_CARBON && __ppc__)
VST_EXPORT AEffect* main_macho(audioMasterCallback audioMaster) {
  return VSTPluginMain(audioMaster);
}
#elif WIN32
VST_EXPORT AEffect* MAIN(audioMasterCallback audioMaster) {
  return VSTPluginMain(audioMaster);
}
#elif BEOS
VST_EXPORT AEffect* main_plugin(audioMasterCallback audioMaster) {
  return VSTPluginMain(audioMaster);
}
#endif
}
// extern "C"

//------------------------------------------------------------------------
#if WIN32
#include <windows.h>
void* hInstance;

extern "C" {
VST_EXPORT __declspec(dllexport) int main(audioMasterCallback audioMaster) {
  // Get VST Version
  if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0)) return 0;  // old version

  // Create the AudioEffect
  AudioEffect* effect = createEffectInstance(audioMaster);
  if (!effect) return 0;

  return (int)effect->getAeffect();
}
}  // extern "C"

extern "C" {
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID lpvReserved) {
  hInstance = hInst;
  return 1;
}
}  // extern "C"
#endif
