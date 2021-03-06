/*服务器端实现*/
#include "ChargePort.h"
#include "TSocket_server.h"
#include<algorithm>
#pragma warning(disable : 4996)

extern TSocket server_sock;

//记录用户信息文件名和日志文件名，该文件和服务器端main.cpp位于同一目录下
#define USER_FILENAME "C:\\Users\\Zheng siyang\\Desktop\\user.txt"
#define LOG_FILENAME   "C:\\Users\\Zheng siyang\\Desktop\\log.txt"

//充电桩数目为5，编号为0~1代表快充，编号为2~4代表慢充
#define CHARGEPORT_NUM 5

//用户数据库条目
typedef struct UsrEntry {
    string usrname;
    string passwd;//密码sha1值
    string role;//customer 或 admin
    int balance;//余额
    UsrEntry() {};
}usrEntry;

//日志条目
typedef struct LogEntry {
    string start_time;//开始充电时间
    string usrname;
    int SID;//充电桩ID
    string queueNum;//排队号码
    int mode;//充电模式
    int time;//实际充电时间
    int cost;//金额
    LogEntry() {};
}logEntry;

//数据库更新
class DBupdate {
public:
    /*数据库初始化*/
    DBupdate();

    /*数据库条目解析*/
    int entryResolve(usrEntry* uE, string line);//用户条目
    int entryResolve(logEntry* lE, string line);//日志条目

    //数据库所有用户信息
    map<string, usrEntry*>usrData;

    //服务日志
    map<string, vector<logEntry*>>logData;

    /*用户信息维护*/
    int addUser(usrEntry* data); //新增用户     

    /*充电日志*/
    int addLogEntry(logEntry* data);//新增日志条目

    //数据库更新
    int update();

    //数据库条目构造
    string getEntry(usrEntry* data);//用户条目
    string getEntry(logEntry* data);//日志条目
};



class Server {
public:

    /*服务器初始化*/
    void load();
    Server();
    Server(int PID);/**故障情况
    /*服务器下线*/
    ~Server();

    /*服务器所控制的数据库*/
    DBupdate database;
    int usrFind(string usrname, usrEntry* res);//用户信息查询
    int logFind(string usrname, vector<logEntry*>& res);//用户充电记录查询
    int usrDataUpdate(bool to_delete, usrEntry* uE);//用户信息更新<新增用户，用户信息改变，用户注销>

    /*with chargePort*/
    vector<ChargePort*>cData;//记录充电桩状态信息
    int FNum, TNum;  //当前等候区内正在等候的快充/慢充车辆排队号码
    int failID;//出现故障的充电桩ID
    time_t failTime;//出现故障的时间
    void forwardRequet(string usrname, int SID);//向充电桩转发充电区内充电请求，预备充电
    void sendDetail(ChargeTablePool* next);//向用户发送对应的充电详单并进行扣费

   /*记录当前已登录的用户及用户地址*/
    map<string, string>usrAdress;

    /*with client*/
    map<string, CarAsk*>WUser;    //记录车辆位于等候区的用户及其请求
    map<string, CarAsk*>CUser;    //记录车辆位于充电区的用户及其请求
    map<string, pair<string, int>>queueData;//记录排队号码和前车等待数量

    int replyClient(Info usrInfo);//响应用户客户端请求
    int logIn(string usrname, string passwd, usrEntry* uE);//登录验证，登录成功则获取用户信息
    int signUp(string usrname, string passwd, string role);//注册认证
    int deleteUsr(string usrname);//用户注销

    //CUSTOMER
    int balanceChange(string usrname, int amount);//充值、扣费（customer)
    int copeChargeRequest(CarAsk* ask);//处理等候区内用户的充电请求(customer)
    int cancelCharge(string usrname);//处理用户取消充电申请(customer)
    int getQueueData(string usrname, string& qNum, int& curWait);//获取排队信息(customer)
    string queueNumGenerate(string usrname, int mode);//车辆排队号码生成(customer)
    int getCurWaitNum(string usrname);//获取最新的前车等待数量信息(customer)
    int Calling(string& fUser, string& tUser);//获取等候区内即将进入充电区的快充和慢充usrname(叫号）(customer)
    int getFreeCP(int& fSID, int& tSID, int& fTime, int& tTime);//获取充电区内排队队列有空位且等待时间最短的充电桩(customer)
    int resolveRequest(Info& usrInfo, CarAsk* ask);//提取充电请求(customer)

    //ADMIN
    int openCP(string usrname, int SID);//开启充电桩
    int closeCP(string usrname, int SID);//关闭充电桩
    int getChargePortData(string usrname);//获取充电桩状态信息
    int getReport(string usrname);//获取报表
    int getCarData(string usrname);//获取等候服务的车辆信息

    //获取时间
    string getCurTime();
};




//以字符ch为分隔符字符串拆分
int split(vector<string>& target, string line, char ch);
