
#include "Server.h"
extern struct Info send_info, recv_info;
/*Server��ʵ��*/

//��������ʼ��
Server::Server(int res) {
    //���ݿ��Զ�ͬ��������<���ݿ⹹�캯���Ѿ����>

    //��һ�����ߣ���������ʷ��Ϣ
    if (res == 0) {
        //��������ʼ�����׮
        for (int i = 0; i < CHARGEPORT_NUM; i++)
        {
            //���
            if (i < FAST_NUM)
                cData.push_back(new ChargePort(i, true, true, MAX_WAIT_NUM));
            else
                cData.push_back(new ChargePort(i, false, true, MAX_WAIT_NUM));
        }
    }

    //����������׮��ʷ��Ϣ
    else {
        ifstream hy;
        hy.open(HISTORY, ios::in | ios::out);
        if (hy.is_open()) {
            for (int i = 0; !hy.eof() && i < CHARGEPORT_NUM; i++) {
                //���׮д����ʷ��Ϣ
                int CCnt = -1;
                double CCost = -1, ECost = -1;
                long long CTime = -1;
                double TElect = -1;
                double SCost = -1;
                hy >> CCnt >> CCost >> ECost >> CTime >> TElect >> SCost;
                //���
                if (i < FAST_NUM)
                    cData.push_back(new ChargePort(i, true, true, MAX_WAIT_NUM, CCnt, CCost, ECost, CTime, TElect, SCost));
                //����
                else
                    cData.push_back(new ChargePort(i, false, true, MAX_WAIT_NUM, CCnt, CCost, ECost, CTime, TElect, SCost));
            }
        }
        else {
            cout << "[FATAL ERROR]: history file " << HISTORY << " open failure" << endl;
        }
    }

    //��ʼ���Ŷ���Ϣ
    FNum = TNum = 0;
    cout << "��������ʼ����ɣ�" << endl;
}

//����������ǰͬ�����ݿ�
Server::~Server()
{
    database.update();
}

/*database Interface*/

//�û���Ϣ��ѯ---ok
int Server::usrFind(string usrname, usrEntry* res)
{
    res= database.usrData[usrname];
    //�û�������
    if (!res )
    {
        cout << "[From Database]: �û�"<<usrname<<"�����ڣ�" << endl;
        return -1;
    }
    return 0;
}

//�û�����¼��ѯ
int Server::logFind(string usrname, vector<logEntry*>& res)
{
    auto it = database.logData.find(usrname);
    if (it == database.logData.end())
    {
        cout << "the usr " << usrname << " has no charge log currently." << endl;
        return -1;
    }
    res.assign(it->second.begin(), it->second.end());
    return 0;
}

//�û���Ϣ����<�����û����û���Ϣ�ı䣬�û�ע��>--ok
int Server::usrDataUpdate(bool to_delete, usrEntry* uE)
{
    if (!uE) {
        cout << "[fatal error]:uEΪ��" << endl;
        return -1;
    }

    //ע��
    if (to_delete)
    {
        //��ע�����û���������
        if (database.usrData[uE->usrname] == nullptr)
        {
            cout << "��ע�����˻�<" << uE->usrname << ">�����ڣ�" << endl;
            return 0;
        }
        database.usrData.erase(uE->usrname);
        cout << "�˻�<" << uE->usrname << ">ע���ɹ���" << endl;
        return 0;
    }

    //ע�� or �û���Ϣ�ı�
    if(database.usrData[uE->usrname])
        database.usrData.erase(uE->usrname);
    database.usrData[uE->usrname] = uE;
    return 0;
}


