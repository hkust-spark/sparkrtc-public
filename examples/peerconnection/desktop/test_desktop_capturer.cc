#include "examples/peerconnection/desktop/test_desktop_capturer.h"

#include "api/video/i420_buffer.h"
#include "api/video/video_rotation.h"
#include "rtc_base/logging.h"

namespace webrtc {

void TestDesktopCapturer::AddOrUpdateSink(
    rtc::VideoSinkInterface<VideoFrame>* sink,
    const rtc::VideoSinkWants& wants) {
  broadcaster_.AddOrUpdateSink(sink, wants);
  UpdateVideoAdapter();
}

void TestDesktopCapturer::RemoveSink(
    rtc::VideoSinkInterface<VideoFrame>* sink) {
  broadcaster_.RemoveSink(sink);
  UpdateVideoAdapter();
}

void TestDesktopCapturer::UpdateVideoAdapter() {
  video_adapter_.OnSinkWants(broadcaster_.wants());
	// rtc::VideoSinkWants wants = broadcaster_.wants();
	// video_adapter_.OnResolutionFramerateRequest(
	// 	wants.target_pixel_count, wants.max_pixel_count, wants.max_framerate_fps);
}

void TestDesktopCapturer::OnFrame(const VideoFrame& frame) {
  int cropped_width = 0;
  int cropped_height = 0;
  int out_width = 0;
  int out_height = 0;

  if (!video_adapter_.AdaptFrameResolution(
          frame.width(), frame.height(), frame.timestamp_us() * 1000,
          &cropped_width, &cropped_height, &out_width, &out_height)) {
    // Drop frame in order to respect frame rate constraint.
    return;
  }

  if (out_height != frame.height() || out_width != frame.width()) {
    // Video adapter has requested a down-scale. Allocate a new buffer and
    // return scaled version.
    // For simplicity, only scale here without cropping.
    rtc::scoped_refptr<I420Buffer> scaled_buffer =
        I420Buffer::Create(out_width, out_height);
    scaled_buffer->ScaleFrom(*frame.video_frame_buffer()->ToI420());
    VideoFrame::Builder new_frame_builder =
        VideoFrame::Builder()
            .set_video_frame_buffer(scaled_buffer)
            .set_rotation(kVideoRotation_0)
            .set_timestamp_us(frame.timestamp_us())
            .set_id(frame.id());
    broadcaster_.OnFrame(new_frame_builder.build());
  } else {
    // No adaptations needed, just return the frame as is.
    broadcaster_.OnFrame(frame);
  }
}

}  // namespace webrtc