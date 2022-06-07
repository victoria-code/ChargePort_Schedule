#include "User.h"
#include "main.h"

/*
用户登录
服务器：检查用户名和密码
返回：登录成功/失败(1/0)
*/
int User::logIN()
{
    int suc = 0;
    //获取用户输入的用户名和密码
    string usrname, password;
    cout<<"请输入用户名：";
    while(getline(cin, usrname), usrname == "");
    cout<<"请输入密码：";
    while(getline(cin, password), password == "");

    //向服务器发送登录请求
    suc = sendLogInRequest(usrname, password);

    //提示登录成功/失败信息
    // suc=1表示成功，2表示用户不存在，3表示密码错误
    while (suc != 1)
    {
        if (suc == 2)
        {
            cout << "登录失败，请检查用户名是否正确" << endl;
            cout << "重新输入用户名:";
            cin >> usrname;
        }
        else if (suc == 3)
        {
            cout << "登录失败，请检查密码是否正确" << endl;
            cout << "重新输入密码:";
            cin >> password;
        }
        suc = sendLogInRequest(usrname, password);
    }

    this->usrname = usrname;
    cout << "登录成功！" << endl;
    return suc;
}

/*
发送登录请求
socket
*/
int User::sendLogInRequest(string usrname, string password)
{
    int suc = 0;

    //return suc;
    return 1;   //test
}

/*
用户注册
服务器：检查用户名和密码，新增用户数据
返回：注册成功/失败(1/0)
*/
int User::signUp()
{
    //获取用户输入的用户名和密码
    //对密码没有限制，只要不为空即可；用户名不能与已有的重复
    string usrname, password;
    cout<<"请输入用户名：";
    while(getline(cin, usrname), usrname != "");
    cout<<"请输入密码：";
    while(getline(cin, password), password != "");

    //向服务器发送注册申请
    int suc = sendSignUpRequest(usrname, password);
    while (!suc)
    {
        cout << "注册失败，此用户名不可用" << endl;
        cout<<"重新输入用户名：";
        while(getline(cin, usrname), usrname != "");
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
    int suc = 0;

    //return suc;
    return 1;   //test
}

/*
用户注销
服务器：检查用户名和密码，检查充电和扣费状态，删除用户数据
返回：注销成功/失败(1/0)
*/
int User::deleteAccount()
{
    int suc = 0;
    //向服务器发送注销申请
    suc = sendDeleteRequest(this->usrname);
    //提示注销成功/失败信息
    if(!suc)
    {
        cout<<"注销失败，您有充电任务正在进行，是否等待充电结束？\n1. 等待结束    2. 强行停止"<<endl;
        string id;
        while(getline(cin, id), !isLegalChoice(id, 2))
            cout<<"请输入有效选项: 1. 等待结束    2. 强行停止"<<endl;
    }

    cout<<"注销成功！"<<endl;
    return suc;
}

/*
发送注销请求
socket
*/
int User::sendDeleteRequest(string usrname)
{
    int suc = 0;

    //return suc;
    return 1;   //test
}

//--------------------Customer--------------------
/*
充值
服务器：提供用户余额信息，更新用户余额
返回：充值成功/失败(1/0)
*/
int Customer::recharge()
{
    cout << "==========充值界面=========" << endl;
    cout << "当前账户余额: " << this->balance << endl;
    cout << "请选择您要充值的金额：\n1. 20元    2. 50元    3. 100元    4. 200元    5. 自定义金额" << endl;

    //获取用户选项
    string id_str;
    int id;
    while (getline(cin, id_str), !isLegalChoice(id_str, 5))
        cout << "输入无效，请重新选择：\n1. 20元    2. 50元    3. 100元    4. 200元    5. 自定义金额" << endl;
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
    //向服务器发送更新数据
    int suc = setUsrBalance(amount + this->balance);
    //服务器端更新成功后，更新本地数据
    if (suc)
        this->balance += amount;

    return suc;
}

/*
向服务器发送余额更新数据(num:新的余额)
返回：更新成功/失败(1/0)
socket
*/
int Customer::setUsrBalance(double num)
{
    int suc = 0;

    //return suc;
    return 1;   //test
}

/*
扣费
服务器：提供用户余额信息，更新用户余额
返回：扣费成功/失败(1/0)
*/
int Customer::deduct()
{
    //❓接收来自服务器的扣费信息
    int pay; //待扣取金额
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
        suc = setUsrBalance(this->balance - pay); //扣费
        if (suc)                                  //扣费成功: 更新本地余额信息
            this->balance -= pay;
    }
    return suc;
}

/*
提交充电请求
服务器：返回排队结果
*/
int Customer::chargeRequest()
{
    int suc = 0;
    //获取用户输入的充电模式和充电量
    int mode, charge;   //charge: 充电量
    //...

    //向服务器提交充电请求
    suc = sendChargeRequest(mode, charge);
    //输出服务器返回的排队信息

    return suc;
}

/*
发送充电请求
socket
*/
int Customer::sendChargeRequest(int mode, int charge)
{
    int suc = 0;

    //return suc;
    return 1;   //test
}

/*
修改充电请求
❓复用chargeRequest?
服务器：返回新的排队结果
*/
// int User::changeRequest()
// int Customer::changeChargeRequest()
// {
//     int suc = 0;
//     //获取用户输入的充电模式和充电量
//     //向服务器提交充电请求
//     //输出服务器返回的排队信息
//     return suc;
// }

/*
查看排队结果
服务器：返回排队结果
*/
int Customer::getQueueRes()
{
    int suc = 0;
    int queueNum = 0, waitingNum = 0;
    //向服务器提交查看排队号码申请 和 查看前车等待数量申请
    //-1表示查找失败
    queueNum = sendGetQueueRequest();
    waitingNum = sendGetWaitingRequest();
    //输出排队信息：排队号码 + 前车等待数量
    if(queueNum != -1 && waitingNum != -1)
        cout<<"您当前的排队号码为:"<<queueNum<<", 前方还有"<<waitingNum<<"辆车在等待"<<endl;
    
    //return queueNum;
    return 0;
}

/*
发送查看排队号码请求
*/
int Customer::sendGetQueueRequest()
{
    return 1;   //test
}

/*
发送查看前车等待数量请求
*/
int Customer::sendGetWaitingRequest()
{
    return 1;   //test
}

/*
(充电结束后)查看充电详情
服务器：返回充电详单
*/
string Customer::getChargeInfo()
{
    //向服务器提交查看充电详情申请
    //❓ChargeInfo: 充电详单
    // ChargeInfo *info = sendGetChargeInfoRequest();
    // if(!info){}
    //打印详单内容

    return "";
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
                return false;   //小数点只能出现一次
            if(i == str.size() - 1 || i == 0)
                return false;   //小数点不能是第一位或最后一位
            flag = 1;
            continue;
        }
        if (str[i] < '0' || str[i] > '9')
            return false;

        if (flag) //记录小数点后的位数
            flag++;
    }
    const char* s = str.c_str();
    if(atof(s) <= 0.0)  //充值金额非负
        return false;

    return true;
}

//判断输入是否为合法选项(max: 最大选项序号)
bool User::isLegalChoice(string str, int max)
{
    if(str.size()>1)
        return false;
    int id = str[0] - '0';
    if(id < 1 || id > max)
        return false;
    return true;
}
