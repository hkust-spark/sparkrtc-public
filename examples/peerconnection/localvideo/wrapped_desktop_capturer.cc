#include "examples/peerconnection/localvideo/wrapped_desktop_capturer.h"


#include "rtc_base/logging.h"
#include "third_party/libyuv/include/libyuv.h"

extern std::string local_video_filename;
extern int local_video_width;
extern int local_video_height;
extern int local_video_fps;
extern bool is_sender;
namespace webrtc {

WrappedDesktopCapturer::WrappedDesktopCapturer() : start_flag_(false) {}

rtc::scoped_refptr<webrtc::test::Video> video_d;

bool WrappedDesktopCapturer::Init() 
{

  if (!is_sender)
  {
    return true;
  }
  // rtc::scoped_refptr<webrtc::test::Video> video;
  video_d = webrtc::test::OpenYuvFile(local_video_filename, local_video_width, local_video_height);

	
  fps_ = local_video_fps;

 
  // Start new thread to capture
  return true;
}

WrappedDesktopCapturer* WrappedDesktopCapturer::Create() {
  std::unique_ptr<WrappedDesktopCapturer> dc(new WrappedDesktopCapturer());
  if (!dc->Init()) {
    RTC_LOG(LS_ERROR) << "Failed to create WrappedDesktopCapturer";
    return nullptr;
  }
  return dc.release();
}

void WrappedDesktopCapturer::Destory() {
  StopCapture();

}

WrappedDesktopCapturer::~WrappedDesktopCapturer() {
  Destory();
}

void WrappedDesktopCapturer::StartCapture() {
  if (!is_sender) {
    RTC_LOG(LS_WARNING) << "Not sender, do not start capture";
    return;
  }

  start_flag_ = true;

  // Start new thread to capture
  capture_thread_.reset(new std::thread([this]() {
    // dc_->Start(this);

    while (start_flag_) {
      // dc_->CaptureFrame();
      std::this_thread::sleep_for(std::chrono::milliseconds(1000 / fps_));
      int total_frame = video_d->number_of_frames();

      if (frame_count_ >= total_frame) {
        frame_count_ = 0;
      }
      rtc::scoped_refptr<webrtc::I420BufferInterface> frame_buffer = video_d->GetFrame(frame_count_++);
      webrtc::VideoFrame captureFrame =
        webrtc::VideoFrame::Builder()
        .set_video_frame_buffer(frame_buffer)
        .set_timestamp_rtp(0)//set_ntp_time_ms
              .set_ntp_time_ms(rtc::TimeMillis())
        .set_timestamp_ms(rtc::TimeMillis())
        .set_rotation(webrtc::kVideoRotation_0)
        .build();
      RTC_LOG(LS_INFO) << "Sending " << frame_count_ << " " << rtc::TimeMillis();
    // captureFrame.set_ntp_time_ms(0);
      TestDesktopCapturer::OnFrame(captureFrame);
    }
  }));
}

void WrappedDesktopCapturer::StopCapture() {
  start_flag_ = false;

  if (capture_thread_ && capture_thread_->joinable()) {
    capture_thread_->join();
  }
}

}  // namespace webrtc