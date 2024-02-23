/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "examples/peerconnection/localvideo/linux/fake_wnd.h"



#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cstdint>
#include <map>
#include <utility>

#include "api/video/i420_buffer.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/video_rotation.h"
#include "api/video/video_source_interface.h"
#include "rtc_base/checks.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv/convert.h"
#include "third_party/libyuv/include/libyuv/convert_from.h"
// sleep
#include <unistd.h>

extern bool is_sender;
extern std::string recon_filename;

FakeMainWnd::FakeMainWnd(const char* server,
                       int port,
                       bool autoconnect,
                       bool autocall)
    : callback_(NULL),
      server_(server),
      autoconnect_(autoconnect),
      autocall_(autocall) {
  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%i", port);
  port_ = buffer;
}

FakeMainWnd::~FakeMainWnd() {
  RTC_DCHECK(!IsWindow());
}

void FakeMainWnd::RegisterObserver(MainWndCallback* callback) {
  callback_ = callback;
}


bool FakeMainWnd::IsWindow() {
  return true;
}

void FakeMainWnd::MessageBox(const char* caption,
                            const char* text,
                            bool is_error) {
  RTC_LOG(LS_INFO) << __FUNCTION__ << ": " << text;
}

MainWindow::UI FakeMainWnd::current_ui() {
  if (status == 1)
    return CONNECT_TO_SERVER;

  if (status == 2)
    return LIST_PEERS;

  return STREAMING;
}

void FakeMainWnd::StartLocalRenderer(webrtc::VideoTrackInterface* local_video) {
  local_renderer_.reset(new VideoRenderer(this, local_video));
}

void FakeMainWnd::StopLocalRenderer() {
  local_renderer_.reset();
}

void FakeMainWnd::StartRemoteRenderer(
    webrtc::VideoTrackInterface* remote_video) {
  remote_renderer_.reset(new VideoRenderer(this, remote_video));
}

void FakeMainWnd::StopRemoteRenderer() {
  remote_renderer_.reset();
}


void FakeMainWnd::QueueUIThreadCallback(int msg_id, void* data) {
  RTC_LOG(LS_INFO) << __FUNCTION__;
  callback_->UIThreadCallback(msg_id, data);
}

bool FakeMainWnd::Create() {
  
  SwitchToConnectUI();
  

  return true;
}

bool FakeMainWnd::Destroy() {

  RTC_LOG(LS_INFO) << __FUNCTION__;

  return true;
}

void FakeMainWnd::SwitchToConnectUI() {
  status = 1;
  RTC_LOG(LS_INFO) << __FUNCTION__;
}

void FakeMainWnd::SwitchToPeerList(const Peers& peers) {
  RTC_LOG(LS_INFO) << __FUNCTION__ ;
  status = 2;
  // sl
  

  // typedef std::map<int, std::string> Peers;
  // loop and print all peers
  for (auto it = peers.begin(); it != peers.end(); ++it) {
    RTC_LOG(LS_INFO) << __FUNCTION__ << "peer id: " << it->first << " name: " << it->second;
  }
  // print peer size
  RTC_LOG(LS_INFO) << __FUNCTION__ << "peer size: " << peers.size();
  
  if(autocall_ && peers.size() > 0) {
    int id = peers.begin()->first;
    callback_->ConnectToPeer(id);
  }

}

void FakeMainWnd::SwitchToStreamingUI() {
    status = 3;
    RTC_LOG(LS_INFO) << __FUNCTION__;
}


void FakeMainWnd::AutoLogin() {
  // Make the connect button insensitive, so that it cannot be clicked more than
  // once.  Now that the connection includes auto-retry, it should not be
  // necessary to click it more than once.
  
  // the server and port have been specified when MainWnd is created.
  RTC_LOG(LS_INFO) << __FUNCTION__ << " server: " << server_ << " port: " << port_;

  int port = port_.length() ? atoi(port_.c_str()) : 0;
  callback_->StartLogin(server_, port);
}







FakeMainWnd::VideoRenderer::VideoRenderer(
    FakeMainWnd* main_wnd,
    webrtc::VideoTrackInterface* track_to_render)
    : width_(0),
      height_(0),
      main_wnd_(main_wnd),
      rendered_track_(track_to_render) {
  rendered_track_->AddOrUpdateSink(this, rtc::VideoSinkWants());
// Open file to write
  if (!is_sender) {
    file_ = fopen(recon_filename.c_str(), "wb");
  }
}

FakeMainWnd::VideoRenderer::~VideoRenderer() {

  fclose(file_);
  rendered_track_->RemoveSink(this);
}

void FakeMainWnd::VideoRenderer::SetSize(int width, int height) {

  if (width_ == width && height_ == height) {
    return;
  }

  width_ = width;
  height_ = height;
  image_.reset(new uint8_t[width * height * 4]);

}

void FakeMainWnd::VideoRenderer::OnFrame(const webrtc::VideoFrame& video_frame) {

  rtc::scoped_refptr<webrtc::I420BufferInterface> buffer(
      video_frame.video_frame_buffer()->ToI420());
  if (video_frame.rotation() != webrtc::kVideoRotation_0) {
    buffer = webrtc::I420Buffer::Rotate(*buffer, video_frame.rotation());
  }
  SetSize(buffer->width(), buffer->height());

  // TODO(bugs.webrtc.org/6857): This conversion is correct for little-endian
  // only. Cairo ARGB32 treats pixels as 32-bit values in *native* byte order,
  // with B in the least significant byte of the 32-bit value. Which on
  // little-endian means that memory layout is BGRA, with the B byte stored at
  // lowest address. Libyuv's ARGB format (surprisingly?) uses the same
  // little-endian format, with B in the first byte in memory, regardless of
  // native endianness.
if (!is_sender){
    RTC_LOG(LS_INFO) << __FUNCTION__ << " write to file";
    fwrite(buffer->DataY(), 1, buffer->width() * buffer->height(), file_);
    fwrite(buffer->DataU(), 1, buffer->width() * buffer->height() / 4, file_);
    fwrite(buffer->DataV(), 1, buffer->width() * buffer->height() / 4, file_);
  }
  
  libyuv::I420ToARGB(buffer->DataY(), buffer->StrideY(), buffer->DataU(),
                     buffer->StrideU(), buffer->DataV(), buffer->StrideV(),
                     image_.get(), width_ * 4, buffer->width(),
                     buffer->height());

}
