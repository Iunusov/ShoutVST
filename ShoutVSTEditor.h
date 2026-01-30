#pragma once
#include <aeffeditor.h>
#include <audioeffectx.h>
#include <string>

#include "FLUID/ShoutVSTEditorFL.h"

#include <mutex>

using std::recursive_mutex;

class ShoutVST;

class ShoutVSTEditor : public AEffEditor {
 public:
  explicit ShoutVSTEditor(AudioEffect* effect);
  virtual ~ShoutVSTEditor();

  static void callbackConnect(ShoutVSTEditor* p);
  static void callbackDisconnect(ShoutVSTEditor* p);
  static void callbackMetadata(ShoutVSTEditor* p);

  virtual bool open(void* ptr) override;
  virtual void close() override;
  virtual bool getRect(ERect** erect) override;
  virtual void idle() override;
  void DisableAccordingly();
  string GetBitrate();
  string GetTargetSampleRate();
  string getHostName() const;
  unsigned short getPort() const;
  string getStreamName() const;
  string getStreamURL() const;
  string getStreamGenre() const;
  string getStreamDescription() const;
  string getStreamArtist() const;
  string getStreamTitle() const;
  string getUserName() const;
  string getPassword() const;
  string getMountPoint() const;
  string getEncodingFormat() const;
  string getStreamMetaData() const;
  string getProtocol() const;
  void setHostName(const string& value);
  void setPort(const string& value);
  void setUserName(const string& value);
  void setPassword(const string& value);
  void setMountPoint(const string& value);
  void setProtocol(const string& value);
  void setBitrate(const string& value);
  void setTargetSampleRate(const string& value);
  void setEncodingFormat(const string& value);
  void setStreamName(const string& value);
  void setStreamURL(const string& value);
  void setStreamGenre(const string& value);
  void setStreamArtist(const string& value);
  void setStreamTitle(const string& value);
  void setStreamDescription(const string& value);
  void setStreamMetaData(const string& value);

 private:
  ShoutVSTEditorFL* shoutVSTEditorFL = nullptr;
  static recursive_mutex mtx_;
  ShoutVST* pVST = nullptr;
};
