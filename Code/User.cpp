#include "User.h"
#include "TSocket.h"    //socket
#include "ChargePort.h" //充电详单

struct Info send_info, recv_info;
/*
用户登录
服务器：检查用户名和密码
返回：登录成功/失败(1/0)
*/
int User::logIN()
{
    int suc = 0;
    cout << "==========登录界面=========" << endl;
    //提示用户注册
    string choice[] = {"以现有账号登录", "注册新账号"};
    cout << "请选择登录选项: ";
    printChoice(choice, 2);
    string id;
    while (getline(cin, id), !isLegalChoice(id, 2))
    {
        cout << "输入无效,请重新选择: ";
        printChoice(choice, 2);
    }
    if (id == "2")
    {
        signUp();
        cout << "==========登录界面=========" << endl;
    }

    //获取用户输入的用户名和密码
    string usrname, password;
    cout << "请输入用户名：";
    while (getline(cin, usrname), usrname == "")
        ;
    cout << "请输入密码：";
    while (getline(cin, password), password == "")
        ;

    //向服务器发送登录请求
    suc = sendLogInRequest(usrname, password);

    //提示登录成功/失败信息
    // suc=1表示成功，2表示用户名错误，3表示密码错误
    while (suc != 1)
    {
        if (suc == 2)
        {
            cout << "登录失败，请检查用户名是否正确" << endl;
            cout << "重新输入用户名:";
            while (getline(cin, usrname), usrname == "")
                ;
        }
        else if (suc == 3)
        {
            cout << "登录失败，请检查密码是否正确" << endl;
            cout << "重新输入密码:";
            while (getline(cin, password), password == "")
                ;
        }
        suc = sendLogInRequest(usrname, password);
    }

    //初始化用户名、余额和车辆信息
    this->usrname = usrname;
    if (this->type == CUSTOMER)
    {
        this->balance = getUsrBalance();
        this->car = getCarInfo();
    }

    cout << "登录成功！" << endl;
    return suc;
}
/*
获取消费者余额
socket
*/
double User::getUsrBalance()
{
    cout << "getUsrBalance\n";
    return 0.0; // test
}
/*
获取消费者车辆信息
socket
*/
Car *User::getCarInfo()
{
    cout << "getCarInfo\n";
    return nullptr; // test
}

/*
发送登录请求
socket
*/
int User::sendLogInRequest(string usrname, string password)
{
    return 1; // test

    // send_info.cmd = LOG_IN;
    // strcpy(send_info.UID, usrname.c_str());
    // strcpy(send_info.PWD, password.c_str());

    // //发送报文
    // client_sock.Send(send_info);

    // //接收服务器发回的响应报文
    // client_sock.Recv(recv_info);

    // return recv_info.REPLY;   //test
}

/*
用户注册
服务器：检查用户名和密码，新增用户数据
返回：注册成功/失败(1/0)
*/
int User::signUp()
{
    cout << "==========注册界面=========" << endl;
    //获取用户输入的用户名和密码
    //对密码没有限制，只要不为空即可；用户名不能与已有的重复
    string usrname, password;
    cout << "请输入用户名：";
    while (getline(cin, usrname), usrname == "")
        ;
    cout << "请输入密码：";
    while (getline(cin, password), password == "")
        ;

    //向服务器发送注册申请
    int suc = sendSignUpRequest(usrname, password);
    while (!suc)
    {
        cout << "注册失败，此用户名不可用" << endl;
        cout << "重新输入用户名：";
        while (getline(cin, usrname), usrname != "")
            ;
        suc = sendSignUpRequest(usrname, password);
    }

    this->usrname = usrname;
    cout << "注册成功！" << endl;
    return suc;
}

/*
发送注册请求
socket
*/
int User::sendSignUpRequest(string usrname, string password)
{
    return 1; // test

    // send_info.cmd = SIGN_UP;
    // strcpy(send_info.UID, usrname.c_str());
    // strcpy(send_info.PWD, password.c_str());

    // client_sock.Send(send_info);
    // client_sock.Recv(recv_info);

    // return recv_info.REPLY;   //test
}