//���û����Ͷ�Ӧ�ĳ���굥�����п۷�--ok
void Server::sendDetail(ChargeTablePool* next)
{
    send_info.cmd = DETAIL;
    strcpy_s(send_info.UID, next->ChargeTable.usrname.c_str());

    string res = "<<<<<<<<<<<<<<<<<<<<����굥>>>>>>>>>>>>>>>>>>>>";
    res += "�굥���: " + to_string(next->ChargeTable.ChargeID) + "\n";
    res += "�굥����ʱ��: " + getCurTime(next->ChargeTable.CreateTableTime) + "\n";
    res += "���׮���: " + to_string(next->ChargeTable.SID) + "\n";
    res += "���ģʽ: ";
    if (next->ChargeTable.IsFastCharge)
        res += "FAST\n";
    else
        res += "SLOW\n";
    res += "�û���:" + next->ChargeTable.usrname + "\n";
    res += "����ʱ��:" + getCurTime(next->ChargeTable.StartTime) + "\n";
    res += "ֹͣʱ��:" + getCurTime(next->ChargeTable.End_Time) + "\n";
    res += "�ܳ����:" + to_string(next->ChargeTable.TotalElect) + "\n";
    res += "���ʱ��:" + to_string(next->ChargeTable.ChargeTime) + "s\n";
    res += "�����:" + to_string(next->ChargeTable.ServiceCost) + "\n";
    res += "��ѣ�" + to_string(next->ChargeTable.ElectCost) + "\n";
    res += "�ܷ���:" + to_string(next->ChargeTable.ChargeCost) + "\n";
    res += "--------------------------------------------------------\n";
    //�۷�
    balanceChange(next->ChargeTable.usrname, -next->ChargeTable.ChargeCost);
    send_info.BALANCE = database.usrData[next->ChargeTable.usrname]->balance;
    res += "<�۷ѳɹ�>:��ǰ��" + to_string(send_info.BALANCE) + "Ԫ\n";
    CUser.erase(next->ChargeTable.usrname);
    CUserID.erase(next->ChargeTable.usrname);
    queueData.erase(next->ChargeTable.usrname);
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
    server_sock.Send(send_info);
}

//������������׮ת��������󣨳������--ok
void Server::forwardRequet(string usrname, int SID)
{
    CarReply reply;
    string qNum = queueData[usrname].first;
    CarAsk* ask = this->CUser[usrname];
    reply.Ask = *(ask);
    reply.SID = SID;
    reply.num = cData[SID]->IsCharging; //��ǰ�ŶӶ��б�� 0 or 1
    reply.MODE = qNum[0];               //���ģʽF or T
    reply.queueNum = qNum;              //�ŶӺ���
    reply.waitingNum = reply.num;       //ǰ���ȴ�����0 or 1

    Car* car=new Car();
    car->usrname = usrname;
    car->BatteryCap = ask->BatteryCap;
    car->BatteryNow = ask->BatteryNow;
    car->Ask = ask;
    car->Reply = &reply;

    //ת���������
    cData[SID]->AddCar(reply, car);
}

/*with Client*/

//��Ӧ�û��ͻ�������
int Server::replyClient(Info usrInfo)
{
    string Usrname = string(usrInfo.UID);
    usrEntry* uE = database.usrData[Usrname];
    string qNum,role;
    switch (usrInfo.cmd)
    {
        // cmd:100 ��¼��֤
    case LOG_IN:
        logIn(Usrname, string(usrInfo.PWD), uE);
        break;

        // cmd:101 ע����֤
    case SIGN_UP:
        role = (usrInfo.MODE == 1) ? "customer" : "admin";
        signUp(Usrname, string(usrInfo.PWD), role);
        database.update();
        break;

        // cmd:102 ��ֵ���۷�
    case Balance_CHANGE:
        balanceChange(Usrname, usrInfo.BALANCE);
        database.update();
        break;

        // cmd:103 ע��
    case DELETE_USER:
        deleteUsr(Usrname);
        database.update();
        break;

        // cmd: 104 �������
    case CHARGE_REQUEST: 
    {
        CarAsk* ask = new CarAsk();
        resolveRequest(usrInfo, ask);
        if (ask ==nullptr)
            cout << "null!" << endl;
        copeChargeRequest(ask);
    }
        break;

        // cmd: 105 ȡ�����
    case CANCEL_REQUEST:
        cancelCharge(Usrname);
        break;

        // cmd: 106 ��ȡ�Ŷ���Ϣ
    case GET_QUEUE_DATA:
        int curWait;
        getQueueData(Usrname, qNum, curWait);
        break;

        //cmd: 200 ��ȡ���׮״̬��Ϣ(ADMIN)
    case SPY_CHARGEPORT:
        getChargePortData(Usrname);
        break;

        //cmd:201 �鿴�Ⱥ���׮����ĳ�����Ϣ(ADMIN)
    case WAIT_CAR_DATA:
        getCarData(Usrname);
        break;

        //cmd:202 �鿴����ADMIN)
    case DATA_STATISTICS:
        getReport(Usrname);
        break;

        //cmd:203 �������׮
    case OPEN_CHARGEPORT:
        openCP(Usrname, usrInfo.REPLY);
        break;

        //cmd:204 �رճ��׮
    case CLOSE_CHARGEPORT:
        closeCP(Usrname, usrInfo.REPLY);
        break;
    default:break;
    }
   
    return 0;
}

