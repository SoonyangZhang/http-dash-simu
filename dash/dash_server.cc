#include <algorithm>
#include <unistd.h>
#include "dash_server.h"
#include "base/byte_codec.h"
#include "dash_util.h"
#include <iostream>
namespace basic{
namespace {
const int kBufferSize=1500;    
}
DashServerEndpoint::DashServerEndpoint(basic::BaseContext *context,int fd):
context_(context),fd_(fd){
    context_->epoll_server()->RegisterFD(fd_, this,EPOLLIN|EPOLLET);
}
DashServerEndpoint::~DashServerEndpoint(){
    std::cout<<"DashServerEndpoint dtor"<<std::endl;
}
void DashServerEndpoint::OnRegistration(basic::EpollServer* eps, int fd, int event_mask){}
void DashServerEndpoint::OnModification(int fd, int event_mask){}
void DashServerEndpoint::OnEvent(int fd, basic::EpollEvent* event){
    if(event->in_events & EPOLLIN){
        OnReadEvent(fd);
    }
    if(event->in_events&EPOLLOUT){
        OnCanWrite(fd);
    }
}
void DashServerEndpoint::OnUnregistration(int fd, bool replaced){
    Close();
    DeleteSelf();
}
void DashServerEndpoint::OnShutdown(basic::EpollServer* eps, int fd){
    Close();
    DeleteSelf();
}
void DashServerEndpoint::OnReadEvent(int fd){
    char buffer[kBufferSize];
    while(true){
        int nbytes=read(fd,buffer,kBufferSize);  
        //https://github.com/eklitzke/epollet/blob/master/poll.c  
        if (nbytes == -1) {
            //if(errno == EWOULDBLOCK|| errno == EAGAIN){}
            break;            
        }else if(nbytes==0){
            context_->epoll_server()->UnregisterFD(fd_);            
        }else{
            DataReader  reader(buffer,nbytes);
            uint8_t type;
            bool success=reader.ReadUInt8(&type);
            if(success&&(type==DashMessageType::DASH_REQ)){
                uint32_t length=0;
                success=reader.ReadUInt32(&length);
                if(success){
                    std::cout<<"req "<<counter_<<" "<<length<<std::endl;
                    counter_++;
                    request_bytes_.push_back((int)length);
                    if(request_bytes_.size()>0){                  
                        context_->epoll_server()->ModifyCallback(fd,EPOLLIN|EPOLLOUT);
                        Responce();
                    }
                }
            }
        }       
    }    
}
void DashServerEndpoint::OnCanWrite(int fd){
    Responce();
}
void DashServerEndpoint::Responce(){
    int num=request_bytes_.size();
    while(num>0){
        char buffer[kBufferSize];
        uint32_t bytes=request_bytes_.front();
        bool full=false;
        while(sent_bytes_<bytes){
            int intend=std::min(kBufferSize,(int)(bytes-sent_bytes_));
            int sent=write(fd_,buffer,intend);
            if(sent<0){
                full=true;
                break;
            }
            sent_bytes_+=sent;
        }
        if(sent_bytes_==bytes){
            request_bytes_.pop_front();
            num--;
            sent_bytes_=0;
            if(num==0){
                context_->epoll_server()->ModifyCallback(fd_,EPOLLIN);
            }
        }
        if(full){
            break;
        }
    }
}
void DashServerEndpoint::Close(){
    if(fd_>0){
        close(fd_);
        fd_=-1;
    }    
}
void DashServerEndpoint::DeleteSelf(){
    if(destroyed_){
        return;
    }
    destroyed_=true;
    context_->PostTask([this]{
        delete this;
    });
}
DashServerBackend::DashServerBackend(){}
void DashServerBackend::CreateEndpoint(basic::BaseContext *context,int fd){
    DashServerEndpoint *endpoint=new DashServerEndpoint(context,fd);
    UNUSED(endpoint);
}
PhysicalSocketServer* DashSocketFactory::CreateSocketServer(BaseContext *context){
    std::unique_ptr<DashServerBackend> backend(new DashServerBackend());
    return new  PhysicalSocketServer(context,std::move(backend));
}
}
