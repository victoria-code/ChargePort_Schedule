
#include "Server.h"

/*Server类实现*/

//服务器初始化
Server::Server()
{
    //数据库自动同步到本地

    //启动并初始化充电桩
    for (int i = 0; i < CHARGEPORT_NUM; i++)
    {
        //快充
        if (i < 2)
            cData[i] = new ChargePort(i, true, true);
        else
            cData[i] = new ChargePort(i, false, true);
    }
    //初始化排队信息
    FNum = TNum = 0;

    //新建线程读取充电桩返回的充电详单并进行调度
}

//服务器下线前同步数据库
Server::~Server()
{
    this->database.update();
}

/*database Interface*/

//用户信息查询
int Server::usrFind(string usrname, usrEntry *res)
{
    auto it = this->database.usrData.find(usrname);
    //用户不存在
    if (it == this->database.usrData.end())
    {
        res = nullptr;
        return -1;
    }
    res = it->second;
    return 0;
}

//用户充电记录查询
int Server::logFind(string usrname, vector<logEntry *> &res)
{
    auto it = this->database.logData.find(usrname);
    if (it == this->database.logData.end())
    {
        cout << "the usr " << usrname << " has no charge log currently." << endl;
        return -1;
    }
    res.assign(it->second.begin(), it->second.end());
    return 0;
}

//用户信息更新<新增用户，用户信息改变，用户注销>
int Server::usrDataUpdate(bool to_delete, usrEntry *uE)
{
    auto it = this->database.usrData.find(uE->usrname);

    //注销
    if (to_delete)
    {
        //待注销的用户名不存在
        if (it == this->database.usrData.end())
        {
            cout << "待注销的账户<" << uE->usrname << ">不存在！" << endl;
            return 0;
        }
        this->database.usrData.erase(it);
        cout << "账户<" << uE->usrname << ">注销成功！" << endl;
        return 0;
    }

    //注册 or 用户信息改变
    this->database.usrData[uE->usrname] = uE;
    return 0;
}

/*server with chargePort*/

/*server with admin*/

/*server with user*/

//响应用户客户端请求
int replyUser(Info *usrInfo)
{
}

// cmd:100 登录验证,登录成功则获取用户信息
int Server::logIn(string usrname, string passwd, usrEntry *uE)
{
    //检验用户是否存在
    if (Server::usrFind(usrname, uE) != 0)
    {
        cout << "用户名<" << usrname << ">不存在！" << endl;
        return -1;
    }
    //判断密码是否正确
    else if (passwd != uE->passwd)
    {
        cout << "用户名和密码不匹配！" << endl;
        return -2;
    }
    //登录成功
    else
    {
        cout << "登录成功！" << endl;
        return 0;
    }
}

// cmd:101 注册认证
int Server::signIn(string usrname, string passwd, string role)
{
    //用户名长度非法
    if (usrname.size() > 20 || usrname.size() < 2)
    {
        cout << "用户名长度必须在5~20之间!" << endl;
        return -1;
    }
    //用户名已存在
    else if (this->database.usrData.find(usrname) != this->database.usrData.end())
    {
        cout << "用户名已存在！" << endl;
        return -2;
    }

    cout << "注册成功!" << endl;
    usrEntry *uE = new struct usrEntry();
    uE->usrname = usrname;
    uE->passwd = passwd;
    uE->role = role;
    uE->balance = 0;
    usrDataUpdate(false, uE);
    return 0;
}

// cmd:102 充值、扣费（amount表示具体增、减金额）
int Server::balanceChange(string usrname, int amount)
{
    auto it = this->database.usrData.find(usrname);
    if (it == this->database.usrData.end())
        return -1;

    //充值
    if (amount > 0)
    {
        it->second->balance += amount;
        cout << "充值成功！" << endl;
    }
    //扣费
    if (amount < 0)
    {
        it->second->balance += amount;
        cout << "扣费成功" << endl;
    }
    return 0;
}

// cmd:103 用户注销
int Server::deleteUsr(string usrname)
{
    usrEntry *uE;
    uE->usrname = usrname;
    return Server::usrDataUpdate(true, uE);
}

