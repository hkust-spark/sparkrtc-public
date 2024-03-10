/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <glib.h>
#include <gtk/gtk.h>
#include <stdio.h>

#include "absl/flags/parse.h"
#include "api/scoped_refptr.h"
#include "examples/peerconnection/localvideo/conductor.h"
#include "examples/peerconnection/localvideo/flag_defs.h"
#include "examples/peerconnection/localvideo/linux/main_wnd.h"
#include "examples/peerconnection/localvideo/linux/fake_wnd.h"
#include "examples/peerconnection/localvideo/peer_connection_localvideo.h"
#include "rtc_base/physical_socket_server.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"
#include "system_wrappers/include/field_trial.h"
#include "test/field_trial.h"
#include "rtc_base/event_tracer.h"
#include "rtc_base/logging.h"
#include "rtc_base/log_sinks.h"

#include <sys/stat.h>
std::string local_video_filename;
std::string recon_filename;
int local_video_width;
int local_video_height;
int local_video_fps;
bool is_sender = false;
bool is_GUI = false;
class CustomSocketServer : public rtc::PhysicalSocketServer {
 public:
  explicit CustomSocketServer(GtkMainWnd* wnd)
      : wnd_(wnd), conductor_(NULL), client_(NULL) {}
  virtual ~CustomSocketServer() {}

  void SetMessageQueue(rtc::Thread* queue) override { message_queue_ = queue; }

  void set_client(PeerConnectionClient* client) { client_ = client; }
  void set_conductor(Conductor* conductor) { conductor_ = conductor; }

  // Override so that we can also pump the GTK message loop.
  // This function never waits.

  bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
    // RTC_LOG(LS_INFO) << __FUNCTION__ ;
    // Pump GTK events.
    // TODO(henrike): We really should move either the socket server or UI to a
    // different thread.  Alternatively we could look at merging the two loops
    // by implementing a dispatcher for the socket server and/or use
    // g_main_context_set_poll_func.
    //  int64_t start_ts = rtc::TimeMillis();
    // RTC_LOG(LS_INFO) << __FUNCTION__ << " Wait: " << start_ts;
    
    while (gtk_events_pending())
      gtk_main_iteration();
    
    return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(),
                                           process_io);
  }

 protected:
  rtc::Thread* message_queue_;
  GtkMainWnd* wnd_;
  Conductor* conductor_;
  PeerConnectionClient* client_;
};

class CustomSocketServerNoGUi : public rtc::PhysicalSocketServer {
 public:
  explicit CustomSocketServerNoGUi(FakeMainWnd* wnd)
      : wnd_(wnd), conductor_(NULL), client_(NULL) {}
  virtual ~CustomSocketServerNoGUi() {}

  void SetMessageQueue(rtc::Thread* queue) override { message_queue_ = queue; }

  void set_client(PeerConnectionClient* client) { client_ = client; }
  void set_conductor(Conductor* conductor) { conductor_ = conductor; }

  // Override so that we can also pump the GTK message loop.
  // This function never waits.

  bool Wait(webrtc::TimeDelta max_wait_duration, bool process_io) override {
    // RTC_LOG(LS_INFO) << __FUNCTION__ ;
    // Pump GTK events.
    // TODO(henrike): We really should move either the socket server or UI to a
    // different thread.  Alternatively we could look at merging the two loops
    // by implementing a dispatcher for the socket server and/or use
    // g_main_context_set_poll_func.
    //  int64_t start_ts = rtc::TimeMillis();
    // RTC_LOG(LS_INFO) << __FUNCTION__ << " Wait: " << start_ts;
    usleep(50000);
    return rtc::PhysicalSocketServer::Wait(webrtc::TimeDelta::Zero(),
                                           process_io);
  }

