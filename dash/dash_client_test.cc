#include "base/cmdline.h"
#include "base/socket_address.h"
#include "base/epoll_api.h"
#include "dash/dash_client.h"
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <signal.h>
#include <iostream>
#include <vector>
/*
    ./t_dash_client -l 127.0.0.1 -r 127.0.0.1  -p 3333
*/
using namespace basic;
using namespace std;
static volatile bool g_running=true;
void signal_exit_handler(int sig)
{
    g_running=false;
}
int main(int argc, char *argv[]){
    signal(SIGTERM, signal_exit_handler);
    signal(SIGINT, signal_exit_handler);
    signal(SIGTSTP, signal_exit_handler); 
    cmdline::parser a;
    a.add<string>("remote", 'r', "remote ip", false, "127.0.0.1");
    a.add<string>("local", 'l', "local ip", false, "0.0.0.0");
    a.add<string>("algo", 'a', "adaptation algo", false, "festive");
    a.add<uint16_t>("port", 'p', "port number", false, 3333, cmdline::range(1, 65535));    
    a.parse_check(argc, argv);
    std::string remote_str=a.get<string>("remote");
    std::string local_str=a.get<string>("local");
    std::string adapation=a.get<string>("algo");
    uint16_t port=a.get<uint16_t>("port");
    IpAddress src_ip;
    src_ip.FromString(local_str);
    SocketAddress src_addr(src_ip,0);
    
    IpAddress dst_ip;
    dst_ip.FromString(remote_str);
    SocketAddress dst_addr(dst_ip,port);

    /*
    302.207
    750.274
    1197.18
    1842.64
    2837.87
    4279.25*/
    std::string common("video_size_");
    std::vector<std::string> video_log;
    int n=6;
    for(int i=0;i<n;i++){
        std::string file_name=common+std::to_string(i);
        video_log.push_back(file_name);
    }
    int duration=4000;
    basic::EpollServer *epoll_server=new basic::EpollServer();
    std::vector<uint32_t> request_bytes{15000,18000};
    std::string result;
    std::unique_ptr<AdaptationAlgorithm> algo;
    if(adapation.compare("panda")==0){
        result=std::string("dash_panda_result.txt");
    }else if(adapation.compare("tobasco")==0){
        result=std::string("dash_tobasco_result.txt");        
    }else if(adapation.compare("osmp")==0){
        result=std::string("dash_osmp_result.txt");        
    }else if(adapation.compare("raahs")==0){
        result=std::string("dash_raahs_result.txt");        
    }else if(adapation.compare("fdash")==0){
        result=std::string("dash_fdash_result.txt");        
    }else if(adapation.compare("sftm")==0){
        result=std::string("dash_sftm_result.txt");        
    }else if(adapation.compare("svaa")==0){
        result=std::string("dash_svaa_result.txt");        
    }else{
        result=std::string("dash_festive_result.txt");
    }
    basic::DashClient *client=new basic::DashClient(epoll_server,duration,video_log,result);
    client->SetAdaptationAlgorithm(adapation);
    bool success=client->AsynConnect(src_addr,dst_addr);
    if(success){  
        while(g_running){
            epoll_server->WaitForEventsAndExecuteCallbacks();
            if(client->IsTerminated()){
                break;
            }
        }
    }else{
        std::cout<<"success failed"<<std::endl;
    }
    delete client;
    delete epoll_server;
    return 0;
}