// cmd:100 ��¼��֤,��¼�ɹ����ȡ�û���Ϣok
int Server::logIn(string usrname, string passwd, usrEntry* uE)
{
    strcpy_s(send_info.UID, usrname.c_str());
    send_info.cmd = LOG_IN;
    cout << "��¼��֤..." << endl;
    string res;
    //�����û��Ƿ����
    if (usrFind(usrname, uE) != 0)
    {
        res = "�û���<" + usrname + ">�����ڣ�" + "\n";
        cout << res << endl;
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        return -1;
    }
    //�ж������Ƿ���ȷ
    else if (passwd != uE->passwd)
    {
        res = "�û��������벻ƥ�䣡\n";
        cout << res;
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -2;
        server_sock.Send(send_info); 
        return -2;
    }
    //��¼�ɹ�
    else
    {
        res = "��¼�ɹ���\n";
        cout << res;
        send_info.MODE = 1;
        if (uE->role == "admin")
            send_info.MODE = 2;
        strcpy_s(send_info.output, res.c_str());
        send_info.BALANCE = uE->balance;
        send_info.REPLY = 0;
        server_sock.Send(send_info);
        return 0;
    }
}

// cmd:101 ע����֤ok
int Server::signUp(string usrname, string passwd, string role)
{
    cout << "[ע����֤] : ";
    strcpy_s(send_info.UID, usrname.c_str());
    send_info.cmd = SIGN_UP;
    string res;
    //�û������ȷǷ�
    if (usrname.size() > 20 || usrname.size() < 2||usrname.find(" ")!=string::npos)
    {
        res = "�û������ȱ�����5~20֮��,�Ҳ��ܰ����ո�!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        cout << res;
        return -1;
    }
    //�û����Ѵ���
    else if (database.usrData[usrname])
    {
        res = "�û����Ѵ��ڣ�\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -2;
        server_sock.Send(send_info);
        cout << res;
        return -2;
    }

    usrEntry* uE = new usrEntry();
    cout << usrname << " " << "ע��ɹ�!" << endl;
    uE->usrname = usrname;
    uE->passwd = passwd;
    uE->role = role;
    uE->balance = 0;
    usrDataUpdate(false, uE);
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
  
    server_sock.Send(send_info);
    return 0;
}

// cmd:102 ��ֵ���۷ѣ�amountΪ����ʾ��ֵ�����Ϊ����ʾҪ�۳��Ľ�---ok
int Server::balanceChange(string usrname, int amount)
{
    strcpy_s(send_info.UID, usrname.c_str());
    usrEntry* uE= database.usrData[usrname];
    string res;

    //��ֵ
    if (amount > 0)
    {
        uE->balance = amount;
        res = "<" + usrname + "��ֵ�ɹ�>: ";
        res += "��ǰ���Ϊ" + to_string(uE->balance) + "Ԫ!\n";
        cout << res;
    }
    //�۷�
    if (amount < 0)
    {
        uE->balance += amount;
        res = "<" + usrname + "�۷ѳɹ�>: ";
        res += "��ǰ���Ϊ" + to_string(uE->balance) + "Ԫ!\n";
        cout << res;
    }
    return 0;
}

// cmd:103 �û�ע��ok
int Server::deleteUsr(string usrname)
{
    usrEntry* uE=new usrEntry();
    uE->usrname = usrname;
    send_info.cmd = DELETE_USER;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "�û�<" + usrname + ">ע���ɹ�!\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    cout << res;
    return Server::usrDataUpdate(true, uE);
}

