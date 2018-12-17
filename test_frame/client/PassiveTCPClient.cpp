//  
//  PassiveTCPClient.cpp  
//  被动TCP客户端  
//  
//  Created by chenjianjun on 15/9/7.  
//  Copyright (c) 2015年 jsbn. All rights reserved.  
//  
  
#include "PassiveTCPClient.h"  
#include "NetFrame.h"  
  
namespace NAME_SPACE {  
      
    void PassiveTCPTimeOutEventCb(evutil_socket_t fd, short, void *data) {  
          
        PassiveTCPClient *pPassiveTCPClient = (PassiveTCPClient*)data;  
        if (pPassiveTCPClient->GetHeartFlg()) {  
            // 超时清除标志  
            pPassiveTCPClient->SetHeartFlg(false);  
        } else {  
            // 心跳超时回调  
            pPassiveTCPClient->ProcEvent(BEV_EVENT_TIMEOUT);  
        }  
    }  
      
    void PassiveTCPReadEventCb(struct bufferevent *bev, void *data) {  
          
        PassiveTCPClient* pPassiveTCPClient = (PassiveTCPClient*)data;  
          
        static char databuf[40960];  
        size_t datalen = 0;  
        size_t nbytes;  
          
        while ((nbytes = evbuffer_get_length(bev->input)) > 0) {  
            evbuffer_remove(bev->input, databuf+datalen, sizeof(databuf)-datalen);  
            datalen += nbytes;  
        }  
          
        // 有数据往来，设置标志  
        pPassiveTCPClient->SetHeartFlg(true);  
          
        // 数据接收回调  
        pPassiveTCPClient->PutRecvData(databuf, datalen);  
    }  
      
    void PassiveTCPEventCb(struct bufferevent *bev, short events, void *data) {  
          
        PassiveTCPClient* pPassiveTCPClient = (PassiveTCPClient*)data;  
          
        // 处理事件  
        pPassiveTCPClient->ProcEvent(events);  
    }  
      
    PassiveTCPClient::PassiveTCPClient(SOCKET fd, struct sockaddr_in* sa, short heart_time)  
    :_fd(fd),  
    _client_ip(inet_ntoa(sa->sin_addr)),  
    _client_port(ntohs(sa->sin_port)),  
    _bev(nullptr),  
    _heart_flg(false),  
    _heart_time(heart_time),  
    _pTCPClientSignal(nullptr)  
    {}  
      
    PassiveTCPClient::~PassiveTCPClient() {  
        StopWork();  
        _pTCPClientSignal = nullptr;  
    }  
      
    bool PassiveTCPClient::StartWork(TCPClientSignal* pTCPClientSignal) {  
          
        if (_bev) {  
            return false;  
        }  
          
        _bev = bufferevent_socket_new(NetFrame::_base,  
                                      _fd,  
                                      BEV_OPT_CLOSE_ON_FREE|BEV_OPT_THREADSAFE);  
        if (_bev == nullptr) {  
            return false;  
        }  
          
        _event = event_new(NetFrame::_base,  
                           _fd,  
                           EV_TIMEOUT|EV_PERSIST,  
                           PassiveTCPTimeOutEventCb, this);  
        if (_event == nullptr) {  
            bufferevent_free(_bev);  
            _bev = nullptr;  
            return false;  
        }  
          
        _pTCPClientSignal = pTCPClientSignal;  
        // 设置心跳检测时间  
        struct timeval timeout = {_heart_time, 0};  
        event_add(_event, &timeout);  
          
        bufferevent_setcb(_bev, PassiveTCPReadEventCb, nullptr, PassiveTCPEventCb, this);  
        bufferevent_enable(_bev, EV_READ);  
          
        return true;  
    }  
      
    void PassiveTCPClient::StopWork() {  
          
        if (_bev) {  
            bufferevent_disable(_bev, EV_READ);  
            bufferevent_free(_bev);  
            _bev = nullptr;  
        }  
          
        if (_event) {  
            event_del(_event);  
            event_free(_event);  
            _event = nullptr;  
        }  
          
        // 不要对_pPassiveTCPClientSignal置null，释放由外部传入者负责  
    }  
      
    int PassiveTCPClient::SendData(void* pdata, size_t len) {  
          
        if (_bev == nullptr) {  
            return FUNC_FAILED;  
        }  
  
        if (bufferevent_write(_bev, pdata, len) < 0) {  
            return FUNC_FAILED;  
        }  
          
        return FUNC_SUCCESS;  
    }  
      
    void PassiveTCPClient::PutRecvData(void* data, size_t len) {  
          
        if (_pTCPClientSignal) {  
            _pTCPClientSignal->SignalRecvData(_fd, data, len);  
        }  
    }  
      
    void PassiveTCPClient::ProcEvent(short events) {  
          
        if (!_pTCPClientSignal) {  
            return;  
        }  
          
        if (events & BEV_EVENT_CONNECTED) {  
            _pTCPClientSignal->SignalEvent(_fd, ENE_CONNECTED);  
        }  
          
        if(events & (BEV_EVENT_READING | BEV_EVENT_WRITING | BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))  
        {  
             _pTCPClientSignal->SignalEvent(_fd, ENE_CLOSE);  
        }  
          
    }  
}  


