#pragma once
#include <lame.h>
#include "ShoutVSTEncoder.h"

class ShoutVSTEncoderMP3 : public ShoutVSTEncoder {
 public:
  ShoutVSTEncoderMP3(LibShoutWrapper& ls);
  ~ShoutVSTEncoderMP3();
  bool Initialize(const int bitrate, const int samplerate,
                  const int target_samplerate);
  bool Close();
  bool Process(float** inputs, VstInt32 sampleFrames);
  virtual int getBitrate();

 private:
  lame_global_flags* gfp = NULL;
  int k = 0;
  int wavBufferSize = 0;
  int mp3BufferSize = 0;
  unsigned char* pMP3Buffer = nullptr;
  float* pWAVBuffer = nullptr;
};
