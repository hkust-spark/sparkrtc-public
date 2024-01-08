#ifndef EXAMPLES_LOCALVIDEO_CAPTURE_LOCALVIDEO_CAPTURER_TEST_H_
#define EXAMPLES_LOCALVIDEO_CAPTURE_LOCALVIDEO_CAPTURER_TEST_H_

#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"
#include "examples/peerconnection/localvideo/test_desktop_capturer.h"
#include "api/video/i420_buffer.h"

#include <thread>
#include <atomic>
#include "rtc_tools/video_file_reader.h"
namespace webrtc {

class WrappedDesktopCapturer : public TestDesktopCapturer,
                       public rtc::VideoSinkInterface<VideoFrame> {
 public:

  static WrappedDesktopCapturer* Create();

  ~WrappedDesktopCapturer() override;

  void StartCapture();
  void StopCapture();

  void OnFrame(const VideoFrame& frame) override {}

  std::string GetWindowTitle() const { return window_title_; }

 private:
  WrappedDesktopCapturer();
  bool Init();
  void Destory();



  size_t fps_;
  std::string window_title_;

  std::unique_ptr<std::thread> capture_thread_;
  std::atomic_bool start_flag_;

  rtc::scoped_refptr<I420Buffer> i420_buffer_;

  rtc::VideoBroadcaster broadcaster_;
  cricket::VideoAdapter video_adapter_;


  
  int frame_count_ = 0;
};
}  // namespace webrtc

#endif  // EXAMPLES_LOCALVIDEO_CAPTURE_LOCALVIDEO_CAPTURER_TEST_H_