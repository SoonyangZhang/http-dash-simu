#pragma once
/*
    Reference from https://github.com/haraldott/dash
*/
#include <vector>
#include <deque>
#include <string>
#include <fstream>
#include "base/base_epoll_clock.h"
#include "base/base_alarm.h"
#include "base/base_epoll_alarm_factory.h"
#include "dash_constant.h"
#include "tcp_client.h"
#include "dash_util.h"
#include "dash_algorithm.h"
namespace basic{
enum DashPlayerState{
    PLAYER_NOT_STARTED,
    PLAYER_INIT_BUFFERING,
    PLAYER_PLAY,
    PLAYER_PAUSED,
    PLAYER_DONE,
};
class DashClient :public basic::EpollCallbackInterface{
public:
    DashClient(basic::EpollServer *eps,int segment_ms,std::vector<std::string> &video_log,std::string &trace_name,
    int init_segments=kInitialSegments);
    ~DashClient();
    bool AsynConnect(basic::SocketAddress &local,basic::SocketAddress &remote);
    // From EpollCallbackInterface
    void OnRegistration(basic::EpollServer* eps, int fd, int event_mask) override{}
    void OnModification(int fd, int event_mask) override{}
    void OnEvent(int fd, basic::EpollEvent* event) override;
    void OnUnregistration(int fd, bool replaced) override{}
    void OnShutdown(basic::EpollServer* eps, int fd) override;
    std::string Name() const override {return "dash-client";}
    bool IsTerminated() {return player_state_==PLAYER_DONE;}
    int  get_played_frames() {return played_frames_;}
    int64_t get_buffer_level_ms();
    int64_t get_buffer_diff_ms();
    void PlayBackHandle();
    void DelayRequestHandle();
    void SetAdaptationAlgorithm(std::string &algo);
    const VideoData & get_video_data() {return video_data_ ;}
    const ThroughputData &get_throughput(){return throughput_;}
    const std::deque<int> & get_history_quality(){return history_quality_;}
    const BufferData & get_buffer_data(){return buffer_data_;}
private:
    void OnReadEvent(int fd);
    void OnConnected();
    void Close();
    void RequestSegment();
    void SendRequest(int bytes);
    void CheckBufferStatus();
    void OpenTrace(std::string &trace_name);
    void CloseTrace();
    void LogInfomation();
    void CalculateQoE();
    basic::EpollServer *eps_=nullptr;
    std::fstream m_trace;
    std::unique_ptr<basic::EpollClock> clock_;
    std::unique_ptr<basic::BaseEpollAlarmFactory> alarm_factory_;
    std::unique_ptr<basic::BaseAlarm> player_alarm_;
    std::unique_ptr<basic::BaseAlarm> request_alarm_;
    struct VideoData video_data_;
    std::deque<int> history_quality_;
    int fd_=-1;
    int quality_=0;
    int index_=0;
    int playback_index_=0;
    int played_frames_=0;
    int frames_in_segment_=0;
    int64_t segments_in_buffer_=0;
    uint64_t segment_bytes_=0;
    uint64_t read_bytes_=0;
    uint64_t session_bytes_=0;
    QuicTime first_request_time_{QuicTime::Zero()};
    QuicTime pause_start_{QuicTime::Zero()};
    QuicTime::Delta pause_time_{QuicTime::Delta::Zero()};
    QuicTime::Delta startup_time_{QuicTime::Delta::Zero()};
    struct sockaddr_storage src_addr_;
    struct sockaddr_storage dst_addr_;
    TcpConnectionStatus status_{TCP_DISCONNECT};
    DashPlayerState player_state_=PLAYER_NOT_STARTED;
    ThroughputData throughput_;
    BufferData buffer_data_;
    std::unique_ptr<AdaptationAlgorithm> algorithm_;
    int init_segments_;
    int segment_counter_=0;
};    
}
