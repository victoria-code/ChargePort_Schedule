
#include "Server.h"
extern struct Info send_info, recv_info;
/*Server类实现*/

//服务器初始化
Server::Server(int res) {
    //数据库自动同步到本地<数据库构造函数已经完成>

    time_t cur = time(NULL);
    //第一次上线，不载入历史信息
    if (res == 0) {
        //启动并初始化充电桩
        for (int i = 0; i < CHARGEPORT_NUM; i++)
        {
            //快充
            if (i < FAST_NUM)
                cData.push_back(new ChargePort(i, true, true, MAX_WAIT_NUM));
            else
                cData.push_back(new ChargePort(i, false, true, MAX_WAIT_NUM));
            
            cData[i]->FiveTime = cur;
        }
    }

    //否则载入充电桩历史信息
    else {
        ifstream hy;
        hy.open(HISTORY, ios::in | ios::out);
        if (hy.is_open()) {
            for (int i = 0; !hy.eof() && i < CHARGEPORT_NUM; i++) {
                //充电桩写入历史信息
                int CCnt = -1;
                double CCost = -1, ECost = -1;
                long long CTime = -1;
                double TElect = -1;
                double SCost = -1;
                hy >> CCnt >> CCost >> ECost >> CTime >> TElect >> SCost;
                //快充
                if (i < FAST_NUM)
                    cData.push_back(new ChargePort(i, true, true, MAX_WAIT_NUM, CCnt, CCost, ECost, CTime, TElect, SCost));
                //慢充
                else
                    cData.push_back(new ChargePort(i, false, true, MAX_WAIT_NUM, CCnt, CCost, ECost, CTime, TElect, SCost));
            }
        }
        else {
            std::cout << "[FATAL ERROR]: history file " << HISTORY << " open failure" << endl;
        }
    }

    fstream de;
    de.open(DETAIL_FILE, ios::out | ios::trunc);
    de.close();

    //初始化排队信息
    FNum = TNum = 0;
    std::cout << "服务器初始化完成！" << endl;
}

//服务器下线前同步数据库
Server::~Server()
{
    database.update();
}

//打印充电桩信息
void Server::printStat() {
    //充电桩
    cout << "---";
    for (int i = 0; i < CHARGEPORT_NUM; i++)
        cout << "---" << i << "---";
    cout << endl;

    for (int i = 0; i < CHARGEPORT_NUM; i++) {
        if (cData[i]->ChargingCar)
            cout << cData[i]->ChargingCar->usrname << "\t";
        else
            cout << "\t\t";
    }
    cout << endl;

    for (int i = 0; i < QUEUE_LENGTH; i++) {
        for (int j = 0; j < CHARGEPORT_NUM; j++) {
            if (cData[j]->WaitCount > i)
                cout << cData[j]->WaitingCar[i]->usrname << "\t";
            else
                cout << "       ";
        }
        cout << endl;
    }
}

/*database Interface*/

//用户信息查询---ok
int Server::usrFind(string usrname, usrEntry* res)
{
    res = database.usrData[usrname];
    //用户不存在
    if (!res)
    {
        std::cout << "[From Database]: 用户" << usrname << "不存在！" << endl;
        return -1;
    }
    return 0;
}

//用户充电记录查询
int Server::logFind(string usrname, vector<logEntry*>& res)
{
    auto it = database.logData.find(usrname);
    if (it == database.logData.end())
    {
        std::cout << "the usr " << usrname << " has no charge log currently." << endl;
        return -1;
    }
    res.assign(it->second.begin(), it->second.end());
    return 0;
}

//用户信息更新<新增用户，用户信息改变，用户注销>--ok
int Server::usrDataUpdate(bool to_delete, usrEntry* uE)
{
    if (!uE) {
        std::cout << "[fatal error]:uE为空" << endl;
        return -1;
    }

    //注销
    if (to_delete)
    {
        //待注销的用户名不存在
        if (database.usrData[uE->usrname] == nullptr)
        {
            std::cout << "待注销的账户<" << uE->usrname << ">不存在！" << endl;
            return 0;
        }
        database.usrData.erase(uE->usrname);
        std::cout << "账户<" << uE->usrname << ">注销成功！" << endl;
        return 0;
    }

    //注册 or 用户信息改变
    if (database.usrData[uE->usrname])
        database.usrData.erase(uE->usrname);
    database.usrData[uE->usrname] = uE;
    return 0;
}


