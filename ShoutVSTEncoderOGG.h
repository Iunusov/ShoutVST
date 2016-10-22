#pragma once
#include <vorbis/vorbisenc.h>
#include "ShoutVSTEncoder.h"

class ShoutVSTEncoderOGG : public ShoutVSTEncoder {
 public:
  ShoutVSTEncoderOGG(LibShoutWrapper& ls);
  ~ShoutVSTEncoderOGG();
  bool Initialize(const int bitrate, const int samplerate,
                  const int target_samplerate);
  bool Close();
  bool Process(float** inputs, VstInt32 sampleFrames);
  virtual int getBitrate();

 private:
  int bitrate_ = 0;
  bool SendOGGPageToICE(ogg_page* og);
  ogg_stream_state os; /* take physical pages, weld into a logical
     stream of packets */
  ogg_page og;         /* one Ogg bitstream page.  Vorbis packets are inside */
  ogg_packet op;       /* one raw packet of data for decode */

  vorbis_info vi;    /* struct that stores all the static vorbis bitstream
           settings */
  vorbis_comment vc; /* struct that stores all the user comments */

  vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
  vorbis_block vb;     /* local working space for packet->PCM decode */
};
