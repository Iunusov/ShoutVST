#include "ShoutVSTEncoderMP3.h"
#include <algorithm>
#include <string>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

ShoutVSTEncoderMP3::ShoutVSTEncoderMP3(LibShoutWrapper& ls)
	: ShoutVSTEncoder(ls) {}

ShoutVSTEncoderMP3::~ShoutVSTEncoderMP3() { Close(); }

bool ShoutVSTEncoderMP3::Initialize(const int bitrate, const int samplerate,
	const int target_samplerate) {
	guard lock(mtx_);
	if (bInitialized) {
		return false;
	}
	bInitialized = false;
	k = 0;
	gfp = lame_init();
	if (!gfp) {
		return false;
	}
	lame_set_in_samplerate(gfp, samplerate);
	lame_set_preset(gfp, bitrate);
	lame_set_bWriteVbrTag(gfp, 0);
	lame_set_VBR(gfp, vbr_off);
	lame_set_mode(gfp, JOINT_STEREO);
	lame_set_num_channels(gfp, 2);
	lame_set_brate(gfp, bitrate);
	lame_set_original(gfp, 0);
	lame_set_error_protection(gfp, 1);
	lame_set_extension(gfp, 0);
	lame_set_strict_ISO(gfp, 0);
	lame_set_out_samplerate(gfp, target_samplerate);
	if (lame_init_params(gfp) != 0) {
		lame_close(gfp);
		gfp = nullptr;
		return false;
	}

	// LAME encoding call will accept any number of samples.
	if (0 == lame_get_version(gfp)) {
		// For MPEG-II, only 576 samples per frame per channel
		wavBufferSize = 576 * lame_get_num_channels(gfp);
	}
	else {
		// For MPEG-I, 1152 samples per frame per channel
		wavBufferSize = 1152 * lame_get_num_channels(gfp);
	}
	// Set MP3 buffer size, conservative estimate
	mp3BufferSize = (int)(1.25 * (wavBufferSize / lame_get_num_channels(gfp)) + 7200);
	pMP3Buffer = new unsigned char[mp3BufferSize];
	pWAVBuffer = new short int[wavBufferSize];
	bInitialized = true;
	return true;
}

bool ShoutVSTEncoderMP3::Close() {
	guard lock(mtx_);
	if (!bInitialized) {
		return true;
	}
	bInitialized = false;
	const int nOutputSamples = lame_encode_flush(gfp, pMP3Buffer, 0);
	if (nOutputSamples > 0) {
		libshout.SendDataToICE(pMP3Buffer, nOutputSamples);
	}
	lame_close(gfp);
	delete[] pWAVBuffer;
	delete[] pMP3Buffer;
	return true;
}

bool ShoutVSTEncoderMP3::Process(float** inputs, VstInt32 sampleFrames) {
	guard lock(mtx_);
	if (!bInitialized) return false;
	for (VstInt32 i(0); i < sampleFrames; i++) {
		pWAVBuffer[k++] = (short)(std::min(1.0f, std::max(-1.0f, inputs[0][i])) * 32767.0f);
		pWAVBuffer[k++] = (short)(std::min(1.0f, std::max(-1.0f, inputs[1][i])) * 32767.0f);
		if (k >= wavBufferSize) {
			k = 0;
			const int len = lame_encode_buffer_interleaved(
				gfp, pWAVBuffer, wavBufferSize / lame_get_num_channels(gfp),
				pMP3Buffer, 0);
			if (len < 0) {
				return false;
			}
			if (!libshout.SendDataToICE(pMP3Buffer, len)) {
				return false;
			}
		}
	}
	return true;
}
