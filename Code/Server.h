/*服务器端实现*/
#include "main.h"

//记录用户信息文件名和日志文件名，该文件和main.cpp位于同一目录下
#define USER_FILENAME "user.txt"
#define LOG_FILENAME   "log.txt"

class Server{
    public:
    
        /*服务器所控制的数据库*/
        DBupdate database;

        /*with chargePort*/    
        int FNum,TNum;  //当前等候区内正在等候的快充/慢充车辆数目 
        int fSeq,tSeq;  //当前等候区内即将进入充电区的车辆编号
        vector<chargePortData*>cData;//记录充电桩状态信息
        int spyChargePort();//充电桩信息监控:获取所有充电桩的相关信息

        /*with admin*/        
        vector<string>admin;    //所有管理员账户名,用于验证管理员身份
        int replyAdmin(string usrname,int cmd,vector<pair<string,string>>info);        //响应管理员客户端请求

        /*with user*/
        vector<string>wUser;    //记录车辆位于等候区的用户
        vector<string>CUser;    //记录车辆位于充电区的用户
        vector<string>CingUser;  //记录当前正在充电的用户
        int replyUser(string usrname,int cmd,vector<pair<string,string>>info);//响应用户客户端请求
        string queueNumGenerate(string usrname,int mode);//车辆排队号码生成
        int schedule(string usrname,int mode,int amount);//调度策略生成，返回充电桩编号
        int recordBill(string usrname,int mode,int time);//费用计算，返回指定用户需要支付的充电费用,time为实际充电时间
        bool usrDataMaintain(string usrname,vector<pair<string,string>> info);//用户信息维护：info格式：<“被修改属性名”，“修改后属性值”>】


};

//数据库更新
class DBupdate{
    public:
        /*数据库初始化*/
        DBupdate::DBupdate();

        /*数据库条目解析*/
        int entryResolve(usrEntry* uE,string line);//用户条目
        int entryResolve(logEntry* lE,string line);//日志条目

        //数据库所有用户信息
        map<string,usrEntry*>usrData;

        //服务日志
        map<string,logEntry*>logData;

        /*用户信息维护*/
        int addUser(usrEntry *data); //新增用户     
        int usrDataChange(struct usrEntry* data);//用户信息维护
        
        /*充电日志*/
        int addLogEntry(logEntry *data);//新增日志条目

        //数据库更新
        int update();

        //数据库条目构造
        string getEntry(usrEntry *data);//用户条目
        string getEntry(logEntry *data);//日志条目
};


//用户数据库条目
struct usrEntry{
    string usrname;
    string passwd;//密码sha1值
    string role;//customer 或 admin
    int balance;//余额
};

//日志条目
struct logEntry{
    string start_time;//开始充电时间
    string usrname;
    int SID;//充电桩ID
    string queueNum;//排队号码
    int mode;//充电模式
    int time;//实际充电时间
    int cost;//金额
};


/*充电桩信息记录*/
struct chargePortData{
    int mode;//充电桩模式
    int state;//充电桩状态（开、关）
    int wCarNum;//当前等候该充电桩的车辆数目
    int total;//充电桩开启以来提供的电量
    //待续....
};

//以字符ch为分隔符字符串拆分
int split(vector<string>&target,string line,char ch);