// cmd:104 ����Ⱥ����ڳ����ĳ������ok
int Server::copeChargeRequest(CarAsk* ask)
{
    strcpy_s(send_info.UID, ask->usrname.c_str());
    send_info.cmd = CHARGE_REQUEST;
    send_info.MODE = ask->IsFastCharge;
    string res;
    //��������޷��ύ���޸��û�����
    if (CUser[ask->usrname])
    {
        res = "������ǰλ�ڳ�������޷��޸ĳ������";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        cout << res;
        return -1;
    }
    else
    {
        CarAsk* w = WUser[ask->usrname];
        //���û���һ���ύ�������
        if (w == nullptr)
        {
            //���Ⱥ����������޷���������
            if (WUser.size() > MAX_WAIT_NUM)
            {
                res = "��ǰ�Ⱥ����������޷������µ��û�����\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.REPLY = -2;
                server_sock.Send(send_info);
                cout << res;
                return -2;
            }
            //���������ŶӺ��벢��¼ǰ���ȴ���Ŀ
            else
            {
                double estCost=ask->BatteryCap*(SERVICE_PRICE+ELEC_PRICE);
                if (database.usrData[ask->usrname]->balance < estCost) {
                    send_info.REPLY = -3;
                }

                reqRes[ask->usrname] = false;
                WUser[ask->usrname] = ask;
                string qNum = queueNumGenerate(ask->usrname, ask->IsFastCharge);
                queueData[ask->usrname].first = qNum;
                int curWait = getCurWaitNum(ask->usrname); //��ȡǰ���ȴ���Ŀ
                queueData[ask->usrname].second = curWait;
                res = "����ɹ���\n��ǰ�ŶӺ��룺" + qNum + "\n��ģʽ��ǰ���ȴ������� " + to_string(curWait) + "\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.REPLY = 0;
                server_sock.Send(send_info);
                cout << res;
                reqRes[ask->usrname] = true;
                return 0;
            }
        }
        //���û��ύ�������򸲸�ԭ������
        else
        {
            //���޸��˳��ģʽ�����������ŶӺ���
            if (w->IsFastCharge != ask->IsFastCharge)
            {
                reqRes[ask->usrname] = false;
                queueData.erase(ask->usrname);
                //���������ŶӺ����ǰ���ȴ�����
                string qNum = queueNumGenerate(ask->usrname, ask->IsFastCharge);
                this->queueData[ask->usrname].first = qNum;
                int curWait = getCurWaitNum(ask->usrname);
                this->queueData[ask->usrname].second = curWait;
                WUser[ask->usrname] = ask;
                res = "�����޸ĳɹ���\n��ǰ�ŶӺ��룺" + qNum + "\n��ģʽ��ǰ���ȴ������� " + to_string(curWait) + "\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.REPLY = 0;
                server_sock.Send(send_info);
                cout << res;
                reqRes[ask->usrname] = true;
                return 0;
            }
            //�����ŶӺŲ���,����ԭ������
            else
            {
                reqRes[ask->usrname] = false;
                WUser[ask->usrname] = ask;
                res = "�����޸ĳɹ���\n��ǰ�ŶӺ��룺" + queueData[ask->usrname].first + "\n��ģʽ��ǰ���ȴ������� " + to_string(queueData[ask->usrname].second) + "\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.REPLY = 0;
                server_sock.Send(send_info);
                cout << res;
                reqRes[ask->usrname] = true;
                return 0;
            }
        }
    }
}

// cmd:105 ȡ�����--ok
int Server::cancelCharge(string usrname)
{
    send_info.cmd = CANCEL_REQUEST;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;
    auto qData = queueData.find(usrname);

    //��ǰ�û�û��δ����ĳ������
    if (qData == queueData.end())
    {
        res = "δ�ҵ��û�" + usrname + "�ĳ������!" + "\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        cout << res;
        return -1;
    }

    //��ǰ�û��ڵȺ�������ɾ����������뿪�Ⱥ���
    if (WUser.find(usrname) != WUser.end())
    {
        queueData.erase(usrname);
        WUser.erase(usrname);
        res = "ȡ�����ɹ������������ɾ����\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = 0;
        server_sock.Send(send_info);
        cout << res;
        return 0;
    }

    CarAsk* cU = CUser[usrname];
    int num = CUserID[usrname];
    if (!cU) {
        res = "���ȡ��ʧ�ܣ����û��Ȳ��ڳ����Ҳ���ڵȺ�����\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = 0;
        server_sock.Send(send_info);
        cout << res;
        return -2;
    }
    
    //�����ڳ��(ǰ���ȴ�����Ϊ0)����ǰ������� 
     if (cData[num]->ChargingCar->usrname==usrname){
        //ֹͣ��磬�����굥,�뿪�����
         int num = CUserID[usrname];
         cData[num]->DeleteCar(cData[num]->ChargingCar);
         CUser.erase(usrname);
         queueData.erase(usrname);
         CUserID.erase(usrname);
         return 0;
     }
    
     //�����ڳ�������ڵȴ�״̬��ӵȺ�������Ƴ���ɾ����������뿪�����
     for (int k = 0; k < cData[k]->WaitingCar.size(); k++) {
         if (cData[num]->WaitingCar[k]->usrname == usrname) {
             cData[num]->DeleteCar(cData[num]->WaitingCar[k]);
             CUser.erase(usrname);
             queueData.erase(usrname);
             CUserID.erase(usrname);
             break;
         }
     }
    res = "���ȡ���ɹ���\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    cout << res;
    return 0;
}

