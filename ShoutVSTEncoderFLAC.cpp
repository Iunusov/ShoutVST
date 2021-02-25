#include "ShoutVSTEncoderFLAC.h"

ShoutVSTEncoderFLAC::ShoutVSTEncoderFLAC(LibShoutWrapper& ls) : ShoutVSTEncoder(ls) {}

ShoutVSTEncoderFLAC::~ShoutVSTEncoderFLAC() {
  Close();
}


bool ShoutVSTEncoderFLAC::Initialize(const int bitrate, const int samplerate, const int target_samplerate) {
  guard lock(mtx_);
  if (bInitialized) {
    return false;
  }
  bInitialized = false;

  srand((unsigned)time(0));

  pChannel = 2;
  pSamplerate = samplerate;

  pWAVBufferProcessing = 0;
  pWAVBufferSize = CHUNK_SIZE * pChannel;
  pFlacBufferSize = pWAVBufferSize * 30;

  pFlacBuffer = new unsigned char[pFlacBufferSize];
  pWAVBuffer = new int[pWAVBufferSize];

  initOggFlacEncoder();

  bInitialized = true;
  return true;
}


bool ShoutVSTEncoderFLAC::Close() {
  guard lock(mtx_);
  if (!bInitialized) {
    return true;
  }
  bInitialized = false;
  finish();
  delete[] pWAVBuffer;
  delete[] pFlacBuffer;
  //delete[] pFlacHeader; // I do not understand why this is not necessary
  return true;
}


bool ShoutVSTEncoderFLAC::initOggFlacEncoder()
{
  FLAC__StreamEncoderInitStatus init_status;

  pFlacHeaderProcessing = pFlacHeader; // I do not understand why this is not pFlacHeaderProcessing = &pFlacHeader
  pFlacHeaderWritten = 0;
  pFlacHeaderSize = 0;

  setFlacSettings();

  init_status = init_ogg();
  if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK)
  {
    fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
    return false;
  }

  return true;
}

bool ShoutVSTEncoderFLAC::setFlacSettings()
{
  bool ret = true;

  ret &= set_verify(false);
  ret &= set_compression_level(5);
  ret &= set_channels(pChannel);
  ret &= set_bits_per_sample(BITS_PER_SAMPLE);
  ret &= set_sample_rate(pSamplerate);
  ret &= set_total_samples_estimate(0);
  ret &= set_ogg_serial_number(rand());

  ret &= set_blocksize(4096);

  return ret;
}


bool ShoutVSTEncoderFLAC::Process(float** inputs, VstInt32 sampleFrames) {
  guard lock(mtx_);
  if (!bInitialized) return false;

  int minLoudnessValue = 0;
  int maxLoudnessValue = 0;
  int silenceThreshold = 7;


  for (VstInt32 i(0); i < sampleFrames; i++) {
    /*
      https://xiph.org/flac/api/group__flac__stream__encoder.html
      Note that for either process call, each sample in the buffers should be a signed integer,
      right-justified to the resolution set by FLAC__stream_encoder_set_bits_per_sample().
      For example, if the resolution is 16 bits per sample, the samples should all be in the range [-32768,32767].
    */

    //iterate over all channels
    for (int k = 0; k < pChannel; k++) {
        pWAVBuffer[pWAVBufferProcessing] = (int)(inputs[k][i] * MAX_BITSIZE_VAL);
        if (pWAVBuffer[pWAVBufferProcessing] >= MAX_BITSIZE_VAL) {
            pWAVBuffer[pWAVBufferProcessing] = MAX_BITSIZE_VAL - 1;
        }

        //variables for silence check
        if (pWAVBuffer[pWAVBufferProcessing] < minLoudnessValue) {
            minLoudnessValue = pWAVBuffer[pWAVBufferProcessing];
        }
        if (pWAVBuffer[pWAVBufferProcessing] > maxLoudnessValue) {
            maxLoudnessValue = pWAVBuffer[pWAVBufferProcessing];
        }

        pWAVBufferProcessing++;
    }


    if (i == sampleFrames - 1 || pWAVBufferProcessing >= pWAVBufferSize) {
        if (maxLoudnessValue - minLoudnessValue < silenceThreshold) {
            ditherSilence(sampleFrames);
        }
    }

    //only encode and stream once buffer reached the CHUNK_SIZE
    if (pWAVBufferProcessing >= pWAVBufferSize) {
      const int len = encodeFlacStream();
      pWAVBufferProcessing = 0;
      if (len < 0) {
        return false;
      }

      if (!libshout.SendDataToICE(pFlacBuffer, len)) {
        return false;
      }

    }
  }
  return true;
}

void ShoutVSTEncoderFLAC::ditherSilence(int sampleFrames)
{
  /*
    Flac is not sending data for silence, which leads to problems with many clients
    https://github.com/xiph/flac/issues/90
    Therefore we dither the silence
  */
  int startFrom = pWAVBufferProcessing - (sampleFrames * pChannel);
  int countTo = pWAVBufferProcessing - pChannel;
  if (countTo > pWAVBufferSize) {

  }
  for (int j = startFrom; j < countTo; j += pChannel)
  {
    if (j % 100 < 20) {//only dither a part of the current slice in order to still keep some silence to compress
      int randomNumber = (rand() % 14) - 6;//generate random values from -6 to +7 (3 bits) and apply to all channels
      for (int k = 0; k < pChannel; k++) {
        pWAVBuffer[j + k] = randomNumber;
      }
    }
  }

}

int ShoutVSTEncoderFLAC::encodeFlacStream()
{
  int bytes_written;

  pFlacBufferProcessing = pFlacBuffer;

  if (pFlacHeaderWritten == 0)
  {
    memcpy(pFlacBufferProcessing, pFlacHeader, pFlacHeaderSize);
    pFlacBufferProcessing += pFlacHeaderSize;
    pFlacHeaderWritten = 1;
  }

  process_interleaved(pWAVBuffer, CHUNK_SIZE);

  bytes_written = (int)(pFlacBufferProcessing - pFlacBuffer);

  return bytes_written;
}

FLAC__StreamEncoderWriteStatus ShoutVSTEncoderFLAC::write_callback(const FLAC__byte buffer[], size_t bytes, unsigned samples, unsigned current_frame)
{

  if (current_frame == 0)
  {
    // assemble header
    memcpy(pFlacHeaderProcessing, buffer, bytes);
    pFlacHeaderProcessing += bytes;
    pFlacHeaderSize += bytes;
    return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
  }

  if (pFlacBufferProcessing + bytes >= pFlacBuffer + pFlacBufferSize) {
    return FLAC__STREAM_ENCODER_WRITE_STATUS_FATAL_ERROR; //exceeded buffer length
  }

  if (pFlacBufferProcessing != NULL)
  {
    memcpy(pFlacBufferProcessing, buffer, bytes);
    pFlacBufferProcessing += bytes;
  }

  return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

int ShoutVSTEncoderFLAC::getBitrate() {
  guard lock(mtx_);
  return 0; // bitrate does not matter for FLAC
}