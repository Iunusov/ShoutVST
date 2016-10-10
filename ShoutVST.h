#pragma once
#include <audioeffectx.h>
#include "LibShoutWrapper.h"
#include "ShoutVSTEditor.h"
#include "ShoutVSTEncoderMP3.h"
#include "ShoutVSTEncoderOGG.h"

#include <atomic>

class ShoutVST : public AudioEffectX {
 public:
  explicit ShoutVST(audioMasterCallback audioMaster);
  ~ShoutVST();
  virtual void processReplacing(float** inputs, float** outputs,
                                VstInt32 sampleFrames) override;
  void connect();
  void disconnect();
  virtual bool getEffectName(char* name) override;
  virtual bool getVendorString(char* text) override;
  virtual bool getProductString(char* text) override;
  virtual VstPlugCategory getPlugCategory() override;
  virtual VstInt32 getVendorVersion() override;
  bool IsConnected();
  int GetBitrate();
  int GetTargetSampleRate();
  void UpdateMetadata(const string& metadata);

 private:
  std::atomic<bool> bStreamConnected;
  std::atomic<bool> bConnecting;
  ShoutVSTEncoder* encSelected = nullptr;
  ShoutVSTEncoderOGG* encOGG = nullptr;
  ShoutVSTEncoderMP3* encMP3 = nullptr;
  ShoutVSTEditor* pEditor = nullptr;
  LibShoutWrapper libShoutWrapper;
};
