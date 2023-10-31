#ifndef EXAMPLES_DESKTOP_CAPTURE_DESKTOP_CAPTURER_SOURCE_TEST_H_
#define EXAMPLES_DESKTOP_CAPTURE_DESKTOP_CAPTURER_SOURCE_TEST_H_

#include "api/video/video_frame.h"
#include "api/video/video_source_interface.h"
#include "media/base/video_adapter.h"
#include "media/base/video_broadcaster.h"

namespace webrtc {

class TestDesktopCapturer
    : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
 public:
  TestDesktopCapturer() {}
  ~TestDesktopCapturer() override {}

  void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
                       const rtc::VideoSinkWants& wants) override;

  void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;

 protected:
  // Notify sinkes
  void OnFrame(const webrtc::VideoFrame& frame);

 private:
  void UpdateVideoAdapter();

  rtc::VideoBroadcaster broadcaster_;
  cricket::VideoAdapter video_adapter_;
};

}  // namespace webrtc

#endif  // EXAMPLES_DESKTOP_CAPTURE_DESKTOP_CAPTURER_SOURCE_TEST_H_