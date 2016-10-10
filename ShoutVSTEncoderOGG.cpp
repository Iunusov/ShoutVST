#include "ShoutVSTEncoderOGG.h"

ShoutVSTEncoderOGG::ShoutVSTEncoderOGG(LibShoutWrapper& ls)
    : ShoutVSTEncoder(ls) {}

ShoutVSTEncoderOGG::~ShoutVSTEncoderOGG() { Close(); }

bool ShoutVSTEncoderOGG::Initialize(const int bitrate, const int samplerate,
                                    const int target_samplerate) {
  guard lock(mtx_);
  if (bInitialized) {
    return false;
  }
  int ret = 0;
  vorbis_info_init(&vi);
  long sample = (long)samplerate;
  long br = bitrate * 1000;
  // ret = vorbis_encode_init_vbr(&vi,2,sample,10 / 10.0f);
  ret = vorbis_encode_init(&vi, 2, sample, br, br, br);
  if (ret) {
    vorbis_info_clear(&vi);
    return false;
  }

  vorbis_comment_init(&vc);
  vorbis_comment_add_tag(&vc, "ENCODER", "ShoutVST");
  vorbis_analysis_init(&vd, &vi);
  vorbis_block_init(&vd, &vb);
  ogg_stream_init(&os, rand());  // totally random number

  {
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&vd, &vc, &header, &header_comm, &header_code);
    ogg_stream_packetin(&os, &header); /* automatically placed in its own
         page */
    ogg_stream_packetin(&os, &header_comm);
    ogg_stream_packetin(&os, &header_code);

    bInitialized = true;

    while (true) {
      int result = ogg_stream_flush(&os, &og);
      if (result <= 0) break;
      if (!SendOGGPageToICE(&og)) {
        Close();
        return false;
      }
    }
  }

  return true;
}

bool ShoutVSTEncoderOGG::Close() {
  guard lock(mtx_);
  if (!bInitialized) {
    return true;
  }
  ogg_stream_clear(&os);
  vorbis_block_clear(&vb);
  vorbis_dsp_clear(&vd);
  vorbis_comment_clear(&vc);
  vorbis_info_clear(&vi);
  bInitialized = false;
  return true;
}

bool ShoutVSTEncoderOGG::Process(float** inputs, VstInt32 sampleFrames) {
  guard lock(mtx_);
  if (!bInitialized) return false;

  float** buffer = vorbis_analysis_buffer(&vd, sampleFrames);

  /* uninterleave samples */
  for (int i = 0; i < sampleFrames; i++) {
    buffer[0][i] = inputs[0][i];
    buffer[1][i] = inputs[1][i];
  }

  /* tell the library how much we actually submitted */
  vorbis_analysis_wrote(&vd, sampleFrames);

  /* vorbis does some data preanalysis, then divvies up blocks for
     more involved (potentially parallel) processing.  Get a single
     block for encoding now */
  int eos = 0;

  while (vorbis_analysis_blockout(&vd, &vb) == 1) {
    /* analysis, assume we want to use bitrate management */
    vorbis_analysis(&vb, NULL);
    vorbis_bitrate_addblock(&vb);

    while (vorbis_bitrate_flushpacket(&vd, &op) == 1) {
      /* weld the packet into the bitstream */
      ogg_stream_packetin(&os, &op);

      /* write out pages (if any) */
      while (!eos) {
        int result = ogg_stream_pageout(&os, &og);
        if (result <= 0) break;
        if (!SendOGGPageToICE(&og)) return false;

        /* this could be set above, but for illustrative purposes, I do
                 it here (to show that vorbis does know where the stream
   ends) */

        if (ogg_page_eos(&og)) eos = 1;
      }
    }
  }

  return true;
}

bool ShoutVSTEncoderOGG::SendOGGPageToICE(ogg_page* og) {
  guard lock(mtx_);
  if (!bInitialized) return false;
  if (!libshout.SendDataToICE(og->header, og->header_len)) return false;
  if (!libshout.SendDataToICE(og->body, og->body_len)) return false;
  return true;
}
