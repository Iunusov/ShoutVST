#pragma once

#include "ShoutVSTEncoder.h"
#include <FLAC++/encoder.h>
#include <cmath>

class ShoutVSTEncoderFLAC : public ShoutVSTEncoder, FLAC::Encoder::Stream {

public:

  ShoutVSTEncoderFLAC(LibShoutWrapper& ls);
  ~ShoutVSTEncoderFLAC();
  bool Initialize(const int bitrate, const int samplerate, const int target_samplerate);
  bool Close();
  bool Process(float** inputs, VstInt32 sampleFrames);
  virtual int getBitrate();

private:

  const int CHUNK_SIZE = 2048;
  const int BITS_PER_SAMPLE = 24;
  const int MAX_BITSIZE_VAL = std::pow(2, BITS_PER_SAMPLE-1);//one bit less for actual value since it is signed
  int pSamplerate = 44100;
  int pChannel = 2;

  int pWAVBufferSize = 0;
  int* pWAVBuffer = nullptr;
  int pWAVBufferProcessing = 0;

  int pFlacBufferSize = 0;
  unsigned char* pFlacBuffer = nullptr;
  unsigned char* pFlacBufferProcessing = nullptr;

  unsigned char pFlacHeader[1024] = {0};
  unsigned char* pFlacHeaderProcessing = nullptr;
  int pFlacHeaderWritten = 0;
  size_t pFlacHeaderSize = 0;

  bool initOggFlacEncoder();
  void ditherSilence(int sampleFrames);
  int encodeFlacStream();
  bool setFlacSettings();
  FLAC__StreamEncoderWriteStatus write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame);

};