// int User::deleteAccount() //子类实现
/*
发送注销请求
socket
*/
int User::sendDeleteRequest(string usrname)
{
    return 1; // test

    // int suc = 0;
    // send_info.cmd = DELETE_USER;
    // strcpy(send_info.UID, usrname.c_str());

    // client_sock.Send(send_info);
    // client_sock.Recv(recv_info);

    // //return suc;
    // return recv_info.REPLY;   //test
}

//--------------------Customer--------------------
/*
消费者注销
服务器：检查用户名和密码，检查充电和扣费状态，删除用户数据
返回：注销成功/失败(1/0)
*/
int Customer::deleteAccount()
{
    cout << "==========注销界面=========" << endl;

    //首先检查是否有充电任务正在进行
    int suc = 1;
    int status = getChargeStatus();
    if (status)
    { //如果正在充电
        suc = 0;
        string choice[] = {"等待结束", "强行停止"};
        cout << "您有充电任务正在进行，是否等待充电结束? ";
        printChoice(choice, 2);
        string id;
        while (getline(cin, id), !isLegalChoice(id, 2))
        {
            cout << "请输入有效选项: ";
            printChoice(choice, 2);
        }
        if (id == "1")
            cout << "请等待充电结束后再次尝试注销" << endl;
        else
        {
            suc = cancelCharge(); //申请结束充电
        }
    }

    if (suc) //向服务器发送注销申请
        suc = sendDeleteRequest(this->usrname);
    if (suc)
        cout << "注销成功！" << endl;
    else cout<<"注销失败，请稍后再试"<<endl;

    return suc;
}

/*
充值
服务器：提供用户余额信息，更新用户余额
返回：充值成功/失败(1/0)
*/
int Customer::recharge()
{
    cout << "==========充值界面=========" << endl;
    cout << "当前账户余额: " << this->balance << endl;
    string choice[] = {"20", "50", "100", "200", "自定义"};
    cout << "请选择您要充值的金额(单位：元): ";
    printChoice(choice, 5);

    //获取用户选项
    string id_str;
    int id;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
    {
        cout << "输入无效，请重新选择：";
        printChoice(choice, 5);
    }

    id = id_str[0] - '0';
    double amount = 0.0;
    switch (id)
    {
    case 1:
        amount = 20.0;
        break;
    case 2:
        amount = 50.0;
        break;
    case 3:
        amount = 100.0;
        break;
    case 4:
        amount = 200.0;
        break;
    case 5:
        cout << "请输入自定义充值金额：";
        string str;
        while (getline(cin, str), !isPosNum(str))
            cout << "输入金额无效，请重新输入:";
        const char *s = str.c_str();
        amount = atof(s);
        break;
    }
    //向服务器发送充值请求
    //int suc = sendRechargeRequest(amount);
    int suc = sendUpdateBalanceRequest(amount);
    if (suc) //充值成功，更新本地数据
        this->balance += amount;
    else cout<<"充值失败，请稍后再试"<<endl;
    return suc;
}

/*
发送更新余额请求
socket
*/
int Customer::sendUpdateBalanceRequest(double add)
{
    int suc = 0;
    if(fabs(add) <= 1e-15)  //扣费
    {
        
    }
    else    //充值
    {

    }
    return suc;
}



/*
扣费
服务器：提供用户余额信息，更新用户余额
返回：扣费成功/失败(1/0)
*/
int Customer::deduct()
{
    cout << "==========扣费界面=========" << endl;
    //接收来自服务器的扣费信息
    double pay; //待扣取金额
    // pay = ...    // socket
    cout << "本次消费金额：" << pay << endl;

    //检查账户余额，若余额不足则提示充值
    int suc = 1;
    if (this->balance < pay)
    {
        suc = 0;
        cout << "您的余额不足，请充值" << endl;
        //充值
        suc = recharge();
    }
    if (suc)
    {
        suc = sendUpdateBalanceRequest();   //告知服务器可以进行扣费操作
        if (suc)                   //扣费成功，更新本地余额信息
            this->balance -= pay;
    }
    return suc;
}

