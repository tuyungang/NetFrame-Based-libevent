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
  
// 测试客户端  
class TestClient : public sigslot::has_slots<>, public TCPClientSignal, public Runnable {  
      
public:  
    TestClient():_is_run_flg(false) {  
    }  
      
    ~TestClient() {  
    }  
      
    int Start() {  
        //_pActiveTCPClient = new ActiveTCPClient("192.168.1.5", 8088, 15);  
        _pActiveTCPClient = new ActiveTCPClient("127.0.0.1", 8088, 15);  
        _pActiveTCPClient->SetTCPClientSignal(this);  
  
        SignalEvent.connect(this, &TestClient::Event);  
        SignalRecvData.connect(this, &TestClient::RecvData);  
          
        _is_run_flg = true;  
        //if (!_connect_thread.Start(this))   //modify by tu
        if (!_pActiveTCPClient->StartWork()) {  
            _is_run_flg = false;  
            delete _pActiveTCPClient;  
            return FUNC_FAILED;  
        }  
          
        return FUNC_SUCCESS;  
    }  
      
    void Stop() {  
          
        if (_pActiveTCPClient) {  
              
            _is_run_flg = false;  
            //_connect_thread.Stop(); //modify by tu  
            _pActiveTCPClient->StopWork();  
              
            SignalEvent.disconnect(this);  
            SignalRecvData.disconnect(this);  
              
            _pActiveTCPClient->StopWork();  
              
            delete _pActiveTCPClient;  
            _pActiveTCPClient = nullptr;  
        }  
    }  
      
    int SendData(void* data,size_t len) {  
          
        if (_pActiveTCPClient) {  
            _pActiveTCPClient->SendData(data, len);  
        }  
        return FUNC_SUCCESS;  
    }  
      
    // 数据接收  
    void RecvData(SOCKET fd, void* data, size_t len) {  
        // 接收到数据就回显，正常的程序师丢到队列里面去，让其他线程来处理  
        SendData(data, len);  
    }  
      
    // 套接字事件处理器  
    void Event(SOCKET fd, EM_NET_EVENT msg) {  
          
        if (msg == ENE_CONNECTED) {  
        } else {  
            _pActiveTCPClient->StopWork();  
        }  
          
    }  
      
protected:  
    virtual void Run(void* arg) {  
          
        //TestClient* p = (TestClient*)arg;  
          
        while (_is_run_flg) {  
              
            if (!_pActiveTCPClient->IsConnect()) {  
                _pActiveTCPClient->StartWork();  
            }  
              
            Thread::SleepMs(2000);  
        }  
    }  
      
private:  
    ActiveTCPClient* _pActiveTCPClient;  
    // 运行标志  
    volatile bool _is_run_flg;  
    // 连接检测线程  
    Thread _connect_thread;  
      
};  
  
int main(int argc,char* argv[]) {  
      
    // 初期化网络  
    if (NetFrame::Instance()->NetWorkInit() != FUNC_SUCCESS) {  
        LOG(ERROR)<<"网络初期化失败....";  
        return -1;  
    }  
      
    {  
        // 测试客户端  
        TestClient mTestClient;  
        mTestClient.Start();  
        char buf[4] = "bye";  
        for (int i = 0; i < 200; ++i) {  
            memset(buf, 0x00, sizeof(buf));  
            sprintf(buf, "%03d", i);  
            sleep(1);  
            LOG(INFO)<<buf;  
            mTestClient.SendData(buf, 3);  
            //LOG(INFO)<<buf;  
        }  
          
        mTestClient.Stop();  
    }  
      
    // 关闭网络  
    NetFrame::Instance()->NetWorkExit();  
      
    return 0;  
}  

