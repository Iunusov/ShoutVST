#include "ShoutVST.h"
#include <thread>
#include "version.h"

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
  return new ShoutVST(audioMaster);
}

VstInt32 ShoutVST::getVendorVersion() { return SHOUTVST_VERSION_INT; }

VstPlugCategory ShoutVST::getPlugCategory() { return kPlugCategEffect; }

ShoutVST::ShoutVST(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, 1, 0) {
  setNumInputs(2);
  setNumOutputs(2);
  setUniqueID(CCONST('b', 'q', '9', 'e'));
  canProcessReplacing();
  canDoubleReplacing(false);
  noTail(false);
  encSelected = nullptr;
  pEditor = std::make_shared<ShoutVSTEditor>(this);
  setEditor(pEditor.get());
}

ShoutVST::~ShoutVST() {
  disconnect();
  setEditor(nullptr);
}

bool ShoutVST::IsConnected() {
  guard lock(mtx_);
  return (encSelected != nullptr);
}

int ShoutVST::GetBitrate() { return std::stoi(pEditor->GetBitrate()); }

int ShoutVST::GetTargetSampleRate() {
  return std::stoi(pEditor->GetTargetSampleRate());
}

void ShoutVST::processReplacing(float** inputs, float** outputs,
                                VstInt32 sampleFrames) {
  if (!inputs || !outputs || sampleFrames <= 0) {
    return;
  }
  float* in1 = inputs[0];
  float* in2 = inputs[1];
  float* out1 = outputs[0];
  float* out2 = outputs[1];
  for (VstInt32 i(0); i < sampleFrames; ++i) {
    out1[i] = in1[i];
    out2[i] = in2[i];
  }
  if (encSelected) {
    if (!encSelected->Process(inputs, sampleFrames)) {
      disconnect();
    }
  }
}

void ShoutVST::connect() {
  guard lock(mtx_);
  if (encSelected) {
    return;
  }
  encTmp = nullptr;
  if (pEditor->getEncodingFormat() == "mp3") {
    encTmp = std::make_shared<ShoutVSTEncoderMP3>(libShoutWrapper);
  }
  if (pEditor->getEncodingFormat() == "ogg") {
    encTmp = std::make_shared<ShoutVSTEncoderOGG>(libShoutWrapper);
  }
  if (!encTmp ||
      !encTmp->Initialize(GetBitrate(), (const int)updateSampleRate(),
                          GetTargetSampleRate()) ||
      !libShoutWrapper.InitializeICECasting(
          pEditor->getHostName(), pEditor->getProtocol(), pEditor->getPort(),
          pEditor->getStreamName(), pEditor->getStreamURL(),
          pEditor->getStreamGenre(), pEditor->getStreamDescription(),
          std::to_string(encTmp->getBitrate()), pEditor->GetTargetSampleRate(),
          pEditor->getStreamArtist(), pEditor->getStreamTitle(),
          pEditor->getUserName(), pEditor->getPassword(),
          pEditor->getMountPoint(), pEditor->getEncodingFormat())) {
    disconnect();
  }
  std::thread t([this]() {
    if (libShoutWrapper.waitForConnect()) {
      encSelected = encTmp;
      pEditor->DisableAccordingly();
    } else {
      disconnect();
    }
  });
  t.detach();
}

void ShoutVST::disconnect() {
  guard lock(mtx_);
  encSelected = nullptr;
  encTmp = nullptr;
  libShoutWrapper.StopICECasting();
  pEditor->DisableAccordingly();
}

void ShoutVST::UpdateMetadata(const string& metadata) {
  libShoutWrapper.UpdateMetadata(metadata.c_str());
}

bool ShoutVST::getEffectName(char* name) {
  if (!name) {
    return false;
  }
  vst_strncpy(name, "ShoutVST", kVstMaxEffectNameLen);
  return true;
}

bool ShoutVST::getVendorString(char* text) {
  if (!text) {
    return false;
  }
  vst_strncpy(text, "github.com/R-Tur/ShoutVST", kVstMaxVendorStrLen);
  return true;
}

bool ShoutVST::getProductString(char* text) {
  if (!text) {
    return false;
  }
  vst_strncpy(text, "ShoutVST", kVstMaxProductStrLen);
  return true;
}
