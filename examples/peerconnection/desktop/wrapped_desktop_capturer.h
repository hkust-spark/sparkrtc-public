#ifndef EXAMPLES_DESKTOP_CAPTURE_DESKTOP_CAPTURER_TEST_H_
#define EXAMPLES_DESKTOP_CAPTURE_DESKTOP_CAPTURER_TEST_H_

#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "examples/peerconnection/desktop/test_desktop_capturer.h"
#include "modules/desktop_capture/desktop_capturer.h"
#include "modules/desktop_capture/desktop_frame.h"
#include "api/video/i420_buffer.h"

#include <thread>
#include <atomic>

namespace webrtc {

class WrappedDesktopCapturer : public TestDesktopCapturer,
                       public DesktopCapturer::Callback,
                       public rtc::VideoSinkInterface<VideoFrame> {
 public:
  static WrappedDesktopCapturer* Create(size_t target_fps, size_t capture_screen_index);

  ~WrappedDesktopCapturer() override;

  void StartCapture();
  void StopCapture();

  void OnFrame(const VideoFrame& frame) override {}

  std::string GetWindowTitle() const { return window_title_; }

 private:
  WrappedDesktopCapturer();
  bool Init(size_t target_fps, size_t capture_screen_index);
  void Destory();

  void OnCaptureResult(DesktopCapturer::Result result,
                       std::unique_ptr<DesktopFrame> frame) override;

  std::unique_ptr<DesktopCapturer> dc_;

  size_t fps_;
  std::string window_title_;

  std::unique_ptr<std::thread> capture_thread_;
  std::atomic_bool start_flag_;

  rtc::scoped_refptr<I420Buffer> i420_buffer_;

  rtc::VideoBroadcaster broadcaster_;
  cricket::VideoAdapter video_adapter_;
};
}  // namespace webrtc

#endif  // EXAMPLES_DESKTOP_CAPTURE_DESKTOP_CAPTURER_TEST_H_