// cmd:104 处理等候区内车辆的充电申请
int Server::copeChargeRequest(CarAsk *ask)
{
    //充电区内无法提交、修改用户请求
    if (CUser.find(ask->usrname) != CUser.end())
    {
        cout << "车辆当前位于充电区，无法修改充电请求！" << endl;
        return -1;
    }
    else
    {
        auto w = WUser.find(ask->usrname);
        //若用户第一次提交充电请求
        if (w == WUser.end())
        {
            //若等候区已满则无法处理请求
            if (WUser.size() > 6)
            {
                cout << "当前等候区已满，如法处理新的用户请求！" << endl;
                return -2;
            }
            //否则生成排队号码并记录前车等待数目
            else
            {
                string qNum = queueNumGenerate(ask->usrname, ask->IsFastCharge);
                this->queueData[ask->usrname].first = qNum;
                int curWait = getCurWaitNum(ask->usrname); //获取前车等待数目
                this->queueData[ask->usrname].second = curWait;
                WUser[ask->usrname] = ask;
                cout << "请求成功！"
                     << "\n"
                     << "当前排队号码：" << qNum << "\n"
                     << "本模式下前车等待数量： " << curWait << endl;
                return 0;
            }
        }
        //若用户提交过请求则覆盖原有请求
        else
        {
            //若修改了充电模式则重新生成排队号码
            if (w->second->IsFastCharge != ask->IsFastCharge)
            {
                queueData.erase(ask->usrname);
                //重新生成排队号码和前车等待数量
                string qNum = queueNumGenerate(ask->usrname, ask->IsFastCharge);
                this->queueData[ask->usrname].first = qNum;
                int curWait = getCurWaitNum(ask->usrname);
                this->queueData[ask->usrname].second = curWait;
                WUser[ask->usrname] = ask;
                cout << "请求修改成功！"
                     << "\n"
                     << "当前排队号码：" << qNum << "\n"
                     << "本模式下前车等待数量： " << curWait << endl;
                return 0;
            }
            //否则排队号不变,覆盖原有请求
            else
            {
                WUser[ask->usrname] = ask;
                cout << "请求修改成功！"
                     << "\n"
                     << "当前排队号码：" << queueData[ask->usrname].first << "\n"
                     << "本模式下前车等待数量： " << queueData[usrname].second << endl;
                return 0;
            }
        }
    }
}

// cmd:105 取消充电
int Server::cancelCharge(string usrname)
{
    auto qData = queueData.find(usrname);

    //当前用户没有未处理的充电请求
    if (qData == queueData.end())
    {
        cout << "未找到用户" << usrname << "的充电请求!" << endl;
        return -1;
    }

    //当前用户在等候区，则删除充电请求，离开等候区
    if (WUser.find(usrname) != WUser.end())
    {
        queueData.erase(usrname);
        WUser.erase(usrname);
        cout << "取消充电成功，充电请求已删除！" << endl;
        return 0;
    }

    //当前用户在充电区
    auto cU = CUser.find(usrname);
    //若正在充电则提前结束充电
    for (int i = 0; i < CHARGEPORT_NUM; i++)
    {
        if (cData[i]->IsCharging && cData[i]->ChargingCar->usrname == usrname)
        {
            //停止充电，返回详单
            cData[i]->DeleteCar(cData[i]->ChargingCar);
            return 0;
        }
    }
    //若还在充电区处于等待状态则删除充电请求，离开充电区
    CUser.erase(usrname);
    queueData.erase(usrname);
    cout << "充电取消成功！" << endl;
    return 0;
}

