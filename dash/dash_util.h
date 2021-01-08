#pragma once
#include <stdint.h>
#include <string>
#include "dash_stream_interface.h"
namespace basic{
enum DashMessageType:uint8_t{
    DASH_REQ,
    DASH_RES,
};
void ReadSegmentFromFile(std::vector<std::string> &video_log,struct VideoData &video_data);
}
