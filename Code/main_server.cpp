#include "Server.h"

struct Info send_info, recv_info;
//接收充电桩生成的充电详单并返回给对应的用户
//监测充电桩空闲状态并进行叫号、调度
int recvCostTable(Server* server) {
    for (;;)
    {

        //接收到充电详单
        if (ChargeTableHead->isAvailable)
        {
            for (;;)
            {
                if (ChargeTablelock.try_lock())
                {
                    break;
                }
            }
            ChargeTablePool* next = ChargeTableHead->next;
            server->sendDetail(next); //向对应的用户发送充电详单并发起扣费信息
            delete ChargeTableHead;
            ChargeTableHead = next;
            ChargeTablelock.unlock();
        }

        //监测到充电区有空位且等候区对应模式有请求则进行调度
        string fUser = "", tUser = "";
        int fID = -1, tID = -1, fTime = INT32_MAX, tTime = INT32_MAX;
        server->getFreeCP(fID, tID, fTime, tTime); //获取不同模式下等待时间最短的充电桩ID
        server->Calling(fUser, tUser);             //获取即将被叫号的用户
        if (fID != -1)
        {
            auto it = server->WUser.find(fUser);
            if (it == server->WUser.end())
            {
                cout << "[fatal Error]: 无法在等候区找到用户" << fUser << endl;
                return -1;
            }
            //用户进入充电区
            server->CUser[fUser] = it->second;
            server->WUser.erase(it);
            //叫号，返回给用户充电桩调度结果，用户进入充电区
            send_info.cmd = CALL;
            strcpy(send_info.UID, fUser.c_str());
            send_info.REPLY = fID;
            string res = "请用户<" + fUser + ">进入充电区！充电桩编号为" + to_string(fID) + "\n";
            cout << res;
            strcpy(send_info.output, res.c_str());
            server_sock.Send(send_info);
            //向充电桩发送请求
            server->forwardRequet(fUser, fID);
        }

        if (tID != -1)
        {
            auto it = server->WUser.find(tUser);
            if (it == server->WUser.end())
            {
                cout << "[fatal Error]: 无法在等候区找到用户" << fUser << endl;
                return -1;
            }
            //用户进入充电区
            server->CUser[tUser] = it->second;
            server->WUser.erase(it);
            send_info.cmd = CALL;
            strcpy(send_info.UID, tUser.c_str());
            send_info.REPLY = tID;
            string res = "请用户<" + tUser + ">进入充电区！充电桩编号为" + to_string(tID) + "\n";
            cout << res;
            strcpy(send_info.output, res.c_str());
            server_sock.Send(send_info);
            //向充电桩发送请求
            server->forwardRequet(tUser, tID);
        }
    }
}

int main() {
    Server server;
    //新建线程读取充电桩返回的充电详单并进行调度
    thread withChargePort(recvCostTable, &server);
    withChargePort.detach();
    for (;;)
    {
        server_sock.Recv(recv_info);
        server.replyClient(recv_info);
    }
    return 0;
}



