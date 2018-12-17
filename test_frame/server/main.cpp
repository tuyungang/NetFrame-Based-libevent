//借鉴了google的一个开源项目里面的sigslot机制
#include <glog/logging.h>  
#include <map>  
#include "NetFrame.h"  
#include "ServerWorker.h"  
#include "PassiveTCPClient.h"  
#include "ActiveTCPClient.h"  
#include "NetSignal.h"  
#include "RWLock.h"  
  
using namespace NAME_SPACE;  
  
// 测试服务器  
class TestServer : public sigslot::has_slots<>, public TCPClientSignal, public TCPServerSignal {  
      
public:  
    TestServer() {  
        pthread_mutex_init(&_work_mutex, nullptr);  
    }  
    ~TestServer() {  
        pthread_mutex_destroy(&_work_mutex);  
    }  
      
    int Start() {  
          
        //_pServerWorker = new ServerWorker("192.168.1.74",8088);  
        _pServerWorker = new ServerWorker("192.168.1.162",8088);  
          
        SignalAccept.connect(this, &TestServer::Accept);  
        SignalAcceptError.connect(this, &TestServer::Event);  
        SignalRecvData.connect(this, &TestServer::RecvData);  
        SignalEvent.connect(this, &TestServer::Event);  
          
        if (!_pServerWorker->StartWork(this)) {  
            LOG(ERROR)<<"服务器监听启动失败";  
            return FUNC_FAILED;  
        }  
          
        return FUNC_SUCCESS;  
    }  
      
    void Stop() {  
          
        _pServerWorker->StopWork();  
          
        pthread_mutex_lock(&_work_mutex);  
        std::map<SOCKET, PassiveTCPClient*>::iterator it = _map_clients.begin();  
        while (it != _map_clients.end()) {  
            it->second->StopWork();  
            delete it->second;  
            _map_clients.erase(it++);  
        }  
        pthread_mutex_unlock(&_work_mutex);  
          
    }  
      
    int SendData(SOCKET fd, void* data, size_t len) {  
          
        pthread_mutex_lock(&_work_mutex);  
        std::map<SOCKET, PassiveTCPClient*>::iterator it = _map_clients.find(fd);  
        if (it != _map_clients.end()) {  
            it->second->SendData(data, len);  
        }  
        pthread_mutex_unlock(&_work_mutex);  
          
        return 0;  
    }  
      
public:  
    // 数据接收  
    void RecvData(SOCKET fd, void* data, size_t len) {  
        // 接收到数据就回显，正常的程序师丢到队列里面去，让其他线程来处理  
        SendData(fd, data, len);  
    }  
  
    // 套接字事件处理器  
    void Event(SOCKET fd, EM_NET_EVENT msg) {  
          
        LOG(ERROR)<<"收到事件通知."<< msg;  
          
        pthread_mutex_lock(&_work_mutex);  
        std::map<SOCKET, PassiveTCPClient*>::iterator it = _map_clients.find(fd);  
        if (it != _map_clients.end()) {  
            it->second->StopWork();  
            delete it->second;  
            _map_clients.erase(it);  
        }  
        pthread_mutex_unlock(&_work_mutex);  
          
    }  
  
    // 客户端连接触发器  
    void Accept(SOCKET fd, struct sockaddr_in* sa) {  
          
        LOG(ERROR)<<"收到客户端连接.";  
          
        pthread_mutex_lock(&_work_mutex);  
          
        std::map<SOCKET, PassiveTCPClient*>::iterator it = _map_clients.find(fd);  
        if (it != _map_clients.end()) {  
            it->second->StartWork(this);  
            delete it->second;  
            _map_clients.erase(it);  
        }  
         
        PassiveTCPClient* pPassiveTCPClient = new PassiveTCPClient(fd, sa, 15);  
        if (!pPassiveTCPClient->StartWork(this)) {  
            LOG(ERROR)<<"启动客户端失败";  
        } else {  
            _map_clients[fd] = pPassiveTCPClient;  
        }  
          
        pthread_mutex_unlock(&_work_mutex);  
    }  
      
private:  
    ServerWorker* _pServerWorker;  
      
    pthread_mutex_t _work_mutex;  
    std::map<SOCKET, PassiveTCPClient*> _map_clients;  
};  
  
int main(int argc,char* argv[]) {  
      
    // 初期化网络  
    if (NetFrame::Instance()->NetWorkInit() != FUNC_SUCCESS) {  
        LOG(ERROR)<<"网络初期化失败....";  
        return -1;  
    }  
      
    {  
        // 测试服务器  
        TestServer mTestServer;  
        mTestServer.Start();  
        sleep(200);// 模拟测试，休眠10分钟时间来测试整个网络库  
        mTestServer.Stop();  
    }  
    
    // 关闭网络  
    NetFrame::Instance()->NetWorkExit();  
      
    return 0;  
}  
