#include "Server.h"
#include<queue>

struct Info send_info, recv_info;
std::mutex serverlock;       //服务器操作互斥锁
std::mutex serverlock2;       //服务器操作互斥锁
queue<struct Info>toCope;

bool server_free = false;
//接收充电桩生成的充电详单并返回给对应的用户
int recvCostTable(Server* server) {
    for (;;)
    {
        //cout << "当前线程在监听充电桩回应...." << endl;
        //接收到充电详单
        if (ChargeTableHead&&ChargeTableHead->isAvailable)
        {
            for (;;)
            {
    
                if (ChargeTablelock.try_lock())
                {
                    break;
                }
            }
            ChargeTablePool* next = ChargeTableHead->next;
           // serverlock.lock();
            server->sendDetail(next); //向对应的用户发送充电详单并发起扣费信息
         //   serverlock.unlock();
            delete ChargeTableHead;
            ChargeTableHead = next;
            ChargeTablelock.unlock();
        }

        //监测到充电区有空位且等候区对应模式有请求则进行调度
        string fUser = "", tUser = "";
        int fID = -1, tID = -1, fTime = INT32_MAX, tTime = INT32_MAX;

        server->getFreeCP(fID, tID, fTime, tTime); //获取不同模式下等待时间最短的充电桩ID
        if (fID == -1 && tID == -1) {
            Sleep(1000);
            continue;
        }

        server->Calling(fUser, tUser);             //获取即将被叫号的用户
        if (fUser == "" && tUser == "") {
            continue;
        }

        cout << "lock3" << endl;
        if (fID != -1 && fUser != "")
        {
            string res = "当前可用的快充充电桩：" + to_string(fID) + "\n预计等待时间：" + to_string(fTime) + "s\n";
            auto it = server->WUser.find(fUser);
            if (it == server->WUser.end())
            {
                it = server->FUser.find(fUser);
                if (it == server->FUser.end())
                    cout << "[fatal Error]: 无法在等候区和故障队列中找到用户" << fUser << endl;
                return -1;
            }
            //用户进入充电区
            server->CUser[fUser] = it->second;
            if (server->WUser[fUser])
                server->WUser.erase(it);
            else
                server->FUser.erase(it);
            server->CUserID[fUser] = fID;
            //叫号，返回给用户充电桩调度结果，用户进入充电区
            send_info.cmd = CALL;
            strcpy(send_info.UID, fUser.c_str());
            send_info.REPLY = fID;
            res += "请用户<" + fUser + ">进入充电区！充电桩编号为" + to_string(fID) + "\n";
            cout << res;
            strcpy(send_info.output, res.c_str());
            server_sock.Send(send_info);
            if (fUser == "V2")
                cout << 1 << endl;
            server->forwardRequet(fUser, fID);
            cout << "请求已经转发给充电桩。。。" << endl;
        }

        if (tID != -1 && tUser != "")
        {
            string res = "当前可用的慢充充电桩：" + to_string(tID) + "\n预计等待时间：" + to_string(tTime) + "s\n";
            auto it = server->WUser.find(tUser);
            if (it == server->WUser.end())
            {
                it = server->FUser.find(tUser);
                if (it == server->FUser.end())
                    cout << "[fatal Error]: 无法在等候区和故障队列中找到用户" << tUser << endl;
                return -1;
            }
            //用户进入充电区
            server->CUser[tUser] = it->second;
            if (server->WUser[tUser])
                server->WUser.erase(it);
            else
                server->FUser.erase(it);
            server->CUserID[tUser] = tID;
            send_info.cmd = CALL;
            strcpy(send_info.UID, tUser.c_str());
            send_info.REPLY = tID;
            res += "请用户<" + tUser + ">进入充电区！充电桩编号为" + to_string(tID) + "\n";
            cout << res;
            strcpy(send_info.output, res.c_str());
            server_sock.Send(send_info);

            //向充电桩发送请求
            server->forwardRequet(tUser, tID);
        }
    }
}



int copeMsg(Server* server) {
    for (;;) {
       // cout << "当前线程在处理消息队列...." << endl;
        if (toCope.size()) {
            server->replyClient(toCope.front());
            
            toCope.pop();
        }
   }
    return 0;
}


int main() {
    
   /* cout << "服务器是否为第一次上线：";
    int res;
    cin >> res;*/

    //服务器初始化
    Server server(0);

    BuildChargePortThread();

    //新建线程读取充电桩返回的充电详单
    thread withChargePort(recvCostTable, &server);
    withChargePort.detach();
  
   
    thread cope(copeMsg, &server);
    cope.detach();

    for (;;)
    {
        server_sock.Recv(recv_info);
        cout << "[客户端请求]--";
        cout << "cmd: " << recv_info.cmd << endl;
        //cmd:300 断开连接
        if (recv_info.cmd == CLOSE)
        {
            server_sock.Close();
            break;
        }
        //将收到的消息放入消息处理队列
        toCope.push(recv_info);
        
    }
    return 0;
}