// cmd:106 监测到充电结束后获取充电详情，包含计费信息
//监测到充电区有空位且等候区有请求时，调度策略生成
void Server::returnBill_schedule()
{
    for (;;)
    {
        //监测到某充电桩充电结束
        if (ChargeTableHead->isAvailable)
        {
            ChargeTablePool *next = ChargeTableHead->next;
            string usrname = next->ChargeTable.usrname;
            //返回充电详单至对应的用户,并发起扣费请求
            //扣费完成
            //删除充电请求
            queueData.erase(usrname);
            //离开充电区
            CUser.erase(usrname);

            delete ChargeTableHead;
            ChargeTableHead = next;
        }

        //监测到充电区有空位且等候区对应模式有请求则进行调度
        string fUser="",tUser="";
        int fID=-1,tID=-1,fTime=INT32_MAX,tTime=INT32_MAX;
        getFreeCP(fID,tID,fTime,tTime);//获取不同模式下等待时间最短的充电桩ID
        Calling(fUser,tUser);//获取即将被叫号的用户
        if(fID!=-1){
            auto it=WUser.find(fUser);
            if(it==WUser.end()){
                cout<<"[fatal Error]: 无法在等候区找到用户"<<fUser<<endl;
                return -1;
            }
            //用户进入充电区
            CUser[fUser]=it->second;
            WUser.erase(it);
            //向充电桩发送请求
            ChargeThreadPool *cur=new Charge; 

        }
        
        if(sID!=-1){
            auto it=WUser.find(tUser);
            if(it==WUser.end()){
                cout<<"[fatal Error]: 无法在等候区找到用户"<<fUser<<endl;
                return -1;
            }
            //用户进入充电区
            CUser[tUser]=it->second;
            WUser.erase(it);
            //向充电桩发送请求

        }
    }
}

// cmd:107 获取排队信息
int Server::getQueueData(string usrname, string &qNum, int &curWait)
{
    auto it = queueData.find(usrname);
    if (it == queueData.end())
    {
        cout << "未找到用户" << usrname << "的充电请求！" << endl;
        ‘ return -1;
    }
    qNum = it->second.first;
    curWait = getCurWaitNum(usrname);
    it->second.second = curWait;
    cout << "排队号码：" << qNum << "前车等待数量" << curWait << endl;
    return 0;
}

//车辆排队号码生成
string Server::queueNumGenerate(string usrname, int mode)
{
    string qNum;
    if (mode == FAST)
        qNum = "F" + to_string(++FNum);
    else
        qNum = "T" + to_string(++TNum);
    queueData[usrname] = make_pair(qNum, -1);
}

//获取本模式下最新的前车等待数量信息
int Server::getCurWaitNum(string usrname)
{
    int num = 0;
    auto ask = this->queueData.find(usrname);
    int qNum = stoi(ask->second.first.substr(1)); //排队号码

    //如果车辆位于等候区
    auto w = WUser.find(usrname);
    if (w != WUser.end())
    {
        if (w->second->IsFastCharge)
        {
            //获取等候区内前车等待数目
            for (auto i = WUser.rbegin(); i != WUser.rend(); i++)
            {
                if (i->second->IsFastCharge && qNum > stoi(queueData[i->first].first.substr(1)))
                    num++;
            }
            num += cData[0]->IsWaiting + cData[1]->IsWaiting;
        }
        else
        {
            //获取等候区内前车等待数目
            for (auto i = WUser.rbegin(); i != WUser.rend(); i++)
            {
                if (!i->second->IsFastCharge && qNum > stoi(queueData[i->first].first.substr(1)))
                    num++;
            }
            num += cData[2]->IsWaiting + cData[3]->IsWaiting + cData[4]->IsWaiting;
        }
        return num;
    }

    //如果车辆位于充电区
    if (CUser.find(usrname) != CUser.end())
    {
        for (int i = 0; i < CHARGEPORT_NUM; i++)
        {
            //正在充电则无需等待
            if (cData[i]->IsCharging && cData[i]->ChargingCar->usrname == usrname)
                return 0;
        }
        //否则需要等待前一辆车充电完毕
        return 1;
    }
    cout << "用户当前既不在等候区，也不在充电区！" << endl;
    return -1;
}

//获取等候区内即将进入充电区的快充和慢充usrname(叫号）
int Server::Calling(string &fUser, string &tUser)
{
    fUser = "";
    tUser = "";
    int fast = INT32_MAX, slow = INT32_MAX;
    for (auto i = queueData.begin(); i != queueData.end(); i++)
    {
        int cur = stoi(i->second.first.substr(1));
        if (i->second.first[0] == 'F' && cur < fast)
        {
            fast = cur;
            fUser = i->first;
        }
        if (i->second.first[0] == 'T' && cur < slow)
        {
            slow = cur;
            tUser = i->first;
        }
    }
    return 0;
}

