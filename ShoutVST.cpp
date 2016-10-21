#include "ShoutVST.h"
#include <thread>
#include "version.h"

AudioEffect* createEffectInstance(audioMasterCallback audioMaster) {
	return new ShoutVST(audioMaster);
}

VstInt32 ShoutVST::getVendorVersion() { return SHOUTVST_VERSION_INT; }

VstPlugCategory ShoutVST::getPlugCategory() { return kPlugCategEffect; }

ShoutVST::ShoutVST(audioMasterCallback audioMaster)
	: AudioEffectX(audioMaster, 1, 0),
	bStreamConnected(false),
	bConnecting(false) {
	setNumInputs(2);
	setNumOutputs(2);
	setUniqueID(CCONST('b', 'q', '9', 'e'));
	canProcessReplacing();
	canDoubleReplacing(false);
	noTail(false);
	encMP3 = new ShoutVSTEncoderMP3(libShoutWrapper);
	encOGG = new ShoutVSTEncoderOGG(libShoutWrapper);
	encSelected = encMP3;
	pEditor = new ShoutVSTEditor(this);
	setEditor(pEditor);
}

ShoutVST::~ShoutVST() {
	disconnect();
	pEditor->close();
	setEditor(nullptr);
	delete encMP3;
	delete encOGG;
	delete pEditor;
}

bool ShoutVST::IsConnected() { return bStreamConnected; }

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
	if (bStreamConnected) {
		if (!encSelected->Process(inputs, sampleFrames)) {
			disconnect();
		}
	}
}

void ShoutVST::connect() {
	if (bStreamConnected) {
		pEditor->DisableAccordingly();
		return;
	}
	std::thread t([this]() {
		if (bConnecting) {
			return;
		}
		bConnecting = true;
		const bool icecastingInitialized = libShoutWrapper.InitializeICECasting(
			pEditor->getHostName(), pEditor->getProtocol(), pEditor->getPort(),
			pEditor->getStreamName(), pEditor->getStreamURL(),
			pEditor->getStreamGenre(), pEditor->getStreamDescription(),
			pEditor->GetBitrate(), pEditor->GetTargetSampleRate(),
			pEditor->getStreamArtist(), pEditor->getStreamTitle(),
			pEditor->getUserName(), pEditor->getPassword(),
			pEditor->getMountPoint(), pEditor->getEncodingFormat());

		if (!icecastingInitialized) {
			libShoutWrapper.StopICECasting();
			pEditor->DisableAccordingly();
			bConnecting = false;
			return;
		}

		const bool connectedToServer = libShoutWrapper.waitForConnect();
		if (!connectedToServer) {
			libShoutWrapper.StopICECasting();
			pEditor->DisableAccordingly();
			bConnecting = false;
			return;
		}

		if (pEditor->getEncodingFormat() == "mp3") {
			encSelected = encMP3;
		}

		if (pEditor->getEncodingFormat() == "ogg") {
			encSelected = encOGG;
		}

		if (!encSelected->Initialize(GetBitrate(), (const int)updateSampleRate(),
			GetTargetSampleRate())) {
			libShoutWrapper.StopICECasting();
			pEditor->DisableAccordingly();
			bConnecting = false;
			return;
		}

		bStreamConnected = true;

		pEditor->DisableAccordingly();
		bConnecting = false;
	});
	t.detach();
}

void ShoutVST::disconnect() {
	encSelected->Close();
	libShoutWrapper.StopICECasting();
	bStreamConnected = false;
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