/*
提交充电请求
服务器：返回排队结果
*/
int Customer::newChargeRequest()
{
    cout << "==========充电请求界面=========" << endl;
    //首先检查消费者是否添加了车辆
    if (!this->car)
    {
        this->car = new Car;
        cout << "系统检测到您尚未添加车辆，请先完善车辆信息" << endl;
        cout << "请输入车辆电池容量(单位：度，最多两位小数): ";
        string kwh_str;
        while (getline(cin, kwh_str), !isPosNum(kwh_str))
            cout << "输入无效，请重新输入电池容量: ";
        const char *s = kwh_str.c_str();
        double kwh = atof(s);
        //this->car->setBatteryCap(kwh); //设置电池容量
        this->car->BatteryCap = kwh;
    }

    int status = getChargeStatus(); //向服务器获取：是否正在充电
    // status=1表示正在充电，=0表示未正在充电
    if (status == 1)
    {
        cout << "很抱歉，您已开始充电，无法提交新的充电请求" << endl;
        return 0;
    }

    //获取用户输入的充电模式和充电量
    int mode, charge; // charge: 充电量
    string choice1[] = {"快充", "慢充"};
    cout << "请选择充电模式: ";
    printChoice(choice1, 2);
    string id;
    while (getline(cin, id), !isLegalChoice(id, 2))
    {
        cout << "输入无效，请重新选择: ";
        printChoice(choice1, 2);
    }
    mode = (id == "1" ? FAST : SLOW);

    //市面上主流电动汽车的电池容量为15~60kWh
    string choice2[] = {"10", "20", "30", "40", "50", "60"};
    int charge_n[] = {0, 10, 20, 30, 40, 50, 60};
    cout << "请输入充电量(单位：度): ";
    printChoice(choice2, 6);
    while (getline(cin, id), !isLegalChoice(id, 6))
    {
        cout << "输入无效，请重新选择";
        printChoice(choice2, 6);
    }
    charge = charge_n[id[0] - '0'];
    //请求充电量超出电池容量，则自动设置为电池容量（向下取整）
    //double cap = this->car->getBatteryCap();
    double cap = this->car->BatteryCap;
    if (charge > cap)
    {
        cout << "您选择的充电量超出了车辆电池容量，系统已将其自动调整为车辆电池容量: " << cap << "kWh\n";
        charge = floor(cap);
    }

    //设置充电请求
    CarAsk *ask = new CarAsk;
    //ask->BatteryCap = this->car->getBatteryCap();
    ask->BatteryCap = this->car->BatteryCap;
    ask->BatteryNow = this->car->BatteryNow;
    ask->IsFastCharge = (mode == FAST ? true : false);
    //ask->ChargeCap = charge;
    ask->RequestCharge = charge;
    //this->car->setAsk(ask);
    this->car->Ask = ask;

    //向服务器提交充电请求
    int suc = 0;
    suc = sendChargeRequest(); //直接读取this->car->Ask的内容
    if (suc)                   //查看排队结果
    {
        cout<<"提交充电请求成功，正在获取排队信息..."<<endl;
        suc = getQueueRes();
    }
    else cout<<"提交充电请求失败，请稍后再试"<<endl;

    return suc;
}
/*获取充电状态（是否正在充电）
socket
*/
int Customer::getChargeStatus()
{
    // 1表示正在充电，0表示未正在充电
    return 0; // test
}
/*
发送充电请求
socket
*/
// int Customer::sendChargeRequest(int mode, int charge)
int Customer::sendChargeRequest()
{
    return 1; // test
}