 protected:
  rtc::Thread* message_queue_;
  FakeMainWnd* wnd_;
  Conductor* conductor_;
  PeerConnectionClient* client_;
};

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);

  // InitFieldTrialsFromString stores the char*, so the char array must outlive
  // the application.
  const std::string forced_field_trials =
      absl::GetFlag(FLAGS_force_fieldtrials);
  webrtc::field_trial::InitFieldTrialsFromString(forced_field_trials.c_str());

  // Abort if the user specifies a port that is outside the allowed
  // range [1, 65535].
  if ((absl::GetFlag(FLAGS_port) < 1) || (absl::GetFlag(FLAGS_port) > 65535)) {
    printf("Error: %i is not a valid port.\n", absl::GetFlag(FLAGS_port));
    return -1;
  }

  const std::string server = absl::GetFlag(FLAGS_server);

  local_video_filename = absl::GetFlag(FLAGS_file);
  recon_filename = absl::GetFlag(FLAGS_recon);
  local_video_width = absl::GetFlag(FLAGS_width);
  local_video_height = absl::GetFlag(FLAGS_height);
  local_video_fps = absl::GetFlag(FLAGS_fps);
  is_GUI = absl::GetFlag(FLAGS_gui);
  
  std::string logname = absl::GetFlag(FLAGS_logname);

  // If the ./logs directory doesn't exist, create it.
  struct stat st = {0};
  if (stat("./logs", &st) == -1) {
    mkdir("./logs", 0700);
  }


  

  static const std::string  event_log_file_name = "./logs/rtc_event_" + std::to_string(::time(NULL))+ ".json";
  rtc::tracing::StartInternalCapture(event_log_file_name.c_str());

  rtc::LogMessage::LogTimestamps(true);
  rtc::LogMessage::LogThreads(true);

  if (logname == "NONE") {
    logname = "log_" + std::to_string(::time(NULL));
  }

  rtc::FileRotatingLogSink frls("./logs", logname, 10 << 20, 10);
  frls.Init();
  rtc::LogMessage::AddLogToStream(&frls, rtc::LS_VERBOSE);
  
  

  bool autocall = false;
  if (local_video_filename != "NONE") {
    is_sender = true;
    autocall = true;
  }

  if (is_GUI) {
    gtk_init(&argc, &argv);
// g_type_init API is deprecated (and does nothing) since glib 2.35.0, see:
// https://mail.gnome.org/archives/commits-list/2012-November/msg07809.html
#if !GLIB_CHECK_VERSION(2, 35, 0)
    g_type_init();
#endif
// g_thread_init API is deprecated since glib 2.31.0, see release note:
// http://mail.gnome.org/archives/gnome-announce-list/2011-October/msg00041.html
#if !GLIB_CHECK_VERSION(2, 31, 0)
    g_thread_init(NULL);
#endif
    GtkMainWnd wnd(server.c_str(), absl::GetFlag(FLAGS_port),
                   absl::GetFlag(FLAGS_autoconnect), autocall);
    wnd.Create();

    CustomSocketServer socket_server(&wnd);
    rtc::AutoSocketServerThread thread(&socket_server);

    rtc::InitializeSSL();
    // Must be constructed after we set the socketserver.
    PeerConnectionClient client;
    auto conductor = rtc::make_ref_counted<Conductor>(&client, &wnd);
    socket_server.set_client(&client);
    socket_server.set_conductor(conductor.get());

    wnd.OnClicked(NULL);

    thread.Run();

    // gtk_main();
    wnd.Destroy();

    // TODO(henrike): Run the Gtk main loop to tear down the connection.
    /*
    while (gtk_events_pending()) {
      gtk_main_iteration();
    }
    */
    rtc::CleanupSSL();
  }
  else{
    FakeMainWnd wnd(server.c_str(), absl::GetFlag(FLAGS_port),
                   absl::GetFlag(FLAGS_autoconnect), autocall);
    wnd.Create();

    CustomSocketServerNoGUi socket_server(&wnd);
    rtc::AutoSocketServerThread thread(&socket_server);

    rtc::InitializeSSL();
    // Must be constructed after we set the socketserver.
    PeerConnectionClient client;
    auto conductor = rtc::make_ref_counted<Conductor>(&client, &wnd);
    socket_server.set_client(&client);
    socket_server.set_conductor(conductor.get());

    wnd.AutoLogin();

    thread.Run();

    // gtk_main();
    wnd.Destroy();

    // TODO(henrike): Run the Gtk main loop to tear down the connection.
    /*
    while (gtk_events_pending()) {
      gtk_main_iteration();
    }
    */
    rtc::CleanupSSL();
  }

  return 0;
}
