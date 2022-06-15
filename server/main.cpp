#include "Server.h"

struct Info send_info, recv_info;
//���ճ��׮���ɵĳ���굥�����ظ���Ӧ���û�
//�����׮����״̬�����нкš�����
int recvCostTable(Server* server) {
    for (;;)
    {

        //���յ�����굥
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
            server->sendDetail(next); //���Ӧ���û����ͳ���굥������۷���Ϣ
            delete ChargeTableHead;
            ChargeTableHead = next;
            ChargeTablelock.unlock();
        }


        //��⵽������п�λ�ҵȺ�����Ӧģʽ����������е���
        string fUser = "", tUser = "";
        int fID = -1, tID = -1, fTime = INT32_MAX, tTime = INT32_MAX;

        server->getFreeCP(fID, tID, fTime, tTime); //��ȡ��ͬģʽ�µȴ�ʱ����̵ĳ��׮ID
        if (fID == -1 && tID == -1)
            continue;

        server->Calling(fUser, tUser);             //��ȡ�������кŵ��û�
        if (fUser == "" && tUser == "")
            continue;
        
        if (fID != -1&&fUser!="")
        {
            string res = "��ǰ���õĿ����׮��" + to_string(fID) + "\nԤ�Ƶȴ�ʱ�䣺" + to_string(fTime) + "s\n";            
            auto it = server->WUser.find(fUser);
            if (it == server->WUser.end())
            {
                cout << "[fatal Error]: �޷��ڵȺ����ҵ��û�" << fUser << endl;
                return -1;
            }
            //�û���������
            server->CUser[fUser] = it->second;
            server->WUser.erase(it);
            server->CUserID[fUser] = fID;
            //�кţ����ظ��û����׮���Ƚ�����û���������
            send_info.cmd = CALL;
            strcpy(send_info.UID, fUser.c_str());
            send_info.REPLY = fID;
            res += "���û�<" + fUser + ">�������������׮���Ϊ" + to_string(fID) + "\n";
            cout << res;
            strcpy(send_info.output, res.c_str());
            server_sock.Send(send_info);
            //����׮��������
            server->forwardRequet(fUser, fID);
        }

        if (tID != -1&&tUser!="")
        {
            string res = "��ǰ���õ�������׮��" + to_string(tID) + "\nԤ�Ƶȴ�ʱ�䣺" + to_string(tTime) + "s\n";
            auto it = server->WUser.find(tUser);
            if (it == server->WUser.end())
            {
                cout << "[fatal Error]: �޷��ڵȺ����ҵ��û�" << tUser << endl;
                return -1;
            }
            //�û���������
            server->CUser[tUser] = it->second;
            server->WUser.erase(it);
            server->CUserID[tUser] = tID;
            send_info.cmd = CALL;
            strcpy(send_info.UID, tUser.c_str());
            send_info.REPLY = tID;
            res += "���û�<" + tUser + ">�������������׮���Ϊ" + to_string(tID) + "\n";
            cout << res;
            strcpy(send_info.output, res.c_str());
            server_sock.Send(send_info);
            //����׮��������
            server->forwardRequet(tUser, tID);
        }
    }
}

int main() {
    
    cout << "�������Ƿ�Ϊ��һ�����ߣ�";
    int res;
    cin >> res;

    cout << "�Ƿ��ǹ��ϣ�������ϳ��׮��ţ���";
    int PID;
    cin >> PID;

    //��������ʼ��
    Server server(res,PID);

    BuildChargePortThread();

    //�½��̶߳�ȡ���׮���صĳ���굥�����е���
    thread withChargePort(recvCostTable, &server);
    withChargePort.detach();
    for (;;)
    {
        server_sock.Recv(recv_info);
        cout<<"���յ��ͻ��˵�����"<<endl;
        cout << "cmd: " << recv_info.cmd << endl;
        server.replyClient(recv_info);

    }
    return 0;
}