//向用户发送对应的充电详单并进行扣费--ok
void Server::sendDetail(ChargeTablePool* next)
{
    send_info.cmd = DETAIL;
    strcpy_s(send_info.UID, next->ChargeTable.usrname.c_str());

    string res = "--------------------------------充电详单--------------------------------\n";
    res += "详单编号: " + to_string(next->ChargeTable.ChargeID) + "\n";
    res += "详单生成时间: " + getCurTime(next->ChargeTable.CreateTableTime) + "\n";
    res += "充电桩编号: " + to_string(next->ChargeTable.SID) + "\n";
    res += "充电模式: ";
    if (next->ChargeTable.IsFastCharge)
        res += "FAST\n";
    else
        res += "SLOW\n";
    res += "用户名:" + next->ChargeTable.usrname + "\n";
    res += "启动时间:" + getCurTime(next->ChargeTable.StartTime) + "\n";
    res += "停止时间:" + getCurTime(next->ChargeTable.End_Time) + "\n";
    res += "总充电量:" + to_string(next->ChargeTable.TotalElect) + "\n";
    res += "充电时长:" + to_string(next->ChargeTable.ChargeTime) + "s\n";
    res += "服务费:" + to_string(next->ChargeTable.ServiceCost) + "\n";
    res += "电费：" + to_string(next->ChargeTable.ElectCost) + "\n";
    res += "总费用:" + to_string(next->ChargeTable.ChargeCost) + "\n";
    res += "--------------------------------------------------------------------------\n\n";
    fstream de;
    de.open(DETAIL_FILE, ios::out | ios::app);
    de << res;
    de.close();
    //扣费
    balanceChange(next->ChargeTable.usrname, -next->ChargeTable.ChargeCost);
    database.update();
    send_info.BALANCE = database.usrData[next->ChargeTable.usrname]->balance;
    res += "<扣费成功>:当前余额：" + to_string(send_info.BALANCE) + "元\n";
    cout << res << endl;
    CUser.erase(next->ChargeTable.usrname);
    CUserID.erase(next->ChargeTable.usrname);
    if (FUser[next->ChargeTable.usrname] == nullptr)
        queueData.erase(next->ChargeTable.usrname);
    strcpy_s(send_info.output, res.c_str());

    send_info.REPLY = 0;
    server_sock.Send(send_info);
}

//服务器端向充电桩转发充电请求（充电区）--ok
void Server::forwardRequet(string usrname, int SID)
{
    CarReply reply;
    string qNum = queueData[usrname].first;
    CarAsk* ask = this->CUser[usrname];
    reply.Ask = *(ask);
    reply.SID = SID;
    reply.num = cData[SID]->WaitCount; //当前排队队列编号 
    reply.MODE = qNum[0];               //充电模式F or T
    reply.queueNum = qNum;              //排队号码
    reply.waitingNum = cData[SID]->WaitCount;       //前车等待数量

    Car* car = new Car();
    car->usrname = usrname;
    car->BatteryCap = ask->BatteryCap;
    car->BatteryNow = ask->BatteryNow;
    car->Ask = ask;
    car->Reply = &reply;
    cout << "即将发送..."<<endl;
    //转发充电请求
    cData[SID]->AddCar(reply, car);
}

/*with Client*/

//响应用户客户端请求
int Server::replyClient(Info usrInfo)
{
    string Usrname = string(usrInfo.UID);
    usrEntry* uE = database.usrData[Usrname];
    string qNum, role;
    switch (usrInfo.cmd)
    {
        // cmd:100 登录验证
    case LOG_IN:
        logIn(Usrname, string(usrInfo.PWD), uE);
        break;

        // cmd:101 注册验证
    case SIGN_UP:
        role = (usrInfo.MODE == 1) ? "customer" : "admin";
        signUp(Usrname, string(usrInfo.PWD), role);
        database.update();
        break;

        // cmd:102 充值、扣费
    case Balance_CHANGE:
        balanceChange(Usrname, usrInfo.BALANCE);
        database.update();
        break;

        // cmd:103 注销
    case DELETE_USER:
        deleteUsr(Usrname);
        database.update();
        break;

        // cmd: 104 充电请求
    case CHARGE_REQUEST:
    {
        CarAsk* ask = new CarAsk();
        resolveRequest(usrInfo, ask);
        if (ask == nullptr)
            std::cout << "null!" << endl;
        copeChargeRequest(ask);
    }
    break;

    // cmd: 105 取消充电
    case CANCEL_REQUEST:
        cancelCharge(Usrname);
        break;

        // cmd: 106 获取排队信息
    case GET_QUEUE_DATA:
        int curWait;
        getQueueData(Usrname, qNum, curWait);
        break;

        //cmd: 200 获取充电桩状态信息(ADMIN)
    case SPY_CHARGEPORT:
        getChargePortData(Usrname);
        break;

        //cmd:201 查看等候充电桩服务的车辆信息(ADMIN)
    case WAIT_CAR_DATA:
        getCarData(Usrname);
        break;

        //cmd:202 查看报表（ADMIN)
    case DATA_STATISTICS:
        getReport(Usrname);
        break;

        //cmd:203 开启充电桩
    case OPEN_CHARGEPORT:
        openCP(Usrname, usrInfo.REPLY);
        break;

        //cmd:204 关闭充电桩
    case CLOSE_CHARGEPORT:
        closeCP(Usrname, usrInfo.REPLY);
        break;
        // default:break;
    }

    return 0;
}

