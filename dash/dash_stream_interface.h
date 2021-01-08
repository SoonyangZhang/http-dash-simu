#pragma once
#include <vector>
#include <deque>
#include <stdint.h>
#include "base/base_time.h"
namespace basic{
struct AlgorithmReply
{
  AlgorithmReply():nextQuality(0),nextDownloadDelay(QuicTime::Delta::Zero()){}
  int nextQuality; //!< representation level index of the next segement to be downloaded by the client
  QuicTime::Delta nextDownloadDelay; 
};
struct ThroughputData
{
  std::deque<QuicTime> transmissionRequested;       //!< Simulation time in microseconds when a segment was requested by the client
  std::deque<QuicTime> transmissionStart;       //!< Simulation time in microseconds when the first packet of a segment was received
  std::deque<QuicTime> transmissionEnd;       //!< Simulation time in microseconds when the last packet of a segment was received
  std::deque<int64_t> bytesReceived;       //!< Number of bytes received, i.e. segment size
};
struct BufferData
{
  std::vector<QuicTime> timeNow;       //!< current simulation time
  std::vector<int64_t> bufferLevelOld;       //!< buffer level in microseconds before adding segment duration (in microseconds) of just downloaded segment
  std::vector<int64_t> bufferLevelNew;       //!< buffer level in microseconds after adding segment duration (in microseconds) of just downloaded segment
};
typedef std::vector<int64_t> SegmentSize;
struct VideoData
{
  std::vector<SegmentSize> representation;       //!< vector holding representation levels in the first dimension and their particular segment sizes in bytes in the second dimension
  std::vector <double> averageBitrate;       //!< holding the average bitrate of a segment in representation i in bits
  int64_t segmentDuration;       //!< duration of a segment in microseconds
}; 
struct PlaybackData
{
  std::vector <int64_t> playbackIndex;       //!< Index of the video segment
  std::vector <QuicTime> playbackStart; //!< Point in time in microseconds when playback of this segment started
};
}