// cmd:106 ��ȡ�Ŷ���Ϣ--ok
int Server::getQueueData(string usrname, string& qNum, int& curWait)
{
    send_info.cmd = GET_QUEUE_DATA;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;

    auto it = queueData.find(usrname);
    if (it == queueData.end())
    {
        send_info.Q_NUM = -1;
        res = "δ�ҵ��û�<" + usrname + ">�ĳ������\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        cout << res;
        return -1;
    }
    qNum = it->second.first;
    curWait = getCurWaitNum(usrname);
    it->second.second = curWait;
    send_info.MODE = (qNum[0] == 'F') ? 1 : 0;
    send_info.Q_NUM = stoi(qNum.substr(1));
    send_info.W_NUM = curWait;
    res = "�ŶӺ��룺" + qNum + "ǰ���ȴ�����" + to_string(curWait) + "\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    cout << res;
    return 0;
}

//�����ŶӺ�������--ok
string Server::queueNumGenerate(string usrname, int mode)
{
    string qNum;
    if (mode == FAST)
        qNum = "F" + to_string(++FNum);
    else
        qNum = "T" + to_string(++TNum);
    queueData[usrname] = make_pair(qNum, -1);
    return qNum;
}

//��ȡ��ģʽ�����µ�ǰ���ȴ�������Ϣ---ok
int Server::getCurWaitNum(string usrname)
{
    int num = 0;
    auto ask = this->queueData.find(usrname);
    int qNum = stoi(ask->second.first.substr(1)); //�ŶӺ���

    //�������λ�ڵȺ���
    CarAsk* w = WUser[usrname];
    if (w)
    {
        if (w->IsFastCharge)
        {
            //��ȡ�Ⱥ�����ǰ���ȴ���Ŀ
            for (auto i = WUser.begin(); i != WUser.end(); i++)
            {
                if (i->second->IsFastCharge && qNum > stoi(queueData[i->first].first.substr(1)))
                    num++;
            }
            //��ȡ�������ͬģʽ�����еȴ��ĳ�����Ŀ
            for (int k = 0; k < FAST_NUM; k++) {
                num += cData[k]->WaitCount;
            }
           
        }
        else
        {
            //��ȡ�Ⱥ�����ǰ���ȴ���Ŀ
            for (auto i = WUser.rbegin(); i != WUser.rend(); i++)
            {
                if (!i->second->IsFastCharge && qNum > stoi(queueData[i->first].first.substr(1)))
                    num++;
            }
            //��ȡ�������ͬģʽ�����еȴ��ĳ�����Ŀ
            for (int k = FAST_NUM; k < CHARGEPORT_NUM; k++) {
                num += cData[k]->WaitCount;
            }

        }
        return num;
    }

    //�������λ�ڳ����
    CarAsk* c = CUser[usrname];
    if ( c )
    {
        num = 0;
        int k = CUserID[usrname];
        for (int j = 0; j < cData[k]->WaitingCar.size(); j++) {
            string usr = cData[k]->WaitingCar[j]->usrname;
            int cNum = stoi(queueData[usr].first.substr(1));
            if (cNum < qNum)
                num++;
        }
        return num;
    }

    //��������Ⱥ���
    cout << "�û�" << usrname << "�Ȳ��ڵȺ�����Ҳ���ڳ����\n";
    return -1;
}

//��ȡ�Ⱥ����ڼ������������Ŀ�������usrname(�кţ�
int Server::Calling(string& fUser, string& tUser)
{
    fUser = "";
    tUser = "";
    int fast = INT32_MAX, slow = INT32_MAX;
    for (auto i = queueData.begin(); i != queueData.end(); i++)
    {
        int cur = stoi(i->second.first.substr(1));
        if (WUser[i->first]&&reqRes[i->first]&&i->second.first[0] == 'F' && cur< fast)
        {
            fast = cur;
            fUser = i->first;
        }
        if (WUser[i->first] && reqRes[i->first] &&i->second.first[0] == 'T' && cur < slow )
        {
            slow = cur;
            tUser = i->first;
        }
    }
    return 0;
}

