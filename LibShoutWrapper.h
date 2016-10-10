#pragma once

#include <shout/shout.h>
#include <mutex>
#include <string>
#include <atomic>

using std::string;
using std::recursive_mutex;
using std::lock_guard;

class LibShoutWrapper {
 private:
  typedef std::lock_guard<std::recursive_mutex> guard;
  recursive_mutex mtx;
  shout_t *pShout = nullptr;
  std::atomic<bool> isConnected;
  bool abort = false;

 public:
  LibShoutWrapper();
  ~LibShoutWrapper();
  bool InitializeICECasting(
      const string &hostname, const string &protocol, unsigned short port,
      const string &streamname, const string &streamurl,
      const string &streamgenre, const string &streamdescription,
      const string &bitrate, const string &targetsamplerate,
      const string &artist, const string &title, const string &username,
      const string &password, const string &mountpoint, const string &format);

  void StopICECasting();

  bool waitForConnect();

  bool SendDataToICE(unsigned char *pData, size_t nLen);

  void UpdateMetadata(const char *sz);
};
