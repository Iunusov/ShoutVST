#include "ShoutVST.h"
#include <shout/shout.h>
#include <string>
#include "ShoutVSTEditor.h"

#ifdef __linux__
#include <X11/Xlib.h>
#endif

recursive_mutex ShoutVSTEditor::mtx_;
typedef std::lock_guard<std::recursive_mutex> guard;

ShoutVSTEditor::ShoutVSTEditor(AudioEffect* effect) : AEffEditor(effect) {
  pVST = reinterpret_cast<ShoutVST*>(effect);
  systemWindow = nullptr;
  shoutVSTEditorFL = new ShoutVSTEditorFL();
  shoutVSTEditorFL->connectCallback = std::bind(callbackConnect, this);
  shoutVSTEditorFL->disconnectCallback = std::bind(callbackDisconnect, this);
  shoutVSTEditorFL->metadataCallback = std::bind(callbackMetadata, this);
}

ShoutVSTEditor::~ShoutVSTEditor() {
  guard lock(mtx_);
  shoutVSTEditorFL->hide();
  systemWindow = nullptr;
  delete shoutVSTEditorFL;
}

void ShoutVSTEditor::callbackConnect(ShoutVSTEditor* p) {
  guard lock(mtx_);
  p->shoutVSTEditorFL->setDisable(true);
  p->pVST->connect();
}

void ShoutVSTEditor::callbackDisconnect(ShoutVSTEditor* p) {
  p->pVST->disconnect();
}

void ShoutVSTEditor::callbackMetadata(ShoutVSTEditor* p) {
  guard lock(mtx_);
  const string metadata = p->getStreamMetaData();
  p->pVST->UpdateMetadata(metadata.c_str());
}

void ShoutVSTEditor::idle() { Fl::wait(0); }

string ShoutVSTEditor::getStreamMetaData() const {
  return shoutVSTEditorFL->getStreamMetaData();
}

string ShoutVSTEditor::getHostName() const {
  return shoutVSTEditorFL->getHostName();
}

string ShoutVSTEditor::getStreamName() const {
  return shoutVSTEditorFL->getStreamName();
}

string ShoutVSTEditor::getStreamURL() const {
  return shoutVSTEditorFL->getStreamURL();
}

string ShoutVSTEditor::getStreamGenre() const {
  return shoutVSTEditorFL->getStreamGenre();
}

string ShoutVSTEditor::getStreamDescription() const {
  return shoutVSTEditorFL->getStreamDescription();
}

string ShoutVSTEditor::getStreamArtist() const {
  return shoutVSTEditorFL->getStreamArtist();
}

string ShoutVSTEditor::getStreamTitle() const {
  return shoutVSTEditorFL->getStreamTitle();
}

unsigned short ShoutVSTEditor::getPort() const {
  return std::stoi(shoutVSTEditorFL->getPort());
}

string ShoutVSTEditor::getUserName() const {
  return shoutVSTEditorFL->getUserName();
}

string ShoutVSTEditor::getPassword() const {
  return shoutVSTEditorFL->getPassword();
}

string ShoutVSTEditor::getMountPoint() const {
  return shoutVSTEditorFL->getMountPoint();
}

string ShoutVSTEditor::getEncodingFormat() const {
  return shoutVSTEditorFL->getEncodingFormat();
}

string ShoutVSTEditor::getProtocol() const {
  return shoutVSTEditorFL->getProtocol();
}

bool ShoutVSTEditor::open(void* parentWindow) {
  guard lock(mtx_);
  systemWindow = parentWindow;
  Fl::scheme("Plastic");
  shoutVSTEditorFL->show();
#ifdef _WIN32
  HWND hWnd = (HWND)fl_xid(shoutVSTEditorFL->fl_window);
  // SetWindowLong(hWnd, GWL_STYLE, (GetWindowLong(hWnd, GWL_STYLE) & ~WS_POPUP)
  // | WS_CHILD);
  SetParent(hWnd, (HWND)parentWindow);
#endif

#ifdef __linux__
  Window xid = fl_xid(shoutVSTEditorFL->fl_window);
  Display* display = XOpenDisplay(NULL);
  if (display) {
    Window parent = (Window)parentWindow;
    XReparentWindow(display, xid, parent, 0, 0);
    XSync(display, False);
    XCloseDisplay(display);
  }
#endif

  shoutVSTEditorFL->fl_window->position(0, 0);
  DisableAccordingly();
  return true;
}

void ShoutVSTEditor::close() {
  guard lock(mtx_);
  shoutVSTEditorFL->hide();
}

bool ShoutVSTEditor::getRect(ERect** erect) {
  static ERect r = {};
  r.right = shoutVSTEditorFL->fl_window->decorated_w();
  r.bottom = shoutVSTEditorFL->fl_window->decorated_h();
  *erect = &r;
  return true;
}

string ShoutVSTEditor::GetBitrate() { return shoutVSTEditorFL->getBitRate(); }

string ShoutVSTEditor::GetTargetSampleRate() {
  return shoutVSTEditorFL->getSampleRate();
}

void ShoutVSTEditor::DisableAccordingly() {
  guard lock(mtx_);
  shoutVSTEditorFL->setDisable(pVST->IsConnected());
}
