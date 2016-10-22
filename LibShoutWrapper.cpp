#include "LibShoutWrapper.h"

#include <chrono>
#include <thread>

LibShoutWrapper::LibShoutWrapper() : isConnected(false) {}

LibShoutWrapper::~LibShoutWrapper() { StopICECasting(); }

bool LibShoutWrapper::InitializeICECasting(
    const string &hostname, const string &protocol, unsigned short port,
    const string &streamname, const string &streamurl,
    const string &streamgenre, const string &streamdescription,
    const string &bitrate, const string &targetsamplerate, const string &artist,
    const string &title, const string &username, const string &password,
    const string &mountpoint, const string &format) {
  if (isConnected) {
    return false;
  }
  guard lock(mtx);
  if (pShout) {
    return false;
  }
  abort = false;
  shout_init();
  shout_t *tmp = nullptr;
  unsigned int shout_format = 0;
  int shout_proto = SHOUT_PROTOCOL_HTTP;
  int ret = -1;
  if (!(tmp = shout_new())) {
    goto err;
  }

  if (shout_set_host(tmp, hostname.c_str()) != SHOUTERR_SUCCESS) {
    goto err;
  }

  if (protocol == "http") {
    shout_proto = SHOUT_PROTOCOL_HTTP;
  } else if (protocol == "xaudiocast") {
    shout_proto = SHOUT_PROTOCOL_XAUDIOCAST;
  } else if (protocol == "icy") {
    shout_proto = SHOUT_PROTOCOL_ICY;
  } else if (protocol == "roaraudio") {
    shout_proto = SHOUT_PROTOCOL_ROARAUDIO;
  }

  if (shout_set_protocol(tmp, shout_proto) != SHOUTERR_SUCCESS) {
    goto err;
  }

  if (shout_set_port(tmp, port) != SHOUTERR_SUCCESS) {
    goto err;
  }

  shout_set_agent(tmp, "ShoutVST");

  if (!streamname.empty()) shout_set_name(tmp, streamname.c_str());

  if (!streamurl.empty()) shout_set_url(tmp, streamurl.c_str());

  if (!streamgenre.empty()) shout_set_genre(tmp, streamgenre.c_str());

  if (!streamdescription.empty())
    shout_set_description(tmp, streamdescription.c_str());

  shout_set_audio_info(tmp, SHOUT_AI_BITRATE, bitrate.c_str());

  shout_set_audio_info(tmp, SHOUT_AI_SAMPLERATE, targetsamplerate.c_str());

  if (!artist.empty()) shout_set_audio_info(tmp, "artist", artist.c_str());

  if (!title.empty()) shout_set_audio_info(tmp, "title", title.c_str());

  if (shout_set_user(tmp, username.c_str()) != SHOUTERR_SUCCESS) {
    goto err;
  }

  if (shout_set_password(tmp, password.c_str()) != SHOUTERR_SUCCESS) {
    goto err;
  }

  if (shout_set_mount(tmp, mountpoint.c_str()) != SHOUTERR_SUCCESS) {
    goto err;
  }

  shout_set_public(tmp, 1);

  if (format == "mp3") {
    shout_format = SHOUT_FORMAT_MP3;
  }

  if (format == "ogg") {
    shout_format = SHOUT_FORMAT_OGG;
  }

  if (shout_set_format(tmp, shout_format) != SHOUTERR_SUCCESS) {
    goto err;
  }

  if (shout_set_nonblocking(tmp, 1) != SHOUTERR_SUCCESS) {
    goto err;
  }
  ret = shout_open(tmp);
  if (ret != SHOUTERR_SUCCESS && ret != SHOUTERR_BUSY) {
    goto err;
  }

  if (pShout) {
    shout_close(tmp);
    goto err;
  }
  pShout = tmp;

  return true;

err:
  if (tmp) {
    shout_free(tmp);
    tmp = nullptr;
  }
  return false;
}

bool LibShoutWrapper::waitForConnect() {
  {
    guard lock(mtx);
    if (!pShout) {
      return false;
    }
  }
  int ret = SHOUTERR_BUSY;
  size_t counter = 0;
  while (ret == SHOUTERR_BUSY) {
    if (counter++ > 50) {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    guard lock(mtx);
    if (abort) {
      return false;
    }
    ret = shout_get_connected(pShout);
    if (SHOUTERR_CONNECTED == ret) {
      isConnected = true;
      return true;
    }
  }
  return false;
}

void LibShoutWrapper::StopICECasting() {
  guard lock(mtx);
  abort = true;
  isConnected = false;
  if (pShout) {
    shout_close(pShout);
    shout_free(pShout);
    pShout = nullptr;
  }
  shout_shutdown();
}

bool LibShoutWrapper::SendDataToICE(unsigned char *pData, size_t nLen) {
  if (!isConnected) {
    return false;
  }
  guard lock(mtx);
  if (!pShout) {
    return false;
  }
  int sent = shout_send(pShout, pData, nLen);
  if (sent < 0) {
    return false;
  }
  if (shout_queuelen(pShout) > 0) {
    shout_sync(pShout);
  }
  return true;
}

void LibShoutWrapper::UpdateMetadata(const char *sz) {
  if (!isConnected) return;
  guard lock(mtx);
  if (!pShout) return;
  shout_metadata_t *meta = shout_metadata_new();
  if (!meta) return;

  shout_metadata_add(meta, "song", sz);

  shout_set_metadata(pShout, meta);

  shout_metadata_free(meta);
}