// cmd:100 登录验证,登录成功则获取用户信息ok
int Server::logIn(string usrname, string passwd, usrEntry* uE)
{
    strcpy_s(send_info.UID, usrname.c_str());
    send_info.cmd = LOG_IN;
    std::cout << "登录验证..." << endl;
    string res;
    //检验用户是否存在
    if (usrFind(usrname, uE) != 0)
    {
        res = "用户名<" + usrname + ">不存在！" + "\n";
        std::cout << res << endl;
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        return -1;
    }
    //判断密码是否正确
    else if (passwd != uE->passwd)
    {
        res = "用户名和密码不匹配！\n";
        std::cout << res;
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -2;
        server_sock.Send(send_info);
        return -2;
    }
    //登录成功
    else
    {
        res = "登录成功！\n";
        std::cout << res;
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

// cmd:101 注册认证ok
int Server::signUp(string usrname, string passwd, string role)
{
    std::cout << "[注册验证] : ";
    strcpy_s(send_info.UID, usrname.c_str());
    send_info.cmd = SIGN_UP;
    string res;
    //用户名长度非法
    if (usrname.size() > 20 || usrname.size() < 2 || usrname.find(" ") != string::npos)
    {
        res = "用户名长度必须在5~20之间,且不能包含空格!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        std::cout << res;
        return -1;
    }
    //用户名已存在
    else if (database.usrData[usrname])
    {
        res = "用户名已存在！\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -2;
        server_sock.Send(send_info);
        std::cout << res;
        return -2;
    }

    usrEntry* uE = new usrEntry();
    std::cout << usrname << " " << "注册成功!" << endl;
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

// cmd:102 充值、扣费（amount为正表示充值后的余额，为负表示要扣除的金额）---ok
int Server::balanceChange(string usrname, double amount)
{
    strcpy_s(send_info.UID, usrname.c_str());
    usrEntry* uE = database.usrData[usrname];
    string res;

    //充值
    if (amount > 0)
    {
        uE->balance = amount;
        res = "<" + usrname + "充值成功>: ";
        res += "当前余额为" + to_string(uE->balance) + "元!\n";
        std::cout << res;
    }
    //扣费
    if (amount < 0)
    {
        uE->balance += amount;
        res = "<" + usrname + "扣费成功>: ";
        res += "当前余额为" + to_string(uE->balance) + "元!\n";
        std::cout << res;
    }
    return 0;
}

// cmd:103 用户注销ok
int Server::deleteUsr(string usrname)
{
    usrEntry* uE = new usrEntry();
    uE->usrname = usrname;
    send_info.cmd = DELETE_USER;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "用户<" + usrname + ">注销成功!\n";
    //等候区注销
    if (WUser.find(usrname)!=WUser.end()) {
        WUser.erase(usrname);
        queueData.erase(usrname);
    }

    //充电区注销
    else if (CUser.find(usrname)!=CUser.end()) {
        //正在充电则先生成详单
        int num = CUserID[usrname];
        if (cData[num]->ChargingCar->usrname == usrname) {
            cData[num]->DeleteCar(cData[num]->ChargingCar);
            cout << "等待详单生成..." << endl;
        }
        else {
            CUser.erase(usrname);
            CUserID.erase(usrname);
            queueData.erase(usrname);
        }
    }
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    std::cout << res;
    return Server::usrDataUpdate(true, uE);
}

// cmd:104 处理等候区内车辆的充电申请ok
int Server::copeChargeRequest(CarAsk* ask)
{
    strcpy_s(send_info.UID, ask->usrname.c_str());
    send_info.cmd = CHARGE_REQUEST;
    send_info.MODE = ask->IsFastCharge;
    string res;
    //充电区内无法提交、修改用户请求
    if (CUser.find(ask->usrname)!=CUser.end())
    {
        res = "车辆当前位于充电区，无法修改充电请求！";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        std::cout << res;
        return -1;
    }
    else
    {
        auto ss = WUser.find(ask->usrname);
        CarAsk* w = nullptr;
        if (ss != WUser.end())
            w = ss->second;
        //若用户第一次提交充电请求
        if (w == nullptr)
        {
            //若等候区已满则无法处理请求
            if (WUser.size() > MAX_WAIT_NUM )
            {
                res = "当前等候区已满，无法处理新的用户请求！\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.REPLY = -2;
                server_sock.Send(send_info);
                std::cout << res;
                return -2;
            }
            //否则生成排队号码并记录前车等待数目
            else
            {
                double estCost = ask->BatteryCap * (SERVICE_PRICE + ELEC_PRICE);
                if (database.usrData[ask->usrname]->balance < estCost) {
                    send_info.REPLY = -3;
                    res = "用户余额不足，请充值后再提交充电请求！\n";
                    strcpy_s(send_info.output, res.c_str());
                    server_sock.Send(send_info);
                    std::cout << res;
                    return -3;
                }

                reqRes[ask->usrname] = false;
                WUser[ask->usrname] = ask;
                string qNum = queueNumGenerate(ask->usrname, ask->IsFastCharge);
                queueData[ask->usrname].first = qNum;
                int curWait = getCurWaitNum(ask->usrname); //获取前车等待数目
                queueData[ask->usrname].second = curWait;
                res = "请求成功！\n当前排队号码：" + qNum + "\n本模式下前车等待数量： " + to_string(curWait) + "\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.REPLY = 0;
                send_info.Q_NUM = stoi(qNum.substr(1));
                server_sock.Send(send_info);
                std::cout << res;
                reqRes[ask->usrname] = true;
                return 0;
            }
        }
        //若用户提交过请求则覆盖原有请求
        else
        {
            //若修改了充电模式则重新生成排队号码
            if (w->IsFastCharge != ask->IsFastCharge)
            {
                reqRes[ask->usrname] = false;
                queueData.erase(ask->usrname);
                //重新生成排队号码和前车等待数量
                string qNum = queueNumGenerate(ask->usrname, ask->IsFastCharge);
                this->queueData[ask->usrname].first = qNum;
                int curWait = getCurWaitNum(ask->usrname);
                this->queueData[ask->usrname].second = curWait;
                WUser[ask->usrname] = ask;
                res = "请求修改成功！\n当前排队号码：" + qNum + "\n本模式下前车等待数量： " + to_string(curWait) + "\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.REPLY = 0;
                send_info.Q_NUM = stoi(qNum.substr(1));
                server_sock.Send(send_info);
                std::cout << res;
                reqRes[ask->usrname] = true;
                return 0;
            }
            //否则排队号不变,覆盖原有请求
            else
            {
                reqRes[ask->usrname] = false;
                WUser[ask->usrname] = ask;
                res = "请求修改成功！\n当前排队号码：" + queueData[ask->usrname].first + "\n本模式下前车等待数量： " + to_string(queueData[ask->usrname].second) + "\n";
                strcpy_s(send_info.output, res.c_str());
                send_info.Q_NUM = stoi(queueData[ask->usrname].first.substr(1));
                send_info.REPLY = 0;
                server_sock.Send(send_info);
                std::cout << res;
                reqRes[ask->usrname] = true;
                return 0;
            }
        }
    }
}

// cmd:105 取消充电--ok
int Server::cancelCharge(string usrname)
{
    send_info.cmd = CANCEL_REQUEST;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;
    auto qData = queueData.find(usrname);

    //当前用户没有未处理的充电请求
    if (qData == queueData.end())
    {
        res = "未找到用户" + usrname + "的充电请求!" + "\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        std::cout << res;
        return -1;
    }

    //当前用户在等候区或故障队列，则删除充电请求，离开等候区
    if (WUser.find(usrname)!=WUser.end() || FUser[usrname])
    {
        queueData.erase(usrname);
        if (WUser.find(usrname)!=WUser.end())
            WUser.erase(usrname);
        else
            FUser.erase(usrname);
        res = "取消充电成功，充电请求已删除！\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = 0;
        server_sock.Send(send_info);
        std::cout << res;
        return 0;
    }

    CarAsk* cU = CUser[usrname];
    int num = CUserID[usrname];
    if (!cU) {
        res = "充电取消失败！，用户既不在充电区也不在等候区！\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -2;
        server_sock.Send(send_info);
        std::cout << res;
        return -2;
    }
    cout << "num:" << num << endl;

    //若正在充电(前车等待数量为0)则提前结束充电 
    if (cData[num]->ChargingCar->usrname == usrname) {
        //停止充电，返回详单,离开充电区
        int num = CUserID[usrname];
        res = "充电取消成功，即将生成详单...\n";

        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = 0;
        server_sock.Send(send_info);

        cData[num]->DeleteCar(cData[num]->ChargingCar);
        CUser.erase(usrname);
        queueData.erase(usrname);
        CUserID.erase(usrname);
        std::cout << res;
        return 0;
    }
    //若还在充电区处于等待状态则从等候队列中移除、删除充电请求，离开充电区
    for (int k = 0; k < cData[k]->WaitCount; k++) {
        if (cData[num]->WaitingCar[k]->usrname == usrname) {
            CUser.erase(usrname);
            queueData.erase(usrname);
            CUserID.erase(usrname);
            cData[num]->DeleteCar(cData[num]->WaitingCar[k]);
            break;
        }
    }
    res = "充电取消成功！\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    std::cout << res;
    return 0;
}

// cmd:106 获取排队信息--ok
int Server::getQueueData(string usrname, string& qNum, int& curWait)
{
    send_info.cmd = GET_QUEUE_DATA;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;

    auto it = queueData.find(usrname);
    if (it == queueData.end())
    {
        send_info.Q_NUM = -1;
        res = "未找到用户<" + usrname + ">的充电请求！\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        std::cout << res;
        return -1;
    }
    qNum = it->second.first;
    curWait = getCurWaitNum(usrname);
    it->second.second = curWait;
    send_info.MODE = (qNum[0] == 'F') ? 1 : 0;
    send_info.Q_NUM = stoi(qNum.substr(1));
    send_info.W_NUM = curWait;
    res = "排队号码：" + qNum + "前车等待数量" + to_string(curWait) + "\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    std::cout << res;
    return 0;
}

//车辆排队号码生成--ok
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

//获取本模式下最新的前车等待数量信息---ok
int Server::getCurWaitNum(string usrname)
{
    int num = 0;
    auto ask = this->queueData.find(usrname);
    int qNum = stoi(ask->second.first.substr(1)); //排队号码

    auto ss =WUser.find(usrname);
    CarAsk* w=nullptr;
    //如果车辆位于等候区
    if (ss != WUser.end())
        w = ss->second;
    if (w)
    {
        if (w->IsFastCharge)
        {
            //获取等候区内前车等待数目
            for (auto i = WUser.begin(); i != WUser.end(); i++)
            {
                if (WUser.find(i->first)!=WUser.end() && i->second->IsFastCharge && qNum > stoi(queueData[i->first].first.substr(1)))
                    num++;
            }
            //获取充电区内同模式下所有等待的车辆数目
            for (int k = 0; k < FAST_NUM; k++) {
                num += cData[k]->WaitCount;
            }

        }
        else
        {
            //获取等候区内前车等待数目
            for (auto i = WUser.rbegin(); i != WUser.rend(); i++)
            {
                if (WUser.find(i->first)!=WUser.end()&&!i->second->IsFastCharge && qNum > stoi(queueData[i->first].first.substr(1)))
                    num++;
            }
            //获取充电区内同模式下所有等待的车辆数目
            for (int k = FAST_NUM; k < CHARGEPORT_NUM; k++) {
                num += cData[k]->WaitCount;
            }

        }
        return num;
    }

    //如果车辆位于充电区
    CarAsk* c = CUser[usrname];
    if (c)
    {
        num = 0;
        int k = CUserID[usrname];
        for (int j = 0; j < cData[k]->WaitCount; j++) {
            string usr = cData[k]->WaitingCar[j]->usrname;
            int cNum = stoi(queueData[usr].first.substr(1));
            if (cNum < qNum)
                num++;
        }
        return num;
    }

    //即将进入等候区
    std::cout << "用户" << usrname << "既不在等候区，也不在充电区\n";
    return -1;
}

//获取等候区内即将进入充电区的快充和慢充usrname(叫号）
int Server::Calling(string& fUser, string& tUser)
{
    fUser = "";
    tUser = "";
    int fast = INT32_MAX, slow = INT32_MAX;
    bool f_ok = false, t_ok = false;

    //从故障队列叫号
    for (auto i = queueData.begin(); i != queueData.end(); i++) {
        int cur = stoi(i->second.first.substr(1));
        if (FUser[i->first] && i->second.first[0] == 'F' && cur < fast) {
            fast = cur;
            fUser = i->first;
        }
        if (FUser[i->first] && i->second.first[0] == 'T' && cur < slow) {
            slow = cur;
            tUser = i->first;
        }
    }

    //从等候区叫号
    for (auto i = queueData.begin(); i != queueData.end(); i++)
    {
        int cur = stoi(i->second.first.substr(1));
        if ((WUser.find(i->first)!=WUser.end() && reqRes[i->first]) && i->second.first[0] == 'F' && cur < fast)
        {
            fast = cur;
            fUser = i->first;
        }
        if (WUser.find(i->first) != WUser.end() && reqRes[i->first] && i->second.first[0] == 'T' && cur < slow)
        {
            slow = cur;
            tUser = i->first;
        }
    }
    return 0;
}

//获取充电区内排队队列有空位且等待时间最短的充电桩--ok
int Server::getFreeCP(int& fSID, int& tSID, int& fTime, int& tTime)
{
    //记录两种模式下总时间最短的充电桩及其时间
    fTime = tTime = INT32_MAX;
    fSID = tSID = -1;
    for (int i = 0; i < FAST_NUM; i++) {
        if (cData[i]->WaitCount +1 == QUEUE_LENGTH || cData[i]->OnState == false)
            continue;//无空位或故障或关闭的充电桩跳过
      
        int cur = 0;
        for (int j = 0; j < cData[i]->WaitCount; j++) {
            string usr = cData[i]->WaitingCar[j]->usrname;
            cur += (CUser[usr]->ChargeCap / FAST_POWER) * 60;
        }
        if(cData[i]->IsCharging)
            cur += (cData[i]->CurElectReq / FAST_POWER) * 60;
        if (cur < fTime) {
            fSID = i;
            fTime = cur;
        }
    }
    for (int i = FAST_NUM; i < CHARGEPORT_NUM; i++) {
        if (cData[i]->WaitCount +1 == QUEUE_LENGTH || cData[i]->OnState == false)
            continue;//无空位或故障或关闭的充电桩跳过
        
        int cur = 0;
        for (int j = 0; j < cData[i]->WaitCount; j++) {
            string usr = cData[i]->WaitingCar[j]->usrname;
            cur += (CUser[usr]->ChargeCap / SLOW_POWER) * 60;
        }
        if (cData[i]->IsCharging)
            cur += (cData[i]->CurElectReq / SLOW_POWER) * 60;
        if (cur < tTime) {
            tSID = i;
            tTime = cur;
        }
    }
    return 0;
}

//提取充电请求
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

//cmd200: 获取充电桩状态信息--ok
int Server::getChargePortData(string usrname) {
    send_info.cmd = SPY_CHARGEPORT;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "<<<<<<<<<<<<<<<<<<<<充电状态信息>>>>>>>>>>>>>>>>>>>>\n";
    for (int i = 0; i < CHARGEPORT_NUM; i++) {
        res += "充电桩编号：" + to_string(i) + "\n";
        res += "是否为快充：" + to_string(cData[i]->IsFastCharge) + "\n";
        res += "是否正常工作：" + to_string(1) + "\n";
        res += "是否已经启动" + to_string(cData[i]->OnState) + "\n";
        res += "系统启动后累计充电次数：" + to_string(cData[i]->ChargeCnt) + "\n";
        res += "系统启动后累计充电总时长：" + to_string(cData[i]->ChargeTime) + "\n";
        res += "系统启动后累计充电总电量：" + to_string(cData[i]->TotalElect) + "\n";
        res += "----------------------------------------------------------------------\n";
    }
    send_info.REPLY = 0;
    strcpy_s(send_info.output, res.c_str());
    std::cout << res << endl;
    server_sock.Send(send_info);
    return 0;
}

//cmd201:查看充电桩等候的车辆信息
int Server::getCarData(string usrname) {
    send_info.cmd = WAIT_CAR_DATA;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "<<<<<<<<<<<<<<<<<<<<充电桩等候服务车辆信息>>>>>>>>>>>>>>>>>>>>\n";
    for (int i = 0; i < CHARGEPORT_NUM; i++) {
        res += "----------------------------------------------------------------------\n";
        res += "充电桩编号：" + to_string(i) + "\n";
        res += "是否为快充：" + to_string(cData[i]->IsFastCharge) + "\n";
        for (int j = 0; j < cData[i]->WaitCount; j++) {
            res += to_string(j) + " ) 用户名: " + cData[i]->WaitingCar[j]->usrname;
            res += "\n电池容量：" + to_string(cData[i]->WaitingCar[j]->BatteryCap);
            res += "\n当前电池电量：" + to_string(cData[i]->WaitingCar[j]->BatteryNow) + "\n";
            res += "等待时间：" + to_string(time(NULL) - cData[i]->WaitingCar[j]->Ask->StWaitTime) + "s\n\n";
        }
        res += "----------------------------------------------------------------------\n\n";
    }
    return 0;
}

//cmd202: 查看报表--ok
int Server::getReport(string usrname) {
    send_info.cmd = DATA_STATISTICS;
    strcpy_s(send_info.UID, usrname.c_str());
    string res = "<<<<<<<<<<<<<<<<<<<<充电报表>>>>>>>>>>>>>>>>>>>>\n";
    res += "当前时间：" + getCurTime(0) + "\n";
    for (int i = 0; i < CHARGEPORT_NUM; i++) {
        CPStatusTable report = cData[i]->GetStatus();
        res += "充电桩编号：" + to_string(i) + "\n";
        res += "是否为快充：" + to_string(report.IsFastCharge) + "\n";
        res += "累计充电次数：" + to_string(report.ChargeCnt + report.PastChargeCnt) + "\n";
        res += "累计充电时长：" + to_string(report.ChargeTime + report.PastChargeTime) + "min\n";
        res += "累计充电量：" + to_string(report.TotalElect + report.PastTotalElect) + "度\n";
        res += "累计充电费用：" + to_string(report.ElectCost + report.PastElectCost) + "元\n";
        res += "累计服务费用：" + to_string(report.ServiceCost + report.PastServiceCost) + "元\n";
        res += "累计总费用：" + to_string(report.ChargeCost + report.PastChargeCost) + "元\n";
        res += "--------------------------------------------------------------------\n\n";
    }
    strcpy_s(send_info.output, res.c_str());
    std::cout << res << endl;
    send_info.REPLY = 0;
    server_sock.Send(send_info);
    return 0;
}

//cmd203：开启充电桩
int Server::openCP(string usrname, int SID) {
    usrEntry* uE = new usrEntry();
    usrFind(usrname, uE);
    send_info.cmd = OPEN_CHARGEPORT;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;
    if (cData[SID]->OnState) {
        res = "启动失败，充电桩已经开启!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        std::cout << res;
        return -1;
    }
    if (uE->role == "admin" && 0 <= SID < CHARGEPORT_NUM) {
        cData[SID]->on();
        res = "充电桩" + to_string(SID) + "启动成功!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = 0;
        server_sock.Send(send_info);
        std::cout << res;
        return 0;
    }
    res = "充电桩" + to_string(SID) + "开启失败\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = -2;
    server_sock.Send(send_info);
    std::cout << res;
    return -2;
}

//cmd204: 关闭充电桩
int Server::closeCP(string usrname, int SID) {
    usrEntry* uE = new usrEntry();
    usrFind(usrname, uE);
    send_info.cmd = OPEN_CHARGEPORT;
    strcpy_s(send_info.UID, usrname.c_str());
    string res;
    if (cData[SID]->OnState == false) {
        res = "关闭失败，充电桩已经处于关闭状态!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        std::cout << res;
        server_sock.Send(send_info);
        return -1;
    }
    if (uE->role == "admin" && 0 <= SID < CHARGEPORT_NUM) {
        cData[SID]->off();
        res = "充电桩" + to_string(SID) + "关闭成功!\n";
        strcpy_s(send_info.output, res.c_str());
        send_info.REPLY = -1;
        server_sock.Send(send_info);
        //将正在等待的车移入故障队列
        for (int i = 0; i < cData[SID]->WaitCount; i++) {
            string cur_usr = cData[SID]->WaitingCar[i]->usrname;
            CarAsk* ask = CUser[cur_usr];
            FUser[cur_usr] = ask;
            CUser.erase(cur_usr);
            CUserID.erase(cur_usr);
            cData[SID]->DeleteCar(cData[SID]->WaitingCar[i]);
        }
        //将正在充电的车移入故障队列
        if (cData[SID]->ChargingCar) {
            CarAsk* ask = new CarAsk();
            string cur_usr = cData[SID]->ChargingCar->usrname;
            ask->usrname = cur_usr;
            ask->BatteryCap = cData[SID]->ChargingCar->BatteryCap;
            ask->BatteryNow = cData[SID]->ChargingCar->BatteryNow;
            ask->ChargeCap = cData[SID]->CurElectReq;
            ask->IsFastCharge = cData[SID]->ChargingCar->Ask->IsFastCharge;
            FUser[usrname] = ask;
            cData[SID]->DeleteCar(cData[SID]->ChargingCar);
        }
        std::cout << res;
        return 0;
    }
    res = "充电桩" + to_string(SID) + "关闭失败\n";
    strcpy_s(send_info.output, res.c_str());
    send_info.REPLY = -2;
    server_sock.Send(send_info);
    std::cout << res;
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


/*DBupdate类实现*/

//数据库初始化
DBupdate::DBupdate()
{

    //加载用户文件，若不存在则直接创建
    std::cout << "加载数据库..." << endl;
    ifstream uf;
    uf.open(USER_FILENAME, ifstream::out | ifstream::app);
    if (uf.is_open())
    {
        while (!uf.eof())
        {
            //读取用户条目
            string line;
            getline(uf, line);
            while (line == "\n" && !uf.eof())
            {
                getline(uf, line);
            }
            //用户条目解析
            usrEntry* uE = new usrEntry();
            if (entryResolve(uE, line) == 0)
            {
                DBupdate::usrData[uE->usrname] = uE;
            }
        }
    }
    else
    {
        std::cout << "[FATAL ERROR]: user file " << USER_FILENAME << " open failure" << endl;
    }
    uf.close();


    //加载日志文件，若不存在则直接创建
    ifstream log;
    log.open(LOG_FILENAME, ifstream::out | ifstream::app);
    if (log.is_open())
    {
        while (!log.eof())
        {
            //读取日志条目
            string line;
            getline(log, line);
            while (line == "\n" && !log.eof())
            {
                getline(log, line);
            }
            //日志条目解析
            logEntry* lE = new logEntry();
            if (entryResolve(lE, line) == 0)
            {
                DBupdate::logData[lE->usrname].push_back(lE);
            }
        }
    }
    else
    {
        std::cout << "[FATAL ERROR]: log file " << LOG_FILENAME << " open failure" << endl;
    }
    log.close();
    std::cout << "数据库同步完毕!" << endl;
}

//数据库条目解析(用户条目)
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

//数据库条目解析（日志条目）
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

//新增用户信息
int DBupdate::addUser(usrEntry* data)
{
    if (data != nullptr)
    {
        this->usrData[data->usrname] = data;
        return 0;
    }
    return -1;
}

//新增日志条目
int DBupdate::addLogEntry(logEntry* data)
{
    if (data != nullptr)
    {
        this->logData[data->usrname].push_back(data);
        return 0;
    }
    return -1;
}

//更新数据库
int DBupdate::update()
{

    //更新用户文件
    std::cout << "更新数据库...";
    ofstream uf;
    uf.open(USER_FILENAME, ios::trunc | ios::out);
    //  map<string, usrEntry*>::reverse_iterator iter;
    for (auto iter = usrData.begin(); iter != usrData.end(); iter++)
    {
        string line = getEntry(iter->second);
        if (line.size())
            uf << line << "\n";
    }
    uf.close();

    ////更新服务日志
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
    std::cout << "done\n";
    return 0;
}

//构造条目(用户条目)
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

//构造条目（日志条目）
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
    res += "min ￥";
    res += to_string(data->cost);
    return res;
}

//以字符ch为分隔符字符串拆分
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
    if (temp.size())
        target.push_back(temp);
    return 0;
}