//��ȡ��������ŶӶ����п�λ�ҵȴ�ʱ����̵ĳ��׮--ok
int Server::getFreeCP(int& fSID, int& tSID, int& fTime, int& tTime)
{
    //��¼����ģʽ����ʱ����̵ĳ��׮����ʱ��
    fTime = tTime = INT32_MAX;
    fSID = tSID = -1;
    for (int i = 0; i < FAST_NUM; i++) {
        if (cData[i]->WaitCount > MAX_WAIT_NUM)
            continue;//�޿�λ�ĳ��׮����
        int cur = 0;
        for (int j = 0; j < cData[i]->WaitingCar.size(); j++) {
            string usr = cData[i]->WaitingCar[j]->usrname;
             cur+= (CUser[usr]->ChargeCap / FAST_POWER) * 60;
        }
        cur += (cData[i]->CurElectReq / FAST_POWER) * 60;
        if (cur < fTime) {
            fSID = i;
            fTime = cur;
        }
    }
    for (int i = FAST_NUM; i < CHARGEPORT_NUM; i++) {
        if (cData[i]->WaitCount > MAX_WAIT_NUM)
            continue;//�޿�λ�ĳ��׮����
        int cur = 0;
        for (int j = 0; j < cData[i]->WaitingCar.size(); j++) {
            string usr = cData[i]->WaitingCar[j]->usrname;
            cur += (CUser[usr]->ChargeCap / SLOW_POWER) * 60;
        }
        cur += (cData[i]->CurElectReq / SLOW_POWER) * 60;
        if (cur < tTime) {
            tSID = i;
            tTime = cur;
        }
    }
    return 0;
}

//��ȡ�������
int Server::resolveRequest(Info& usrInfo, CarAsk* ask)
{
    if (ask == nullptr) {
        ask = new CarAsk();
    }
    ask->usrname = string(usrInfo.UID);
    ask->ChargeCap = usrInfo.BALANCE;
    ask->IsFastCharge = usrInfo.MODE;
    ask->BatteryCap = usrInfo.BatteryCap;
    ask->BatteryNow = usrInfo.COST; 
    return 0;
}

//cmd200: ��ȡ���׮״̬��Ϣ--ok
int Server::getChargePortData(string usrname) {
    send_info.cmd = SPY_CHARGEPORT;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "<<<<<<<<<<<<<<<<<<<<���״̬��Ϣ>>>>>>>>>>>>>>>>>>>>\n";
    for (int i = 0; i < CHARGEPORT_NUM; i++) {
        res += "���׮��ţ�" + to_string(i) + "\n";
        res += "�Ƿ�Ϊ��䣺" + to_string(cData[i]->IsFastCharge) + "\n";
        res += "�Ƿ�����������" + to_string(1) + "\n";
        res += "�Ƿ��Ѿ�����" + to_string(cData[i]->OnState) + "\n";
        res += "ϵͳ�������ۼƳ�������" + to_string(cData[i]->ChargeCnt) + "\n";
        res += "ϵͳ�������ۼƳ����ʱ����" + to_string(cData[i]->ChargeTime) + "\n";
        res += "ϵͳ�������ۼƳ���ܵ�����" + to_string(cData[i]->TotalElect) + "\n";
        res += "----------------------------------------------------------------------\n";
    }
    send_info.REPLY = 0;
    strcpy_s(send_info.output, res.c_str());
    cout << res << endl;
    server_sock.Send(send_info);
    return 0;
}

//cmd201:�鿴���׮�Ⱥ�ĳ�����Ϣ
int Server::getCarData(string usrname) {
    send_info.cmd = WAIT_CAR_DATA;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "<<<<<<<<<<<<<<<<<<<<���׮�Ⱥ��������Ϣ>>>>>>>>>>>>>>>>>>>>\n";
    for (int i = 0; i < CHARGEPORT_NUM; i++) {
        res += "----------------------------------------------------------------------\n";
        res += "���׮��ţ�" + to_string(i) + "\n";
        res += "�Ƿ�Ϊ��䣺" + to_string(cData[i]->IsFastCharge) + "\n";
        for (int j = 0; j < cData[i]->WaitingCar.size(); j++) {
            res+= to_string(j)+" ) �û���: " +cData[i]->WaitingCar[j]->usrname;
            res+= "\n���������"+to_string(cData[i]->WaitingCar[j]->BatteryCap);
            res+= "\n��ǰ��ص�����"+to_string( cData[i]->WaitingCar[j]->BatteryNow)+"\n";
            res += "�ȴ�ʱ�䣺" + to_string(time(NULL) - cData[i]->WaitingCar[j]->Ask->StWaitTime) + "s\n\n";
        }
        res += "----------------------------------------------------------------------\n\n";
    }
    return 0;
}

