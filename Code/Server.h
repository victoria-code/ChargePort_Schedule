#include "main.h"
class Server{
    public:
        /*with chargePort*/    
        int FNum,TNum;  //当前等候区内正在等候的快充/慢充车辆数目 
        int fSeq,tSeq;  //当前等候区内即将进入充电区的车辆编号
        vector<chargePortData*>cData;//记录充电桩状态信息
        int spyChargePort();//充电桩信息监控:获取所有充电桩的相关信息

        /*with admin*/        
        vector<string>admin;    //所有管理员账户名,用于验证管理员身份
        int replyAdmin(string usrname,int cmd,vector<pair<string,string>>info);        //响应管理员客户端请求

        /*with user*/
        vector<string,pair<string,int>>user;     //所有用户信息（用户名,<用户密码，余额》)
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
class DBupdata{
    public:
        /*用户信息维护*/
        string usrFileName;//存储用户(顾客、管理员)信息文件的绝对路径名
        int addUser(usrEntry* data); //新增用户     
        int usrDataChange(usrEntry* data);//用户信息维护
        
        /*充电日志*/
        string logFileName;//记录历史充电记录文件绝对路径名
        int addLogEntry();//新增日志条目
};


//用户数据库条目
struct usrEntry{
    string usrname;
    string passwd;//密码
    string role;//customer 或 admin
    int balance;
};

//日志条目
struct logEntry{
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
