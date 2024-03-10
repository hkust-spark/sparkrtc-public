/*
 *  Copyright 2012 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef EXAMPLES_PEERCONNECTION_LOCALVIDEO_FLAG_DEFS_H_
#define EXAMPLES_PEERCONNECTION_LOCALVIDEO_FLAG_DEFS_H_

#include <string>

#include "absl/flags/flag.h"

extern const uint16_t kDefaultServerPort;  // From defaults.[h|cc]
const uint16_t kDefaultHeight = 1080;
const uint16_t kDefaultWidth = 1920;
const uint16_t kDefaultFps = 30;
const bool kDefaultisGUI = false;
// Define flags for the peerconnect_client testing tool, in a separate
// header file so that they can be shared across the different main.cc's
// for each platform.

ABSL_FLAG(bool,
          autoconnect,
          false,
          "Connect to the server without user "
          "intervention.");
ABSL_FLAG(std::string, server, "127.0.0.1", "The server to connect to.");

ABSL_FLAG(std::string, logname, "NONE", "Logfilename.");

ABSL_FLAG(std::string, file, "NONE", "The video file to stream to the peer.");

ABSL_FLAG(std::string, recon, "recon.yuv", "The received and decoded YUV file");

ABSL_FLAG(int,height,kDefaultHeight,"The height of the video file to stream to the peer.");

ABSL_FLAG(int,width,kDefaultWidth,"The width of the video file to stream to the peer.");

ABSL_FLAG(int,fps,kDefaultFps,"The fps of the video file to stream to the peer.");

ABSL_FLAG(bool,gui,kDefaultisGUI,"Graphical User Interface");

ABSL_FLAG(int,
          port,
          kDefaultServerPort,
          "The port on which the server is listening.");
ABSL_FLAG(
    bool,
    autocall,
    false,
    "Call the first available other client on "
    "the server without user intervention.  Note: this flag should only be set "
    "to true on one of the two clients.");

ABSL_FLAG(
    std::string,
    force_fieldtrials,
    "WebRTC-Bwe-StableBandwidthEstimate/Enabled/WebRTC-Bwe-ProbeRateFallback/Enabled/WebRTC-AddPacingToCongestionWindowPushback/Enabled/",
    "Field trials control experimental features. This flag specifies the field "
    "trials in effect. E.g. running with "
    "--force_fieldtrials=WebRTC-FooFeature/Enabled/ "
    "will assign the group Enabled to field trial WebRTC-FooFeature. Multiple "
    "trials are separated by \"/\"");

#endif  // EXAMPLES_PEERCONNECTION_LOCALVIDEO_FLAG_DEFS_H_
