#pragma once

#include <aeffect.h>
#include "LibShoutWrapper.h"
class ShoutVST;

#include <mutex>
#include <string>
using std::recursive_mutex;
using std::string;

class ShoutVSTEncoder {
 public:
  ShoutVSTEncoder(LibShoutWrapper &libshout);
  virtual bool Initialize(const int bitrate, const int samplerate,
                          const int target_samplerate) = 0;
  virtual bool Close() = 0;
  virtual bool Process(float **inputs, VstInt32 sampleFrames) = 0;
  virtual int getBitrate() = 0;
  virtual ~ShoutVSTEncoder();

 protected:
  typedef std::lock_guard<std::recursive_mutex> guard;
  recursive_mutex mtx_;
  LibShoutWrapper &libshout;
  bool bInitialized = false;
};
