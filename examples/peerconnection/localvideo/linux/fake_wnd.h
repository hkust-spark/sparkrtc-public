/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_LOCALVIDEO_LINUX_FAKE_WND_H_
#define EXAMPLES_PEERCONNECTION_LOCALVIDEO_LINUX_FAKE_WND_H_


#include <stdint.h>

#include <memory>
#include <string>

#include "api/media_stream_interface.h"
#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "examples/peerconnection/localvideo/main_wnd.h"
#include "examples/peerconnection/localvideo/peer_connection_localvideo.h"


class FakeMainWnd : public MainWindow {
 public:
  FakeMainWnd(const char* server, int port, bool autoconnect, bool autocall);
  ~FakeMainWnd();

  virtual void RegisterObserver(MainWndCallback* callback);
  virtual bool IsWindow();
  virtual void SwitchToConnectUI();
  virtual void SwitchToPeerList(const Peers& peers);
  virtual void SwitchToStreamingUI();
  virtual void MessageBox(const char* caption, const char* text, bool is_error);
  virtual MainWindow::UI current_ui();
  virtual void StartLocalRenderer(webrtc::VideoTrackInterface* local_video);
  virtual void StopLocalRenderer();
  virtual void StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video);
  virtual void StopRemoteRenderer();

  virtual void QueueUIThreadCallback(int msg_id, void* data);

  // Creates and shows the main window with the |Connect UI| enabled.
  bool Create();

  // Destroys the window.  When the window is destroyed, it ends the
  // main message loop.
  bool Destroy();


  void AutoLogin();




 protected:
  class VideoRenderer : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
   public:
    VideoRenderer(FakeMainWnd* main_wnd,
                  webrtc::VideoTrackInterface* track_to_render);
    virtual ~VideoRenderer();

    // VideoSinkInterface implementation
    void OnFrame(const webrtc::VideoFrame& frame) override;

    const uint8_t* image() const { return image_.get(); }

    int width() const { return width_; }

    int height() const { return height_; }

   protected:
    void SetSize(int width, int height);
    std::unique_ptr<uint8_t[]> image_;
    int width_;
    int height_;
    FakeMainWnd* main_wnd_;
    FILE* file_;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> rendered_track_;
  };

 protected:
  MainWndCallback* callback_;
  std::string server_;
  std::string port_;
  bool autoconnect_;
  bool autocall_;
  std::unique_ptr<VideoRenderer> local_renderer_;
  std::unique_ptr<VideoRenderer> remote_renderer_;
  int width_;
  int height_;
  std::unique_ptr<uint8_t[]> draw_buffer_;
  int draw_buffer_size_;

  int status;
};

#endif