//cmd202: �鿴����--ok
int Server::getReport(string usrname) {
    send_info.cmd = DATA_STATISTICS;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "<<<<<<<<<<<<<<<<<<<<��籨��>>>>>>>>>>>>>>>>>>>>\n";
    res += "��ǰʱ�䣺" + getCurTime(0) + "\n";
    for (int i = 0; i < CHARGEPORT_NUM; i++) {
        CPStatusTable report = cData[i]->GetStatus();
        res += "���׮��ţ�" + to_string(i) + "\n";
        res += "�Ƿ�Ϊ��䣺" + to_string(report.IsFastCharge) + "\n";
        res += "�ۼƳ�������" + to_string(report.ChargeCnt+report.PastChargeCnt) + "\n";
        res += "�ۼƳ��ʱ����" + to_string(report.ChargeTime+report.PastChargeTime) + "min\n";
        res += "�ۼƳ������" + to_string(report.TotalElect+report.PastTotalElect) + "��\n";
        res += "�ۼƳ����ã�" + to_string(report.ElectCost+report.PastElectCost) + "Ԫ\n";
        res += "�ۼƷ�����ã�" + to_string(report.ServiceCost+report.PastServiceCost) + "Ԫ\n";
        res += "�ۼ��ܷ��ã�" + to_string(report.ChargeCost+report.PastChargeCost) + "Ԫ\n";
        res += "--------------------------------------------------------------------\n\n";
    }
    strcpy_s(send_info.output, res.c_str());
    cout << res << endl;
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    return 0;
}

//cmd203���������׮
int Server::openCP(string usrname, int SID) {
    usrEntry* uE=nullptr;
    usrFind(usrname, uE);
    send_info.cmd = OPEN_CHARGEPORT;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;
    if (cData[SID]->OnState) {
        res = "����ʧ�ܣ����׮�Ѿ�����!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        cout << res;
        return -1;
    }
    if (uE->role == "admin" && 0 < SID < CHARGEPORT_NUM) {
        cData[SID]->on();
        res = "���׮" + to_string(SID) + "�����ɹ�!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = 0;
        server_sock.Send(send_info);
        cout << res;
        return 0;
    }
    res = "���׮" + to_string(SID) + "����ʧ��\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = -2;
    server_sock.Send(send_info);
    cout << res;
    return -2;
}

//cmd204: �رճ��׮
int Server::closeCP(string usrname, int SID) {
    usrEntry* uE=nullptr;
    usrFind(usrname, uE);
    send_info.cmd = OPEN_CHARGEPORT;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;
    if (cData[SID]->OnState == false) {
        res = "�ر�ʧ�ܣ����׮�Ѿ����ڹر�״̬!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        cout << res;
        server_sock.Send(send_info);
        return -1;
    }
    if (uE->role == "admin" && 0 < SID < CHARGEPORT_NUM) {
        cData[SID]->off();
        res = "���׮" + to_string(SID) + "�رճɹ�!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        cout << res;
        return 0;
    }
    res = "���׮" + to_string(SID) + "�ر�ʧ��\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = -2;
    server_sock.Send(send_info);
    cout << res;
    return -2;
}

string Server::getCurTime(time_t t) {
    struct tm* local_time = NULL;
    time_t s = t == 0 ? time(NULL) : t;
    local_time = localtime(&s);
    char str_time[100];
    strftime(str_time, sizeof(str_time), "%Y-%m-%d:%H:%M:%S", local_time);
    return string(str_time);
}


/*DBupdate��ʵ��*/