/*
取消充电
*/
// int Customer::cancelChargeRequest()
int Customer::cancelCharge()
{
    cout << "==========取消充电界面=========" << endl;
    cout << "是否取消充电? 1. 是    2. 否" << endl;
    string id;
    while (getline(cin, id), !isLegalChoice(id, 2))
        cout << "输入无效，请重新选择: 1. 是    2. 否" << endl;

    int suc = 0;
    if (id == "1")
    {
        cout << "正在尝试取消充电..." << endl;
        //向服务器发送取消充电的请求
        suc = sendCancelRequest();
        if (suc)
            cout << "取消成功！" << endl;
        else cout<<"取消充电失败，请稍后再试"<<endl;
    }
    return suc;
}
/*发送取消充电的请求
socket
*/
int Customer::sendCancelRequest()
{
    return 1; // test
}

/*
查看排队结果
服务器：返回排队结果
*/
int Customer::getQueueRes()
{
    cout << "==========排队信息界面=========" << endl;
    int suc = 0;
    suc = sendQueueInfoRequest();
    if (suc)
    {
        //CarReply *reply = this->car->getCarReply();
        CarReply *reply = this->car->Reply;
        cout << "您当前的排队号码为: " << reply->queueNum << ", 前方还有" << reply->waitingNum << "辆车在等待" << endl;
    }
    else cout<<"获取排队信息失败，请稍后再试"<<endl;
    return suc;
}
//发送查看排队结果的请求
// socket
int Customer::sendQueueInfoRequest()
{
    //填充this->car->Reply
    return 0;   //test
}

/*
(充电结束后)查看充电详情
服务器：返回充电详单
*/
int Customer::getChargeInfo()
{
    cout << "==========充电详情界面=========" << endl;
    int suc = 0;
    suc = sendChargeInfoRequest(); //填充this->car->info
    if (suc)
    {
        //ChargeInfo *info = this->car->getChargeInfo();
        ChargeInfo *info = this->car->info;
        cout << "----------充电详单----------" << endl;
        cout << "充电桩ID: " << info->SID << "\t充电模式: " << info->ChargeMode
             << "\n充电时长: " << getTime(info->time) << "\t充电电量: " << info->cap << "度"
              << "\n充电费用: " << info->pay << "元" << endl;
        cout << "---------------------------" << endl;
    }
    else cout<<"获取充电详情失败，请稍后再试"<<endl;
    return suc;
}
/*
发送查看充电详情请求
socket
*/
int Customer::sendChargeInfoRequest()
{
    return 0;   //test
}

/*------Utils------*/

//判断充值金额合法性
//判断输入是否为正数（整型或浮点型，且最大小数位数为2）
bool User::isPosNum(string str)
{
    if (str == "")
        return false;

    int flag = 0;
    for (int i = 0; i < str.size(); i++)
    {
        if (flag > 2)
            return false; //最大小数位数为2

        if (str[i] == '.')
        {
            if (flag)
                return false; //小数点只能出现一次
            if (i == str.size() - 1 || i == 0)
                return false; //小数点不能是第一位或最后一位
            flag = 1;
            continue;
        }
        if (str[i] < '0' || str[i] > '9')
            return false;

        if (flag) //记录小数点后的位数
            flag++;
    }
    const char *s = str.c_str();
    if (atof(s) <= 0.0) //充值金额必须为正数
        return false;

    return true;
}

//判断输入是否为合法选项(max: 最大选项序号)
bool User::isLegalChoice(string str, int max)
{
    if (str.size() > 1)
        return false;
    int id = str[0] - '0';
    if (id < 1 || id > max)
        return false;
    return true;
}

//输出选项
void User::printChoice(const string choice[], int size)
{
    string separator = "    ";
    string str = "";
    for (int i = 1; i <= size; i++)
    {
        cout << i << ". " << choice[i - 1] << separator;
    }
    cout << endl;
}

//将秒转换为时分秒格式
string User::getTime(int sec)
{
    int h,m,s;
    h = sec/3600;
    sec = sec%3600;
    m = sec/60;
    s = sec%60;
    return to_string(h) + "时" + to_string(m) + "分" + to_string(s) + "秒";
}
