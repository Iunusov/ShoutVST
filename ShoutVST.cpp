#include "ShoutVST.h"
#include <thread>
#include <sstream>
#include <map>
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
  setUniqueID(CCONST('a', 'h', 'n', 'e'));
  canProcessReplacing();
  canDoubleReplacing(false);
  noTail(false);
  programsAreChunks(true);
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
  guard lock(mtx_);
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
	  guard lock(mtx_);
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
  vst_strncpy(text, "github.com/Iunusov/ShoutVST", kVstMaxVendorStrLen);
  return true;
}

bool ShoutVST::getProductString(char* text) {
  if (!text) {
    return false;
  }
  vst_strncpy(text, "ShoutVST", kVstMaxProductStrLen);
  return true;
}

// Helper function to escape newlines and special characters
static std::string escapeValue(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  for (char c : str) {
    if (c == '\n') {
      result += "\\n";
    } else if (c == '\r') {
      result += "\\r";
    } else if (c == '\\') {
      result += "\\\\";
    } else {
      result += c;
    }
  }
  return result;
}

// Helper function to unescape newlines and special characters
static std::string unescapeValue(const std::string& str) {
  std::string result;
  result.reserve(str.size());
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '\\' && i + 1 < str.size()) {
      char next = str[i + 1];
      if (next == 'n') {
        result += '\n';
        ++i;
      } else if (next == 'r') {
        result += '\r';
        ++i;
      } else if (next == '\\') {
        result += '\\';
        ++i;
      } else {
        result += str[i];
      }
    } else {
      result += str[i];
    }
  }
  return result;
}

// Helper function to parse key=value format
static std::map<std::string, std::string> parseKeyValue(const std::string& str) {
  std::map<std::string, std::string> result;
  std::istringstream stream(str);
  std::string line;

  while (std::getline(stream, line)) {
    size_t pos = line.find('=');
    if (pos != std::string::npos) {
      std::string key = line.substr(0, pos);
      std::string value = line.substr(pos + 1);
      result[key] = unescapeValue(value);
    }
  }

  return result;
}

std::string ShoutVST::serializeState() {
  std::ostringstream ss;

  if (!pEditor) {
    return "";
  }

  ss << "connection.hostname=" << escapeValue(pEditor->getHostName()) << "\n";
  ss << "connection.port=" << escapeValue(std::to_string(pEditor->getPort())) << "\n";
  ss << "connection.username=" << escapeValue(pEditor->getUserName()) << "\n";
  ss << "connection.password=" << escapeValue(pEditor->getPassword()) << "\n";
  ss << "connection.mountpoint=" << escapeValue(pEditor->getMountPoint()) << "\n";
  ss << "connection.protocol=" << escapeValue(pEditor->getProtocol()) << "\n";
  ss << "encoding.bitrate=" << escapeValue(pEditor->GetBitrate()) << "\n";
  ss << "encoding.samplerate=" << escapeValue(pEditor->GetTargetSampleRate()) << "\n";
  ss << "encoding.format=" << escapeValue(pEditor->getEncodingFormat()) << "\n";
  ss << "stream.name=" << escapeValue(pEditor->getStreamName()) << "\n";
  ss << "stream.url=" << escapeValue(pEditor->getStreamURL()) << "\n";
  ss << "stream.genre=" << escapeValue(pEditor->getStreamGenre()) << "\n";
  ss << "stream.artist=" << escapeValue(pEditor->getStreamArtist()) << "\n";
  ss << "stream.title=" << escapeValue(pEditor->getStreamTitle()) << "\n";
  ss << "stream.description=" << escapeValue(pEditor->getStreamDescription()) << "\n";
  ss << "metadata=" << escapeValue(pEditor->getStreamMetaData()) << "\n";

  return ss.str();
}

void ShoutVST::deserializeState(const std::string& state) {
  if (!pEditor || state.empty()) {
    return;
  }

  std::map<std::string, std::string> params = parseKeyValue(state);

  if (params.count("connection.hostname")) {
    pEditor->setHostName(params["connection.hostname"]);
  }
  if (params.count("connection.port")) {
    pEditor->setPort(params["connection.port"]);
  }
  if (params.count("connection.username")) {
    pEditor->setUserName(params["connection.username"]);
  }
  if (params.count("connection.password")) {
    pEditor->setPassword(params["connection.password"]);
  }
  if (params.count("connection.mountpoint")) {
    pEditor->setMountPoint(params["connection.mountpoint"]);
  }
  if (params.count("connection.protocol")) {
    pEditor->setProtocol(params["connection.protocol"]);
  }
  if (params.count("encoding.bitrate")) {
    pEditor->setBitrate(params["encoding.bitrate"]);
  }
  if (params.count("encoding.samplerate")) {
    pEditor->setTargetSampleRate(params["encoding.samplerate"]);
  }
  if (params.count("encoding.format")) {
    pEditor->setEncodingFormat(params["encoding.format"]);
  }
  if (params.count("stream.name")) {
    pEditor->setStreamName(params["stream.name"]);
  }
  if (params.count("stream.url")) {
    pEditor->setStreamURL(params["stream.url"]);
  }
  if (params.count("stream.genre")) {
    pEditor->setStreamGenre(params["stream.genre"]);
  }
  if (params.count("stream.artist")) {
    pEditor->setStreamArtist(params["stream.artist"]);
  }
  if (params.count("stream.title")) {
    pEditor->setStreamTitle(params["stream.title"]);
  }
  if (params.count("stream.description")) {
    pEditor->setStreamDescription(params["stream.description"]);
  }
  if (params.count("metadata")) {
    pEditor->setStreamMetaData(params["metadata"]);
  }
}

VstInt32 ShoutVST::getChunk(void** data, bool isPreset) {
  guard lock(mtx_);
  savedState_ = serializeState();
  *data = (void*)savedState_.c_str();
  return (VstInt32)savedState_.size();
}

VstInt32 ShoutVST::setChunk(void* data, VstInt32 byteSize, bool isPreset) {
  guard lock(mtx_);
  if (!data || byteSize <= 0) {
    return 0;
  }

  std::string state((const char*)data, byteSize);
  deserializeState(state);

  return 0;
}