//���ݿ��ʼ��
DBupdate::DBupdate()
{

    //�����û��ļ�������������ֱ�Ӵ���
    cout << "�������ݿ�..." << endl;
    ifstream uf;
    uf.open(USER_FILENAME, ifstream::out | ifstream::app);
    if (uf.is_open())
    {
        while (!uf.eof())
        {
            //��ȡ�û���Ŀ
            string line;
            getline(uf, line);
            while (line == "\n" && !uf.eof())
            {
                getline(uf, line);
            }
            //�û���Ŀ����
            usrEntry* uE = new usrEntry();
            if (entryResolve(uE, line) == 0)
            {
                DBupdate::usrData[uE->usrname] = uE;
            }
        }
    }
    else
    {
        cout << "[FATAL ERROR]: user file " << USER_FILENAME << " open failure" << endl;
    }
    uf.close();


    //������־�ļ�������������ֱ�Ӵ���
    ifstream log;
    log.open(LOG_FILENAME, ifstream::out | ifstream::app);
    if (log.is_open())
    {
        while (!log.eof())
        {
            //��ȡ��־��Ŀ
            string line;
            getline(log, line);
            while (line == "\n" && !log.eof())
            {
                getline(log, line);
            }
            //��־��Ŀ����
            logEntry* lE = new logEntry();
            if (entryResolve(lE, line) == 0)
            {
                DBupdate::logData[lE->usrname].push_back(lE);
            }
        }
    }
    else
    {
        cout << "[FATAL ERROR]: log file " << LOG_FILENAME << " open failure" << endl;
    }
    log.close();
    cout << "���ݿ�ͬ�����!" << endl;
}

//���ݿ���Ŀ����(�û���Ŀ)
int DBupdate::entryResolve(usrEntry* uE, string line)
{
    vector<string> temp;
    split(temp, line, ' ');
    if (temp.size() < 4)
    {
        return -1;
    }
    uE->usrname = temp[0];
    uE->passwd = temp[1];
    uE->role = temp[2];
    uE->balance = stoi(temp[3]);
    return 0;
}

//���ݿ���Ŀ��������־��Ŀ��
int DBupdate::entryResolve(logEntry* lE, string line)
{
    vector<string> temp;
    split(temp, line, ' ');
    if (temp.size() < 7)
    {
        return -1;
    }
    lE->start_time = temp[0];
    lE->usrname = temp[1];
    lE->SID = stoi(temp[2]);
    lE->queueNum = temp[3];
    lE->mode = stoi(temp[4]);
    lE->time = stoi(temp[5]);
    lE->cost = stoi(temp[6]);
    return 0;
}

//�����û���Ϣ
int DBupdate::addUser(usrEntry* data)
{
    if (data != nullptr)
    {
        this->usrData[data->usrname] = data;
        return 0;
    }
    return -1;
}

//������־��Ŀ
int DBupdate::addLogEntry(logEntry* data)
{
    if (data != nullptr)
    {
        this->logData[data->usrname].push_back(data);
        return 0;
    }
    return -1;
}

//�������ݿ�
int DBupdate::update()
{

    //�����û��ļ�
    cout << "�������ݿ�..."  ;
    ofstream uf;
    uf.open(USER_FILENAME, ios::trunc | ios::out);
  //  map<string, usrEntry*>::reverse_iterator iter;
    for (auto iter = usrData.begin(); iter != usrData.end(); iter++)
    {
        string line = getEntry(iter->second);
        if(line.size())
            uf << line << "\n";
    }
    uf.close();

    ////���·�����־
    //ofstream log;
    //log.open(LOG_FILENAME, ios::trunc | ios::out);
    //map<string, vector<logEntry*>>::reverse_iterator it;
    //for (it = this->logData.rbegin(); it != this->logData.rend(); it++)
    //{
    //    for (int j = 0; j < it->second.size(); j++)
    //    {
    //        string line = getEntry(it->second[j]);
    //        log << line << "\n";
    //    }
    //}
    //uf.close();
    cout << "done\n";
    return 0;
}

//������Ŀ(�û���Ŀ)
string DBupdate::getEntry(usrEntry* data)
{
    if (!data)
        return "";
    string res = data->usrname;
    res += " ";
    res += data->passwd;
    res += " ";
    res += data->role;
    res += " ";
    res += to_string(data->balance);
    return res;
}

//������Ŀ����־��Ŀ��
string DBupdate::getEntry(logEntry* data)
{
    if (!data)
        return "";
    string res = data->start_time;
    res += " ";
    res += data->usrname;
    res += " ";
    res += to_string(data->SID);
    res += " ";
    res += data->queueNum;
    if (data->mode == FAST)
        res += " FAST ";
    else
        res += " SLOW ";
    res += to_string(data->time);
    res += "min ��";
    res += to_string(data->cost);
    return res;
}

//���ַ�chΪ�ָ����ַ������
int split(vector<string>& target, string line, char ch)
{
    string temp;
    for (int i = 0; i < line.size(); i++)
    {
        if (line[i] == ch)
        {
            target.push_back(temp);
            temp.clear();
        }
        else
        {
            temp += line[i];
        }
    }
    if(temp.size())
        target.push_back(temp);
    return 0;
}
