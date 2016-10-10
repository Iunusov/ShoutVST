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

 private:
  ShoutVSTEditorFL* shoutVSTEditorFL = nullptr;
  static recursive_mutex mtx_;
  ShoutVST* pVST = nullptr;
};
