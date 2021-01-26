#include <unistd.h>
#include <error.h>
#include <sys/types.h>
#include <algorithm>
#include "dash/dash_client.h"
#include "base/byte_codec.h"
#include "logging/logging.h"
#include <iostream>
namespace basic{
namespace {
const int kBufferSize=1500;
}
class PlayerDelegate:public basic::BaseAlarm::Delegate{
public:
    PlayerDelegate(DashClient *client):client_(client){}
    void OnAlarm() override{
        client_->PlayBackHandle();
    }
private:
    DashClient *client_=nullptr;
};
class RequestDelegate:public basic::BaseAlarm::Delegate{
public:
    RequestDelegate(DashClient *client):client_(client){}
    void OnAlarm() override{
        client_->DelayRequestHandle();
    }
private:
    DashClient *client_=nullptr;    
};
DashClient::DashClient(basic::EpollServer *eps,int segment_ms,std::vector<std::string> &video_log,std::string &trace_name,
    int init_segments):eps_(eps){
    clock_.reset(new basic::EpollClock(eps_));
    alarm_factory_.reset(new basic::BaseEpollAlarmFactory(eps_));
    video_data_.segmentDuration=segment_ms;
    ReadSegmentFromFile(video_log,video_data_);
    algorithm_.reset(new FestiveAlgorithm(kFestiveTarget,kFestiveHorizon,segment_ms));
    init_segments_=init_segments;
    frames_in_segment_=segment_ms*kFramesPerSecond/1000;
    OpenTrace(trace_name);
}
DashClient::~DashClient(){
    if(player_alarm_){
        player_alarm_->Cancel();
    }
    if(request_alarm_){
        request_alarm_->Cancel();
    }
    eps_->UnregisterFD(fd_);
    Close();
    CalculateQoE();
    CloseTrace();
}
bool DashClient::AsynConnect(basic::SocketAddress &local,basic::SocketAddress& remote){
    src_addr_=local.generic_address();
    dst_addr_=remote.generic_address();
    int yes=1;
    bool success=false;
    if ((fd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return success;
    }
    size_t addr_size = sizeof(struct sockaddr_storage);
    if(bind(fd_, (struct sockaddr *)&src_addr_, addr_size)<0){
        Close();
        return success;
    }
    if(setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))!=0){
        Close();
        return success;        
    }
    if(video_data_.representation.size()==0){
        Close();
        return success;         
    }
    eps_->RegisterFD(fd_, this,EPOLLOUT | EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET);
    if(connect(fd_,(struct sockaddr *)&dst_addr_,addr_size) == -1&& errno != EINPROGRESS){
        //connect doesn't work, are we running out of available ports ? if yes, destruct the socket   
        if (errno == EAGAIN){
            eps_->UnregisterFD(fd_);
            Close();
            return success;                
        }
        player_state_=PLAYER_INIT_BUFFERING;        
    }
    status_=TCP_CONNECTING;
    return true;
}
void DashClient::OnEvent(int fd, basic::EpollEvent* event){
    if (event->in_events&(EPOLLERR|EPOLLRDHUP|EPOLLHUP)){
        std::cout<<"conntected failed"<<std::endl;
        player_state_=PLAYER_DONE;
        eps_->UnregisterFD(fd_);
        Close();   
    }
    if(event->in_events & EPOLLIN){
        OnReadEvent(fd);
    }    
    if(event->in_events&EPOLLOUT){
        if(status_==TCP_CONNECTING){
            status_=TCP_CONNECTED;
            eps_->ModifyCallback(fd_,EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLET);
            OnConnected();
        }
    }
}
void DashClient::OnShutdown(basic::EpollServer* eps, int fd){
    Close();
}
int64_t DashClient::get_buffer_level_ms(){
    int64_t ms=0;
    if(!buffer_data_.bufferLevelNew.empty()){
        ms=buffer_data_.bufferLevelNew.back();
    }
    return ms;
}
int64_t DashClient::get_buffer_diff_ms(){
    int64_t diff=0;
    if(buffer_data_.bufferLevelNew.size()>1){
        int64_t now=buffer_data_.bufferLevelNew.end()[-1];
        int64_t old=buffer_data_.bufferLevelNew.end()[-2];
        diff=now-old;
    }
    return diff;
}
void DashClient::SetAdaptationAlgorithm(std::string &algo){
    if(algo.compare("panda")==0){
        algorithm_.reset(new PandaAlgorithm());
    }else if(algo.compare("tobasco")==0){
        algorithm_.reset(new TobascoAlgorithm());    
    }else if(algo.compare("osmp")==0){
        algorithm_.reset(new OsmpAlgorithm());
    }else if(algo.compare("raahs")==0){
        algorithm_.reset(new RaahsAlgorithm()); 
    }else if(algo.compare("fdash")==0){
        algorithm_.reset(new FdashAlgorithm());
    }else if(algo.compare("sftm")==0){
        algorithm_.reset(new SftmAlgorithm());
    }else if(algo.compare("svaa")==0){
        algorithm_.reset(new SvaaAlgorithm());
    }
}
void DashClient::OnReadEvent(int fd){
    char buffer[kBufferSize];
    while(true){
        int nbytes=read(fd,buffer,kBufferSize);  
        if (nbytes == -1) {
            //if(errno == EWOULDBLOCK|| errno == EAGAIN){}
            break;            
        }else if(nbytes==0){
            eps_->UnregisterFD(fd_);            
        }else{
            QuicTime now=clock_->ApproximateNow();
            if(read_bytes_==0){
                throughput_.transmissionStart.push_back(now);
            }
            read_bytes_+=nbytes;
            if(read_bytes_==segment_bytes_){
                session_bytes_+=segment_bytes_;
                buffer_data_.timeNow.push_back(now);
                if(segment_counter_>0){
                    int64_t ms=buffer_data_.bufferLevelNew.back ();
                    if(player_state_==PLAYER_INIT_BUFFERING){
                        
                    }else{
                        ms=ms-(now-throughput_.transmissionEnd.back()).ToMilliseconds();
                    }
                    
                    buffer_data_.bufferLevelOld.push_back(std::max(ms,(int64_t)0));
                }else{
                    buffer_data_.bufferLevelOld.push_back(0);
                }
                buffer_data_.bufferLevelNew.push_back (buffer_data_.bufferLevelOld.back () + video_data_.segmentDuration);
                throughput_.transmissionEnd.push_back(now);
                throughput_.bytesReceived.push_back(read_bytes_);
                segment_counter_++;
                segments_in_buffer_++;
                CheckBufferStatus();
                if(video_data_.representation.size()>0){
                    int segment_num=video_data_.representation[0].size();
                    if(index_==segment_num){
                        eps_->UnregisterFD(fd_);
                        std::cout<<"read done"<<std::endl;
                        Close();
                        return ;
                    }
                }
                RequestSegment();
                LogInfomation();
            }
        }       
    }     
}
void DashClient::OnConnected(){
    first_request_time_=clock_->ApproximateNow();
    player_state_=PLAYER_INIT_BUFFERING;
    player_alarm_.reset(alarm_factory_->CreateAlarm(new PlayerDelegate(this)));
    request_alarm_.reset(alarm_factory_->CreateAlarm(new RequestDelegate(this)));
    RequestSegment();
    LogInfomation();
}
void DashClient::Close(){
    if(fd_>0){
        status_=TCP_DISCONNECT;
        close(fd_);
        fd_=-1;        
    }      
}
void DashClient::DelayRequestHandle(){
    if(video_data_.representation.size()>0){
        int segment_num=video_data_.representation[0].size();
        QuicTime now=clock_->ApproximateNow();
        if(index_<segment_num){
            throughput_.transmissionRequested.push_back(now);
            SendRequest(segment_bytes_);
            index_++;
        }
        
    }    
}
void DashClient::RequestSegment(){
    QuicTime now=clock_->ApproximateNow();
    int segment_time=video_data_.segmentDuration;
    AlgorithmReply reply=algorithm_->GetNextQuality(this,now,quality_,index_);
    quality_=reply.nextQuality;
    history_quality_.push_back(quality_);
    read_bytes_=0;
    segment_bytes_=video_data_.representation[quality_][index_];
    if(!reply.nextDownloadDelay.IsZero()){
        request_alarm_->Update(now+reply.nextDownloadDelay,QuicTime::Delta::Zero());       
    }else{
        DelayRequestHandle();
    }
    
}
void DashClient::SendRequest(int bytes){
    char buffer[kBufferSize];
    DataWriter writer(buffer,kBufferSize);
    uint8_t type=DashMessageType::DASH_REQ;
    bool success=writer.WriteUInt8(type)&&
                 writer.WriteUInt32(bytes);
    CHECK(success);
    if(fd_>0){
        write(fd_,buffer,writer.length());
    }
}
void DashClient::PlayBackHandle(){
    if(segments_in_buffer_>0){
        played_frames_++;
        if(played_frames_%frames_in_segment_==0){
            segments_in_buffer_--;
            playback_index_++;
            LogInfomation();            
        }
    }
    if(segments_in_buffer_==0){
        int segment_num=video_data_.representation[0].size();
        if(playback_index_>=segment_num){
            player_state_=PLAYER_DONE;
        }else{
            player_state_=PLAYER_PAUSED;
            QuicTime now=clock_->ApproximateNow();
            pause_start_=now;
        }
    }
    if(segments_in_buffer_>0){
        QuicTime now=clock_->ApproximateNow();
        int ms=1000/kFramesPerSecond;
        QuicTime::Delta delay=QuicTime::Delta::FromMilliseconds(ms);
        player_alarm_->Update(now+delay,QuicTime::Delta::Zero());
    }
}
void DashClient::CheckBufferStatus(){
    QuicTime now=clock_->ApproximateNow();
    if((player_state_==PLAYER_INIT_BUFFERING)&&(segments_in_buffer_>=init_segments_)){
        player_state_=PLAYER_PLAY;
        startup_time_=now-first_request_time_;
        player_alarm_->Update(now,QuicTime::Delta::Zero());
    }
    if((player_state_==PLAYER_PAUSED)&&(segments_in_buffer_>0)){
        player_state_=PLAYER_PLAY;
        if(pause_start_>QuicTime::Zero()){
            pause_time_=pause_time_+(now-pause_start_);
        }
        pause_start_=QuicTime::Zero();
        player_alarm_->Update(now,QuicTime::Delta::Zero());
    }
}
void DashClient::OpenTrace(std::string &trace_name){
    m_trace.open(trace_name.c_str(), std::fstream::out);
}
void DashClient::CloseTrace(){
    if(m_trace.is_open()){
        m_trace.close();
    }
}
void DashClient::LogInfomation(){
    if(m_trace.is_open()){
        QuicTime now=clock_->ApproximateNow();
        int wall_time=(now-first_request_time_).ToMilliseconds();
        m_trace<<wall_time<<"\t"<<segments_in_buffer_<<"\t"<<quality_<<std::endl;
        m_trace.flush();        
    }
}
void DashClient::CalculateQoE(){
    double total_qoe=0.0;
    int64_t bytes=0;
    double average_kbps_1=0.0;
    double average_kbps_2=0.0;
    if(video_data_.representation.size()==0){
        return ;
    }
    int segment_num=video_data_.representation[0].size();
    average_kbps_1=1.0*session_bytes_*8/(segment_num*video_data_.segmentDuration);
    QuicTime::Delta denominator=QuicTime::Delta::Zero();
    int n=throughput_.transmissionEnd.size();
    for(int i=0;i<n;i++){
        denominator=denominator+(throughput_.transmissionEnd.at(i)-throughput_.transmissionStart.at(i));
    }
    if(!denominator.IsZero()){
        average_kbps_2=1.0*session_bytes_*8/(denominator.ToMilliseconds());
    }
    double total_bitrate=0.0;
    double total_instability=0.0;
    n=history_quality_.size();
    for(int i=0;i<n;i++){
        double a=video_data_.averageBitrate.at(history_quality_.at(i))/1000;
        total_bitrate+=a;
        if(i<n-1){
            double b=video_data_.averageBitrate.at(history_quality_.at(i+1))/1000;
            total_instability+=std::abs(b-a);
        }
    }
    double time=((pause_time_+startup_time_).ToMilliseconds())*1.0/1000;
    total_qoe=total_bitrate/n-total_instability/(n-1)-kRebufPenality*time;
    if(m_trace.is_open()){
        QuicTime now=clock_->ApproximateNow();
        int wall_time=(now-first_request_time_).ToMilliseconds();
        m_trace<<wall_time<<"\t"<<average_kbps_1<<"\t"<<average_kbps_2<<"\t"<<total_qoe<<std::endl;        
    }
}

}
