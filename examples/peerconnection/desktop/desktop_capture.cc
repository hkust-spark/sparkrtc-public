#include "examples/peerconnection/desktop/desktop_capture.h"

#include "modules/desktop_capture/desktop_capture_options.h"
#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv.h"

namespace webrtc {

OutDesktopCapturer::OutDesktopCapturer() : dc_(nullptr), start_flag_(false) {}

bool OutDesktopCapturer::Init(size_t target_fps, size_t capture_screen_index) 
{
	// 窗口
	/*dc_ = DesktopCapturer::CreateWindowCapturer(
		DesktopCaptureOptions::CreateDefault());*/
	DesktopCaptureOptions options;
	options.set_allow_directx_capturer(true);
	dc_ = DesktopCapturer::CreateScreenCapturer(options);

  if (!dc_)
    return false;

  DesktopCapturer::SourceList sources;
  dc_->GetSourceList(&sources);
  if (capture_screen_index > sources.size()) {
    RTC_LOG(LS_WARNING) << "The total sources of screen is " << sources.size()
                        << ", but require source of index at "
                        << capture_screen_index;
    return false;
  }

  RTC_CHECK(dc_->SelectSource(sources[capture_screen_index].id));
  window_title_ = sources[capture_screen_index].title;
  fps_ = target_fps;

  RTC_LOG(LS_INFO) << "Init OutDesktopCapturer finish window_title = " << window_title_ << " , fps = " << fps_ <<"";
  // Start new thread to capture
  return true;
}

OutDesktopCapturer* OutDesktopCapturer::Create(size_t target_fps,
                                       size_t capture_screen_index) {
  std::unique_ptr<OutDesktopCapturer> dc(new OutDesktopCapturer());
  if (!dc->Init(target_fps, capture_screen_index)) {
    RTC_LOG(LS_WARNING) << "Failed to create OutDesktopCapturer(fps = "
                        << target_fps << ")";
    return nullptr;
  }
  return dc.release();
}

void OutDesktopCapturer::Destory() {
  StopCapture();

  if (!dc_)
    return;

  dc_.reset(nullptr);
}

OutDesktopCapturer::~OutDesktopCapturer() {
  Destory();
}


void OutDesktopCapturer::OnCaptureResult(
    DesktopCapturer::Result result,
    std::unique_ptr<DesktopFrame> frame) {
    RTC_LOG(LS_INFO) << "new Frame";

  static auto timestamp =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          std::chrono::system_clock::now().time_since_epoch())
          .count();
  static size_t cnt = 0;

  cnt++;
  auto timestamp_curr = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())
                            .count();
  if (timestamp_curr - timestamp > 1000) {
    RTC_LOG(LS_INFO) << "FPS: " << cnt;
    cnt = 0;
    timestamp = timestamp_curr;
  }

  // Convert DesktopFrame to VideoFrame
  if (result != DesktopCapturer::Result::SUCCESS) {
    RTC_LOG(LS_ERROR) << "Capture frame faiiled, result: " << result;
  }
  int width = frame->size().width();
  int height = frame->size().height();
  // int half_width = (width + 1) / 2;

  if (!i420_buffer_.get() ||
      i420_buffer_->width() * i420_buffer_->height() < width * height) {
    i420_buffer_ = I420Buffer::Create(width, height);
  }
  // i420_buffer_->set_texture
  libyuv::ConvertToI420(frame->data(), 0, i420_buffer_->MutableDataY(),
                        i420_buffer_->StrideY(), i420_buffer_->MutableDataU(),
                        i420_buffer_->StrideU(), i420_buffer_->MutableDataV(),
                        i420_buffer_->StrideV(), 0, 0, width, height, width,
                        height, libyuv::kRotate0, libyuv::FOURCC_ARGB);


  // setting stream information

  VideoFrame captureFrame =
	  VideoFrame::Builder()
	  .set_video_frame_buffer(i420_buffer_)
	  .set_timestamp_rtp(0)//set_ntp_time_ms
          .set_ntp_time_ms(rtc::TimeMillis())
	  .set_timestamp_ms(rtc::TimeMillis())
	  .set_rotation(kVideoRotation_0)
	  .build();
 // captureFrame.set_ntp_time_ms(0);
  TestDesktopCapturer::OnFrame(captureFrame);
  // rtc media info 
 /* DesktopCaptureSource::OnFrame(
      VideoFrame(i420_buffer_, 0, 0, kVideoRotation_0));*/
}

void OutDesktopCapturer::StartCapture() {
  if (start_flag_) {
    RTC_LOG(LS_WARNING) << "Capture already been running...";
    return;
  }

  start_flag_ = true;

  // Start new thread to capture
  capture_thread_.reset(new std::thread([this]() {
    dc_->Start(this);

    while (start_flag_) {
      dc_->CaptureFrame();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps_));
    }
  }));
}

void OutDesktopCapturer::StopCapture() {
  start_flag_ = false;

  if (capture_thread_ && capture_thread_->joinable()) {
    capture_thread_->join();
  }
}

}  // namespace webrtc