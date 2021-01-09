#pragma once 
#include "tcp/tcp_server.h"
#include <deque>
namespace basic{
class DashServerEndpoint :public basic::EpollCallbackInterface{
public:
   DashServerEndpoint(basic::BaseContext *context,int fd);
   ~DashServerEndpoint();
    // From EpollCallbackInterface
    void OnRegistration(basic::EpollServer* eps, int fd, int event_mask) override;
    void OnModification(int fd, int event_mask) override;
    void OnEvent(int fd, basic::EpollEvent* event) override;
    void OnUnregistration(int fd, bool replaced) override;
    void OnShutdown(basic::EpollServer* eps, int fd) override;
    std::string Name() const override {return "dash-server";}
private:
    void OnReadEvent(int fd);
    void OnCanWrite(int fd);
    void Responce();
    void Close();
    void DeleteSelf();
    basic::BaseContext* context_=nullptr;
    int fd_=-1;
    std::atomic<bool> destroyed_{false};
    int sent_bytes_=0;
    std::deque<int> request_bytes_;
    int counter_=0;
};
class DashServerBackend:public Backend{
public:
    DashServerBackend();
    void CreateEndpoint(basic::BaseContext *context,int fd) override;
};
class DashSocketFactory: public SocketServerFactory{
public:
    ~DashSocketFactory(){}
    PhysicalSocketServer* CreateSocketServer(BaseContext *context) override;
};    
}