//获取充电区内排队队列有空位且等待时间最短的充电桩
int Server::getFreeCP(int &fID, int &tID, int &fTime, int &tTime)
{
    //记录两种模式下总时间最短的充电桩及其时间
    fTime = tTime = INT32_MAX;
    fSID = tSID = -1;
    for (int i = 0; i < CHARGEPORT_NUM; i++)
    {
        //当前充电桩队列已满
        if (cData[i]->IsCharging && cData[i]->IsWaiting)
            continue;
        //当前充电桩无充电任务,则分配到充电桩
        else if (!cData[i]->IsWaiting && !cData[i]->IsCharging)
        {
            if(cData[i]->IsFastCharge){
                fSID=i
                fTime=0;
            }
            else
            {
                tSID=i;
                tTime=0;
            }
        }
        //当前充电桩排队队列只有1个空位
        else if(cData[i]->IsWaiting)
        {
            if(cData[i]->IsFastCharge){
                int cur=cData[i]->CurElectReq/FAST_CHARGE_RATE;
                if(fTime>cur){
                    fTime=cur;
                    fSID=i;
                }
            }
            else{
                int cur=cData[i]->CurElectReq/SLOW_CHARGE_RATE;
                if(tTime>cur){
                    tTime=cur;
                    tSID=i;
                }
            }
        }
    }
}

    /*DBupdate类实现*/

    //数据库初始化
    DBupdate::DBupdate()
    {

        //加载用户文件，若不存在则直接创建
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
                usrEntry *uE = new usrEntry();
                if (entryResolve(uE, line) == 0)
                {
                    DBupdate::usrData.insert(make_pair(uE->usrname, uE));
                }
            }
        }
        else
        {
            cout << "[FATAL ERROR]: user file " << USER_FILENAME << " open failure" << endl;
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
                logEntry *lE = new logEntry();
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
    }

    //数据库条目解析(用户条目)
    int entryResolve(usrEntry * uE, string line)
    {
        vector<string> temp;
        split(temp, line, ' ');
        if (temp.size() < 4)
        {
            cout << "usrEntry "
                 << "\"" << line << " is incomplete" << endl;
            return -1;
        }
        uE->usrname = temp[0];
        uE->passwd = temp[1];
        uE->role = temp[2];
        uE->balance = stoi(temp[3]);
        return 0;
    }

    //数据库条目解析（日志条目）
    int entryResolve(logEntry * lE, string line)
    {
        vector<string> temp;
        split(temp, line, ' ');
        if (temp.size() < 7)
        {
            cout << "logEntry "
                 << "\"" << line << " is incomplete" << endl;
            return -1;
        }
        lE->start_time = temp[0];
        lE->usrname = temp[1];
        lE->SID = stoi(temp[2]);
        lE->queueNum = temp[3];
        lE->mode = stoi(temp[4]);
        lE->time = stoi(temp[5]);
        lE->cost = stoi(temp[6]);
    }

    //新增用户信息
    int DBupdate::addUser(usrEntry * data)
    {
        if (data != nullptr)
        {
            this->usrData.insert(make_pair(data->usrname, data));
            return 0;
        }
        return -1;
    }

    //新增日志条目
    int DBupdate::addLogEntry(logEntry * data)
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
        ofstream uf;
        uf.open(USER_FILENAME, ios::trunc | ios::out);
        map<string, usrEntry *>::reverse_iterator iter;
        for (iter = this->usrData.rbegin(); iter != this->usrData.rend(); iter++)
        {
            string line = DBupdate::getEntry(iter->second);
            uf << line << "\n";
        }
        uf.close();

        //更新服务日志
        ofstream log;
        log.open(LOG_FILENAME, ios::trunc | ios::out);
        map<string, vector<logEntry *>>::reverse_iterator it;
        for (it = this->logData.rbegin(); it != this->logData.rend(); it++)
        {
            for (int j = 0; j < it->second.size(); j++)
            {
                string line = DBupdate::getEntry(it->second[j]);
                log << line << "\n";
            }
        }
        uf.close();
    }

    //构造条目(用户条目)
    string DBupdate::getEntry(usrEntry * data)
    {
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
    string DBupdate::getEntry(logEntry * data)
    {
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
    int split(vector<string> & target, string line, char ch)
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
        return 0;
    }
